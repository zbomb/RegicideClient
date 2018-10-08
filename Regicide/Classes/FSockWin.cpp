#include "FSockWin.h"
#include <asio.hpp>
#include <string>
#include <functional>


using asio::ip::tcp;
using namespace std::placeholders;

/*================================= FSockSystemWin =================================*/
FSockSystemWin::FSockSystemWin()
{
	
	// Init system
}

FSock* FSockSystemWin::CreateSocket( ISOCK_TYPE Type )
{
	if( Type == ISOCK_TYPE::ISOCK_TCP )
	{
		// Create FSockWin
		FSockWinTCP* NewSocket = new FSockWinTCP( &Context );

		return NewSocket;

	}
	else if( Type == ISOCK_TYPE::ISOCK_UDP )
	{
		//FSockWinUDP* NewSocket = new FSockWinUDP( &Context );

		return nullptr;
	}

	return nullptr;
}

void FSockSystemWin::DestroySocket( FSock* Sck )
{
	// Destroy FSockWin
}

void FSockSystemWin::DestroySystem()
{
	// Cleanup system
}



/*================================= FSockWinTCP =================================*/

FSockWinTCP::FSockWinTCP( asio::io_context* inContext )
	: LinkedContext( inContext ), RemoteEndpoint(),
	EndpointIterator(), AddressMode( AddressType::NotSet )
{

}


bool FSockWinTCP::SetRemoteAddress( std::string Address, unsigned int Port, bool bIsHostName )
{
	if( bIsHostName )
	{
		// We need an io context to perform this action
		if( LinkedContext )
		{
			printf( "[Warning] Failed to perform host name lookup with TCP socket because there was no io context linked to the socket!\n" );
			return false;
		}

		// Convert port to a string
		std::string PortStr = std::to_string( Port );

		// Perform lookup
		tcp::resolver AddressResolver( *LinkedContext );

		asio::error_code resErr;
		tcp::resolver::query Query( Address, PortStr );
		EndpointIterator = AddressResolver.resolve( Query, resErr );

		// Check for error
		if( resErr )
		{
			printf( "[Warning] Failed to resolve provided hostname for tcp socket! Hostname: %s  Error: %s\n", Address.c_str(), resErr.message().c_str() );
			return false;
		}

		AddressMode = AddressType::Hostname;
		return true;
	}
	else
	{
		// We were supplied a direct ip in decimal form, so lets build an asio ip from it
		asio::error_code ipError;
		asio::ip::address TargetAddress = asio::ip::address::from_string( Address, ipError );

		if( ipError )
		{
			printf( "[Warning] IP Address supplied to TCP socket was invalid! %s\n", ipError.message().c_str() );
			return false;
		}

		// Set endpoint
		RemoteEndpoint = tcp::endpoint( TargetAddress, Port );
		AddressMode = AddressType::Endpoint;

		return true;
	}
}

bool FSockWinTCP::Connect()
{
	// Check if the current state is valid and we can perform this operation  TODO: Check endpoint iterator validity
	if( AddressMode == AddressType::NotSet || LinkedContext == nullptr ||
		( RemoteEndpoint.address().is_unspecified() && AddressType::Endpoint ) )
	{
		printf( "[Warning] Connect failed because the endpoint has not been properly set!\n" );
		return false;
	}

	// Check if we need to create the socket
	if( !InternalSock )
	{
		InternalSock = std::make_shared< tcp::socket >( *LinkedContext );
	}

	if( AddressMode == AddressType::Endpoint )
	{
		asio::error_code ConnectError;
		InternalSock->connect( RemoteEndpoint, ConnectError );

		if( ConnectError )
		{
			printf( "[Warning] TCP socket failed to connect to the remote address at %s! Error: %s\n", RemoteEndpoint.address().to_string().c_str(), ConnectError.message().c_str() );
			return false;
		}
	}
	else
	{
		asio::error_code ConnectError;
		asio::connect( *InternalSock, EndpointIterator, ConnectError );

		if( ConnectError )
		{
			printf( "[Warning] TCP socket failed to connect to the hostname %s! Error: %s\n", EndpointIterator->host_name().c_str(), ConnectError.message().c_str() );
			return false;
		}
	}

	return true;
}

