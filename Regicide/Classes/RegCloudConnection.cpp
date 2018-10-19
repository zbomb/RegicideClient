#include <memory>
#include <string>
#include <functional>
#include <vector>
#include <deque>
#include "RegCloudConnection.h"
#include "RegCloud.h"
#include "CryptoLibrary.h"
#include "cocos/base/CCConsole.h"
#include "EventHub.h"


using namespace std::placeholders;
using namespace cocos2d;

static bool bFirstConnect = true;

RegCloudConnection::RegCloudConnection()
	: ReceiveBuffer( REGCLOUD_BUFFERLEN ), ParseBuffer(), LinkedContext()
{
    Socket = std::make_shared< tcp::socket >( LinkedContext );
	// Default encryption keys for low security packets
	DefaultKey = new uint8[ 32 ] {	0x81, 0xe5, 0x20, 0x5e, 0x97, 0xf9, 0x37, 0x64, 0x1a, 0xf8, 0x7d, 0x47, 0x79, 0x31, 0x12, 0x92,
									0xb8, 0xb5, 0xa5, 0x6c, 0x9e, 0xa8, 0x48, 0x17, 0x65, 0xee, 0xa7, 0xfc, 0xbe, 0xdf, 0xb6, 0xe9 };
	DefaultIV = new uint8[ 16 ] {	0xc5, 0xa5, 0xb1, 0x1d, 0xe7, 0x72, 0x88, 0x30, 0xab, 0xc1, 0x49, 0x35, 0x3d, 0xee, 0x42, 0x00 };

	// Determine local endianess
	int Value = 1;
	if( *(char *) &Value == 1 )
	{
		cocos2d::log( "[RegSys] Detected local system is using little endian byte order." );
		LocalByteOrder = EndianOrder::LittleEndian;
	}
	else
	{
		cocos2d::log( "[RegSys] Detected local system is using big endian byte order." );
		LocalByteOrder = EndianOrder::BigEndian;
	}
}


RegCloudConnection::~RegCloudConnection()
{
	bKill = true;

	if( Socket )
	{
		if( Socket->is_open() )
		{
			LinkedContext.post( [ this ]()
			{
				asio::error_code ShutdownError;
				Socket->shutdown( asio::socket_base::shutdown_both, ShutdownError );
				if( ShutdownError )
				{
					cocos2d::log( "[ERROR] An error occurred while shutting the socket down! %s\n", ShutdownError.message().c_str() );
				}

				asio::error_code CloseError;
				Socket->close( CloseError );

				if( CloseError )
				{
					cocos2d::log( "[ERROR] An error occurred while closing the connection with RegSys! %s", CloseError.message().c_str() );
				}
				else
				{
					cocos2d::log( "[RegSys] Connection with NetNode was closed!" );
				}
			} );
		}
	}

	if( IOThread )
	{
		IOThread->join();
	}

	Socket.reset();
	ReceiveBuffer.clear();
	ParseBuffer.clear();

	if( SessionKey )
		delete[] SessionKey;

	if( DefaultKey )
		delete[] DefaultKey;

	if( DefaultIV )
		delete[] DefaultIV;

}

