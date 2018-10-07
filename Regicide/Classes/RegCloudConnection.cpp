#include <memory>
#include <string>
#include <functional>
#include "ISockSystem.h"
#include "RegCloudConnection.h"


using namespace std::placeholders;


RegCloudConnection::RegCloudConnection()
	: ReceiveBuffer( REGCLOUD_BUFFERLEN ), ParseBuffer()
{
	// Create Socket
	ISockSystem* SocketSystem = ISockSystem::Get();

	if( SocketSystem == nullptr )
	{
		printf( "[CRITICAL] Invalid socket system! Online features will not function!" );
	}
	else
	{
		Connection = std::shared_ptr< FSock >( SocketSystem->CreateSocket( ISOCK_TYPE::ISOCK_TCP ) );

		if( !Connection )
		{
			printf( "[CRITICAL] Socket system failed to create a new socket for RegCloud!" );
		}
		else
		{
			printf( "[RegCloud] Socket created successfully!" );
		}
	}

}


void RegCloudConnection::BeginConnect()
{
	if( !Connection )
	{
		printf( "[CRITICAL] Failed to connect to RegCloud because the socket is null!" );
		return;
	}

#ifndef REGCLOUD_DEBUG
	bool bUsingHostname = true;
	std::string Address = REGCLOUD_REMOTEADDRESS;
#else
	bool bUsingHostname = false;
	std::string Address = REGCLOUD_DEBUGADDRESS;
#endif
	
	// Set address/hostname
	if( !Connection->SetRemoteAddress( Address, REGCLOUD_PORT, bUsingHostname ) )
	{
		printf( "[CRITICAL] RegCloud failed to resolve hostname.. please check supplied address." );
		return;
	}

	// Bind a callback, and start the async connect
	auto Callback = std::bind( &RegCloudConnection::OnConnect, *this, _1 );
	Connection->ConnectAsync( Callback );

	printf( "[RegCloud] Connecting..." );
}

void RegCloudConnection::OnConnect( FSockError ErrorCode )
{
	if( ErrorCode )
	{
		printf( "[ERROR] Failed to connect to RegSys! %s", ErrorCode.GetErrorMessage() );
		return;
	}
	else if( !Connection )
	{
		printf( "[ERROR] Failed to continue receiving data because the socket is null!" );
		return;
	}

	printf( "[RegCloud] Connected to RegSys!" );

	// Begin receive loop
	auto Callback = std::bind( &RegCloudConnection::OnReceive, *this, _1, _2, _3 );
	ReceiveBuffer.clear();
	Connection->ReceiveAsync( ReceiveBuffer, Callback );
}

bool RegCloudConnection::IsConnected() const
{
	if( !Connection )
	{
		return false;
	}

	return Connection->IsConnected();
}

bool RegCloudConnection::Send( std::vector<uint8>& Data, std::function<void( FSockError, unsigned int )> Callback )
{
	if( !IsConnected() )
	{
		printf( "[Warning] Failed to send data to RegSys because the socket is not connected/null!" );
		return false;
	}
	else if( Data.size() == 0 )
	{
		printf( "[Warning] Failed to send data to RegSys because the supplied buffer was empty!" );
		return false;
	}

	// If a callback wasnt supplied, we will use the default callback to print any send errors
	if( !Callback )
	{
		Callback = std::bind( &RegCloudConnection::OnSendDefault, *this, _1, _2 );
	}

	Connection->SendAsync( Data, Callback );
	return true;
}

void RegCloudConnection::OnSendDefault( FSockError Error, unsigned int BytesSent )
{
	if( Error )
	{
		printf( "[Warning] Error sending data to RegSys! %s", Error.GetErrorMessage() );
	}
}

void RegCloudConnection::OnReceive( FSockError ErrorCode, unsigned int NumBytes, std::vector<uint8>& Data )
{
	// Check for errors and disconnects
	if( ErrorCode )
	{
		if( ErrorCode.IsDisconnect() )
		{
			printf( "[RegCloud] Disconnected from the server!" );
			return;
		}
		else
		{
			printf( "[Warning] An error has occurred while reading data from the server! %s", ErrorCode.GetErrorMessage() );
		}
	}

	// Pass data along to handler
	if( NumBytes > 0 )
	{
		OnData( std::vector< uint8 >( Data.begin(), Data.begin() + NumBytes ) );
	}

	// Continue reading data if possible
	if( !Connection )
	{
		printf( "[ERROR] Failed to continue receiving data from the server because the connection is null" );
	}
	else
	{
		auto Callback = std::bind( &RegCloudConnection::OnReceive, *this, _1, _2, _3 );
		ReceiveBuffer.clear();
		Connection->ReceiveAsync( ReceiveBuffer, Callback );
	}

}

void RegCloudConnection::OnData( std::vector< uint8 >& Data )
{
	// Append data to the parse buffer
	ParseBuffer.insert( ParseBuffer.end(), Data.begin(), Data.end() );

	// Parse received data

}

bool RegCloudConnection::ConsumeBuffer( FIncomingPacket& OutPacket )
{

}

bool RegCloudConnection::DecryptPacket( FIncomingPacket& OutPacket )
{

}

bool RegCloudConnection::ReadHeader( FIncomingPacket& OutPacket )
{

}