bool FSockWinTCP::IsConnected() const
{
	return InternalSock && InternalSock->is_open();
}

unsigned int FSockWinTCP::GetBytesAvailable() const
{
	return (unsigned int)( InternalSock ? InternalSock->available() : 0 );
}

unsigned int FSockWinTCP::Receive( std::vector<uint8>& OutBuffer, FSockError& OutError )
{
	if( !InternalSock )
	{
		OutError = FSockWinError( asio::error::not_connected );
		return 0;
	}

	asio::error_code ReadError;
	unsigned int NumBytes = InternalSock->read_some( asio::buffer( OutBuffer ), ReadError );

	OutError = FSockWinError( ReadError );
	return NumBytes;
}

unsigned int FSockWinTCP::Send( std::vector<uint8> Buffer, FSockError& OutError )
{
	if( !InternalSock )
	{
		OutError = FSockWinError( asio::error::not_connected );
		return 0;
	}

	asio::error_code SendError;
	unsigned int NumBytes = InternalSock->write_some( asio::buffer( Buffer ), SendError );

	OutError = FSockWinError( SendError );
	return NumBytes;
}

void FSockWinTCP::ConnectAsync( std::function< void( FSockError ) > Callback )
{
	if( Callback == nullptr )
	{
		printf( "[Warning] Failed to asynchronously connect to tcp socket because the provided callback was null!\n" );
		return;
	}
	else if( AddressMode == AddressType::NotSet || LinkedContext == nullptr ||
		( RemoteEndpoint.address().is_unspecified() && AddressType::Endpoint ) )
	{
		printf( "[Warning] Failed to asynchronously connect to tcp socket because the remote host was not properly set!\n" );
		Callback( FSockWinError( asio::error_code( asio::error::host_not_found ) ) );
		return;
	}

	// Check if the socket needs to be created
	if( !InternalSock )
	{
		InternalSock = std::make_shared<tcp::socket>( *LinkedContext );
		InternalSock->get_io_context();
	}

	if( AddressMode == AddressType::Endpoint )
	{
		InternalSock->async_connect( RemoteEndpoint, [ Callback, this ]( asio::error_code Error )
		{
			// If the connect failed, then we need to close the socket
			if( Error )
			{
				if( InternalSock )
				{
					InternalSock->close();
				}
				InternalSock.reset();
			}

			// Check if callback is still valid
			if( Callback == nullptr )
			{
				printf( "[Warning] Async Connect callback failed (TCP)! The bound function is null. The connection %s with code %d\n", Error ? "Failed" : "Succeeded", Error.value() );
			}
			else
			{
				Callback( FSockWinError( Error ) );
			}
		} );
	}
	else
	{
		asio::async_connect( *InternalSock, EndpointIterator, [ Callback, this ]( asio::error_code Error, tcp::resolver::iterator It )
		{
			// If there was an error, then automatically close the socket
			if( Error )
			{
				if( InternalSock )
				{
					InternalSock->close();
				}
				InternalSock.reset();
			}

			// Check if callback is still valid
			if( Callback == nullptr )
			{
				printf( "[Warning] Async Connect callback failed (TCP)! The bound function is null. The connection %s with code %d\n", Error ? "Failed" : "Succeeded", Error.value() );
			}
			else
			{
				Callback( FSockWinError( Error ) );
			}

		} );
	}

	// Start running a new thread to handle the async methods on this socket if needed
	if( !AsyncThread )
	{
		
		AsyncThread = std::make_shared<std::thread>( [ this ]() 
		{ 
			try
			{
				LinkedContext->run();
			}
			catch( std::exception& Exception )
			{
				printf( "[ERROR] An exception was thrown while starting socket io service! %s", Exception.what() );
			}
			 
		} );
	}

}