bool RegCloudConnection::BeginConnect()
{
	if( !Socket )
	{
		cocos2d::log( "[CRITICAL] Failed to connect to RegCloud because the socket/context is null!" );
		return false;
	}

	// Check if were already connected
	if( Socket && Socket->is_open() )
	{
		cocos2d::log( "[Warning] Attempt to connect to RegSys, but a connection is already established!" );
		return false;
	}

	CurrentState = ConnectionState::InProgress;
    
    // Reset the io context if needed
    if( !LinkedContext.stopped() )
    {
        LinkedContext.stop();
    }
    
    LinkedContext.reset();
    
    // Stop the current IO thread if needed
    if( IOThread )
    {
        IOThread->join();
        IOThread.reset();
    }
    
    // We need to reopen the socket if its closed
    Socket->open( asio::ip::tcp::v4() );
    
#ifndef REGCLOUD_DEBUG
	bool bUsingHostname = true;
	std::string Address = REGCLOUD_REMOTEADDRESS;
#else
	bool bUsingHostname = false;
	std::string Address = REGCLOUD_DEBUGADDRESS;
#endif
	
	if( bUsingHostname )
	{
		tcp::resolver HostnameResolver( LinkedContext );
		tcp::resolver::query HostnameQuery( Address, std::to_string( REGCLOUD_PORT ) );

		asio::error_code ResolveError;
		tcp::resolver::iterator EndpointIter = HostnameResolver.resolve( HostnameQuery, ResolveError );

		if( ResolveError )
		{
			cocos2d::log( "[Warning] An error has occurred while resolving the RegSys hostname! %s", ResolveError.message().c_str() );
			CurrentState = ConnectionState::Failed;
			return false;
		}
        
		asio::async_connect( *Socket, EndpointIter, [ this ]( const asio::error_code& ErrorCode, tcp::resolver::iterator Iter )
		{
			if( !this )
			{
				cocos2d::log( "[ERROR] Failed to finalize connection with a RegSysNetNode because the connection object is null!" );
				CurrentState = ConnectionState::Failed;
				return;
			}

			this->OnConnect( ErrorCode );
		} );
	}
	else
	{
		asio::error_code IpError;
		asio::ip::address IpAddress = asio::ip::address::from_string( Address, IpError );

		if( IpError )
		{
			cocos2d::log( "[Warning] An error has occurred while converting Ip from string! %s", IpError.message().c_str() );
			CurrentState = ConnectionState::Failed;
			return false;
		}

		tcp::endpoint Endpoint = tcp::endpoint( IpAddress, REGCLOUD_PORT );

		Socket->async_connect( Endpoint, [ this ]( asio::error_code ErrorCode )
		{
            if( !this )
			{
				cocos2d::log( "[ERROR] Failed to finalize connection with a RegSysNetNode because the connection object is null!" );
				CurrentState = ConnectionState::Failed;
				return;
			}

			this->OnConnect( ErrorCode );
		} );
	}
    
    if( !LinkedContext.stopped() && IOThread )
    {
        LinkedContext.stop();
        LinkedContext.reset();
    }
    
    if( IOThread )
    {
        IOThread->join();
        IOThread.reset();
    }
    
    IOThread = std::make_shared<std::thread>( [ this ]()
    {
        try
        {
            LinkedContext.run();
        }
        catch( ... )
        {
            cocos2d::log( "[ERROR] An exception was thrown from the RegSys connection!" );
            if( Socket && Socket->is_open() )
            {
                Socket->close();
            }
        }
    } );
    
    bFirstConnect = false;

	cocos2d::log( "[RegSys] Connecting to NetNode..." );
    NullEventData Data;
    EventHub::Execute( "ConnectBegin", Data );

    return true;
}

void RegCloudConnection::TimeoutConnection( bool bUpdateState )
{
    // Cancel all operations on the underlying socket
    if( Socket )
    {
        Socket->cancel();
        
        if( bUpdateState )
            Socket->close();
    }
    
    if( bUpdateState )
    {
        CurrentState = ConnectionState::Failed;
    }
}

void RegCloudConnection::OnConnect( asio::error_code ErrorCode )
{
	if( ErrorCode )
	{
        if( ErrorCode == asio::error::operation_aborted )
        {
            cocos2d::log( "[ERROR] Connection attempt has timed-out!" );
            
            // The event will be called from the time-out function, more reliably
        }
        else
        {
            cocos2d::log( "[ERROR] Failed to connect to RegSys! %s", ErrorCode.message().c_str() );
        
            NumericEventData Data( (int) ConnectResult::ConnectionError );
            EventHub::Execute( "ConnectResult", Data );
        }
        
        CurrentState = ConnectionState::Failed;
		return;
	}
	else if( !Socket )
	{
		cocos2d::log( "[ERROR] Failed to finalize socket connect because the local sock is null!" );
		
        NumericEventData Data( (int) ConnectResult::ConnectionError );
        EventHub::Execute( "ConnectResult", Data );
		CurrentState = ConnectionState::Failed;
		return;
	}

	if( bKill )
		return;

	CurrentState = ConnectionState::Connected;
	cocos2d::log( "[RegSys] Connected to a NetNode!" );

	// Begin receive loop
	LinkedContext.post( [ this ] ()
	{
		Socket->async_read_some( asio::buffer( ReceiveBuffer ), [ this ]( asio::error_code ErrorCode, unsigned int NumBytes )
		{
			if( !this )
			{
				cocos2d::log( "[ERROR] Failed to process data from a RegSysNetNode because the connection object is null!" );
				return;
			}

			this->OnReceive( ErrorCode, NumBytes );
		} );
	} );

	// Fire off internal event
	NullEventData Data;
    EventHub::Execute( "_Sock_Connect_", Data );

}