void FSockWinTCP::SendAsync( std::vector<uint8> Buffer, std::function< void( FSockError, unsigned int ) > Callback )
{
	// Check for invalid callback
	if( Callback == nullptr )
	{
		printf( "[Warning] Send Async call to TCP socket failed! The provided callback was null!\n" );
		return;
	}
	else if( !InternalSock )
	{
		printf( "[Warning] Async Async call to TCP socket failed! The socket was null, probably not connected!\n" );
		if( Callback != nullptr )
		{
			Callback( FSockWinError( asio::error::not_connected ), 0 );
		}
		return;
	}

	asio::async_write( *InternalSock, asio::buffer( Buffer ), [ Callback ]( asio::error_code Error, unsigned int BytesWritten )
	{
		// Check if callback is valid
		if( Callback == nullptr )
		{
			printf( "[Warning] Async send (TCP) failed to run callback, it was null! The call %s\n", Error ? "Failed" : "Succeeded" );
		}
		else
		{
			Callback( FSockWinError( Error ), BytesWritten );
		}
	} );
}

void FSockWinTCP::ReceiveAsync( std::vector<uint8>& Buffer, std::function< void( FSockError, unsigned int, std::vector< uint8 >& ) > Callback )
{
	if( Callback == nullptr )
	{
		printf( "[Warning] Async receive on a TCP socket failed! The provided callback was null!\n" );
		return;
	}
	else if( !InternalSock )
	{
		printf( "[Warning Async receive on a TCP socket failed! The connection is not open!\n" );

		if( Callback != nullptr )
		{
			Callback( FSockWinError( asio::error::not_connected ), 0, std::vector< uint8 >() );
		}
		return;
	}

	InternalSock->async_read_some( asio::buffer( Buffer ), [ &Buffer, Callback ]( asio::error_code Error, unsigned int BytesRead )
	{
		if( Callback == nullptr )
		{
			printf( "[Warning] Async read call on a TCP socket completed but the callback is null! The operation %s\n", Error ? "failed!" : "succeeded." );
		}
		else
		{
			Callback( FSockWinError( Error ), BytesRead, Buffer );
		}
	} );
}

void FSockWinTCP::Shutdown( ShutdownDir Direction )
{
	if( InternalSock )
	{
		// Convert ShutdownDir to asio shutdown direction
		tcp::socket::shutdown_type AsioShutdownType;
		switch( Direction )
		{
			case FSock::ShutdownDir::BOTH:
				AsioShutdownType = tcp::socket::shutdown_type::shutdown_both;
				break;
			case FSock::ShutdownDir::SEND:
				AsioShutdownType = tcp::socket::shutdown_type::shutdown_send;
				break;
			case FSock::ShutdownDir::RECV:
				AsioShutdownType = tcp::socket::shutdown_type::shutdown_receive;
				break;
		}

		asio::error_code ShutdownError;
		InternalSock->shutdown( AsioShutdownType, ShutdownError );

		if( ShutdownError )
		{
			printf( "[Warning] There was an error while shutting down a tcp socket! %s\n", ShutdownError.message().c_str() );
		}
	}
	else
	{
		printf( "[Warning] Attempt to shutdown a TCP socket that isnt connected!\n" );
	}
}

void FSockWinTCP::Close()
{
	if( InternalSock )
	{
		asio::error_code CloseError;

		if( LinkedContext && AsyncThread )
		{
			LinkedContext->post( [ this, &CloseError ]() { InternalSock->close( CloseError ); } );
		}
		else
		{
			InternalSock->close( CloseError );
		}

		if( CloseError )
		{
			printf( "[Warning] An error occurred while closing a TCP socket! %s\n", CloseError.message().c_str() );
		}

		InternalSock.reset();
	}

	// If were running async operations, then join threads
	if( AsyncThread )
	{
		AsyncThread->join();
	}
}


/*================================= FSockWinUDP =================================*/


/*================================= FSockWinError =================================*/
FSockWinError::FSockWinError( asio::error_code inErr )
	: Error( inErr )
{
}

FSockWinError::FSockWinError()
	: Error()
{
}

bool FSockWinError::IsDisconnect()
{
	return Error == asio::error::eof;
}

FSockWinError::operator bool()
{
	return Error.operator bool();
}

std::string FSockWinError::GetErrorMessage()
{
	return Error.message();
}