bool RegCloudConnection::IsConnected() const
{
	return( Socket && Socket->is_open() );
}

bool RegCloudConnection::Send( std::vector<uint8>& Data, std::function<void( asio::error_code, unsigned int )> Callback, ENetworkEncryption EncryptionLevel )
{
	if( !IsConnected() || bKill )
	{
		cocos2d::log( "[Warning] Failed to send data to RegSys because the socket is not connected/null!" );
		return false;
	}
	else if( Data.size() == 0 )
	{
		cocos2d::log( "[Warning] Failed to send data to RegSys because the supplied buffer was empty!" );
		return false;
	}

	// If a callback wasnt supplied, we will use the default callback to print any send errors
	if( !Callback )
	{
		Callback = std::bind( &RegCloudConnection::OnSendDefault, this, _1, _2 );
	}

	// Determine encryption key needed for the specified encryption level
	// Also, check if the specified encryption level is possible with the current state of the connection
	uint8* EncryptionKey = nullptr;
	uint8* EncryptionIV = nullptr;

	if( EncryptionLevel == ENetworkEncryption::Full )
	{
		if( !SessionKey )
		{
			cocos2d::log( "[ERROR] Failed to send fully encrypted packet to RegSys because a session has not been established!" );
			return false;
		}

		EncryptionKey = SessionKey;
		EncryptionIV = DefaultIV;
	}
	else if( EncryptionLevel == ENetworkEncryption::Light )
	{
		EncryptionKey = DefaultKey;
		EncryptionIV = DefaultIV;
	}
	else if( EncryptionLevel != ENetworkEncryption::None )
	{
		throw std::exception();
		return false;
	}

	// Perform encryption if needed, and check for missing keys
	if( EncryptionLevel != ENetworkEncryption::None )
	{
		if( !EncryptionIV || !EncryptionKey )
		{
			cocos2d::log( "[ERROR] Encryption key needed to send packet not found! Please restart..." );
			return false;
		}
		else if( !CryptoLibrary::AesEncrypt( Data, EncryptionKey, EncryptionIV ) )
		{
			cocos2d::log( "[ERROR] Failed to send packet to RegSys! Packet encryption was not successful" );
			return false;
		}
	}

	// Create public header structure
	FPublicHeader PacketHeader;
	PacketHeader.PacketSize = PacketHeader.GetSize() + Data.size();
	PacketHeader.EncryptionMethod = (int32) EncryptionLevel;
	
	// Determine Endian Order
	uint16 TestValue = 0x0001;
	uint8* TestBytes = (uint8*)&TestValue;
	EEndianOrder Order = TestBytes[ 0 ] ? EEndianOrder::LittleEndian : EEndianOrder::BigEndian;

	PacketHeader.EndianOrder = (uint32) Order;

	// Build Final Packet
	std::vector< uint8 > FinalPacket( PacketHeader.PacketSize );
	std::move( Data.begin(), Data.end(), FinalPacket.begin() + PacketHeader.GetSize() );
	Data.clear();

	if( !PacketHeader.Serialize( FinalPacket.begin(), true, false ) )
	{
		cocos2d::log( "[ERROR] Failed to create packet to send to RegSys, header serialization failed!" );
		FinalPacket.clear();
		return false;
	}

	// Insert this buffer into our write buffer
	LinkedContext.post( [ this, FinalPacket, Callback ] ()
	{
		// Create outgoing packet structure
		FOutgoingPacket OutPacket( FinalPacket.size() );
		std::move( FinalPacket.begin(), FinalPacket.end(), OutPacket.Buffer.begin() );
		OutPacket.Callback = Callback;
		
		// Put the message into the queue
		WriteBuffers.push_back( OutPacket );

		// If were not already in the process of sending data, then start
		if( !bWriteInProgress )
		{
			BeginWriting();
		}
	} );
	
	return true;
}

void RegCloudConnection::BeginWriting( bool bIsRecursiveCall )
{
	if( WriteBuffers.empty() || bKill )
	{
		return;
	}
	else if( !bIsRecursiveCall && bWriteInProgress )
	{
		return;
	}

	bWriteInProgress = true;

	LinkedContext.post( [ this ]()
	{
		asio::async_write( *Socket, asio::buffer( WriteBuffers.front().Buffer ), [ this ]( asio::error_code Error, unsigned int BytesSent )
		{
			// Pop message off from write buffer list
			if( WriteBuffers.empty() )
			{
				bWriteInProgress = false;
				return;
			}

			FOutgoingPacket& Packet = WriteBuffers.front();
			if( Packet.Callback )
			{
				Packet.Callback( Error, BytesSent );
			}
			WriteBuffers.pop_front();

			if( !WriteBuffers.empty() )
			{
				BeginWriting();
			}
			else
			{
				bWriteInProgress = false;
			}
		} );
	} );
}


void RegCloudConnection::OnSendDefault( asio::error_code Error, unsigned int BytesSent )
{
	if( Error )
	{
		cocos2d::log( "[Warning] Error sending data to RegSys! %s", Error.message().c_str() );
	}
}


void RegCloudConnection::OnReceive( asio::error_code ErrorCode, unsigned int NumBytes )
{
    // Check for Cancel, which is caused when certain operations are timed out
    if( ErrorCode == asio::error::operation_aborted )
    {
        // Reset buffer and start receiving again
        ReceiveBuffer.clear();
        LinkedContext.post( [ this ]()
        {
            Socket->async_read_some( asio::buffer( ReceiveBuffer ), [ this ]( asio::error_code ErrorCode, unsigned int NumBytes )
            {
               if( !this )
               {
                   cocos2d::log( "[ERROR] Failed to process incoming data from a RegSysNetNode because the connection object is null!" );
                   return;
               }
               
               this->OnReceive( ErrorCode, NumBytes );
            } );
        } );
        
        log( "[RegCloud] Connection was reset!" );
        return;
    }
	// Check for errors and disconnects
	if( ErrorCode || NumBytes <= 0 )
	{
		if( ErrorCode == asio::error::eof || ErrorCode == asio::error::connection_aborted || NumBytes <= 0 )
		{
			cocos2d::log( "[RegCloud] Disconnected from the server!" );
            
            // Close it down on our end
            CurrentState = ConnectionState::NotStarted;
            
            if( Socket )
                Socket->close();
            
            // Fire Event
            NullEventData Data;
            EventHub::Execute( "Disconnect", Data );
            
			return;
		}
		else
		{
			cocos2d::log( "[Warning] An error has occurred while reading data from the server! %s", ErrorCode.message().c_str() );
		}
	}

	if( bKill )
		return;

	// Pass data along to handler
	if( NumBytes > 0 )
	{
		OnData( ReceiveBuffer.begin(), ReceiveBuffer.begin() + NumBytes );
	}

	// Continue reading data if possible
	if( !Socket )
	{
		cocos2d::log( "[ERROR] Failed to continue receiving data from RegSys NetNode because the socket is null!" );
		return;
	}

	LinkedContext.post( [ this ]()
	{
		Socket->async_read_some( asio::buffer( ReceiveBuffer ), [ this ]( asio::error_code ErrorCode, unsigned int NumBytes )
		{
			if( !this )
			{
				cocos2d::log( "[ERROR] Failed to process incoming data from a RegSysNetNode because the connection object is null!" );
				return;
			}

			this->OnReceive( ErrorCode, NumBytes );
		} );
	} );
}


void RegCloudConnection::OnData( std::vector< uint8 >::iterator DataStart, std::vector< uint8 >::iterator DataEnd )
{
	// Debug Stuff
	const size_t DataSize = DataEnd - DataStart;
	cocos2d::log( "[RegSys Debug] Received data from the server! %d bytes", (int)DataSize );

	// Append data to the parse buffer
	ParseBuffer.insert( ParseBuffer.end(), DataStart, DataEnd );

	// Parse received data
	FIncomingPacket NewPacket;
	
	while( ConsumeBuffer( NewPacket ) )
	{
		if( DecryptPacket( NewPacket ) && ReadHeader( NewPacket ) )
		{
			// Check for ping, we want to respond ASAP
			if( NewPacket.CommandCode == ENetworkCommand::Ping )
			{
				if( !Send( NewPacket.Buffer, []( asio::error_code PingError, unsigned int bytesSent )
				{
					if( PingError )
						cocos2d::log( "[Warning] Failed to send ping packet to RegSys! Error Code: %d", PingError.value() );
					else
						cocos2d::log( "[DEBUG] Relayed ping packet!" );

				}, ENetworkEncryption::None ) )
				{
					cocos2d::log( "[Warning] Failed to relay a ping packet to RegSys!" );
				}
			}
			else
			{
				// Call any callbacks bound to this command code
				for( auto Entry : CallbackList )
				{
					if( Entry.second.Command == NewPacket.CommandCode )
					{
						// Were not going to do anything with the return value for now
						Entry.second.Func( NewPacket );
					}
				}
			}
		}
	}

}


bool RegCloudConnection::ConsumeBuffer( FIncomingPacket& OutPacket )
{
	// Check if theres data on the buffer
	size_t BufferSize = ParseBuffer.size();
	if( BufferSize <= 0 )
	{
		return false;
	}

	// Get commonly used structure sizes
	const size_t PublicHeaderSize = FPublicHeader::GetSize();
	
	// Check if we need to look for a new header, and if theres enough data for one
	if( CurrentHeader.PacketSize <= 0 && BufferSize >= PublicHeaderSize )
	{
		FPublicHeader ParsedHeader;

		if( !ParsedHeader.Serialize( ParseBuffer.begin(), ShouldFlipByteOrder() ) ||
			ParsedHeader.EncryptionMethod < 0 || ParsedHeader.EncryptionMethod > 2 ||
			ParsedHeader.PacketSize <= 0 )
		{
			cocos2d::log( "[WARNING] Invalid header found in buffer! Resetting parse buffer..." );
			CurrentHeader.PacketSize			= -1;
			CurrentHeader.EncryptionMethod		= -1;

			ParseBuffer.clear();
			return false;
		}

		CurrentHeader = ParsedHeader;

		// Pop header data off from the buffer and update buffer size
		ParseBuffer.erase( ParseBuffer.begin(), ParseBuffer.begin() + PublicHeaderSize );
		BufferSize = ParseBuffer.size();
	}

	const size_t PacketBodySize = CurrentHeader.PacketSize - PublicHeaderSize;

	// Check if we have enough data to parse the full packet
	if( BufferSize >= PacketBodySize )
	{
		// Move data into a new vector, clear out empty space from the first
		auto ParseBufferIter = ParseBuffer.begin();
		OutPacket.Buffer.clear();
		OutPacket.Buffer.resize( PacketBodySize );
		std::move( ParseBufferIter, ParseBufferIter + PacketBodySize, OutPacket.Buffer.begin() );
		ParseBuffer.erase( ParseBufferIter, ParseBufferIter + PacketBodySize );

		// Setup OutPacket's header values
		OutPacket.PacketHeader = CurrentHeader;

		// Reset current read header
		CurrentHeader.PacketSize		= -1;
		CurrentHeader.EncryptionMethod	= -1;
		
		return true;
	}

	return false;
}


bool RegCloudConnection::DecryptPacket( FIncomingPacket& OutPacket )
{
	const size_t PrivateHeaderSize = FPrivateHeader::GetSize();

	if( OutPacket.Buffer.size() <= 0 || OutPacket.PacketHeader.EncryptionMethod < 0 || 
		OutPacket.PacketHeader.EncryptionMethod > 2 || OutPacket.PacketHeader.PacketSize <= 0 )
	{
		cocos2d::log( "[Warning] RegCloudConnection packet decryption failed! Invalid packet given" );
		return false;
	}

	ENetworkEncryption Encryption = ENetworkEncryption( OutPacket.PacketHeader.EncryptionMethod );

	uint8* EncryptionKey	= nullptr;
	uint8* EncryptionIV		= nullptr;

	if( Encryption == ENetworkEncryption::Full )
	{
		if( !SessionKey )
		{
			cocos2d::log( "[Warning] Failed to decrypt secure packet because we have not established a session with the server!" );
			return false;
		}

		EncryptionKey	= SessionKey;
		EncryptionIV	= DefaultIV;
	}
	else if( Encryption == ENetworkEncryption::Light )
	{
		EncryptionKey	= DefaultKey;
		EncryptionIV	= DefaultIV;
	}
	else if( Encryption == ENetworkEncryption::None )
	{
		// No need for any decryption, so continue processing this packet
		return true;
	}
	else
	{
		cocos2d::log( "[Warning] Unknown encryption scheme found! Packet is being marked as invalid" );
		return false;
	}

	if( !EncryptionKey || !EncryptionIV )
	{
		cocos2d::log( "[Warning] Missing key needed to decrypt packet from the server!" );
		return false;
	}

	if( !CryptoLibrary::AesDecrypt( OutPacket.Buffer, EncryptionKey, EncryptionIV ) || OutPacket.Buffer.size() < PrivateHeaderSize )
	{
		cocos2d::log( "[Warning] Failed to decrypt incoming packet from the server! Decryption failed/returned no data!" );

		OutPacket.Buffer.clear();
		return false;
	}

	return true;
}


bool RegCloudConnection::ReadHeader( FIncomingPacket& OutPacket )
{
	int CommandCode = -1;

	uint8* VarStart = reinterpret_cast< uint8* >( std::addressof( CommandCode ) );
	
	// TODO: CHECK FOR ENDIANNESS
	bool bFlipOrder = false;
	if( bFlipOrder )
	{
		std::reverse_copy( OutPacket.Buffer.begin(), OutPacket.Buffer.begin() + 4, VarStart );
	}
	else
	{
		std::copy( OutPacket.Buffer.begin(), OutPacket.Buffer.begin() + 4, VarStart );
	}

	if( CommandCode < 0 )
	{
		cocos2d::log( "[Warning] Invalid command code skimmed off from incoming packet! Marking packet as invalid!" );
		return false;
	}

	OutPacket.CommandCode = ENetworkCommand( CommandCode );
	return true;
}


bool RegCloudConnection::RegisterCallback( ENetworkCommand CommandCode, std::string Identifier, std::function<bool( FIncomingPacket& )> Callback )
{
	if( CallbackExists( Identifier ) )
	{
		cocos2d::log( "[Warning] Failed to add new callback \"%s\" because one with this Id already exists!", Identifier.c_str() );
		return false;
	}
	else if( Identifier.length() < 3 )
	{
		cocos2d::log( "[Warning] Failed to add new callback \"%s\" because the supplied identifier was not 3 characters or more!", Identifier.c_str() );
		return false;
	}

	CallbackList.insert( std::pair< std::string, FNetCallback>( Identifier, FNetCallback( CommandCode, Identifier, Callback ) ) );
	return true;
}


bool RegCloudConnection::CallbackExists( std::string Identifier )
{
	return( CallbackList.find( Identifier ) != CallbackList.end() );
}


bool RegCloudConnection::RemoveCallback( std::string Identifier )
{
	auto Position = CallbackList.find( Identifier );

	if( Position == CallbackList.end() )
	{
		cocos2d::log( "[Warning] Failed to remove callback \"%s\" because it was not found!", Identifier.c_str() );
		return false;
	}

	CallbackList.erase( Position );
	return true;
}

void RegCloudConnection::SetSessionKey( uint8* inKey )
{
	if( !inKey )
	{
        if( SessionKey )
            delete[] SessionKey;
        
        SessionKey = nullptr;
		return;
	}

	if( SessionKey )
	{
		delete[] SessionKey;
	}
    
	SessionKey = inKey;
}
