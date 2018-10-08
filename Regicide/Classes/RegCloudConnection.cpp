#include <memory>
#include <string>
#include <functional>
#include <vector>
#include "ISockSystem.h"
#include "RegCloudConnection.h"
#include "CryptoLibrary.h"


using namespace std::placeholders;

RegCloudConnection* SingletonInstance = nullptr;

RegCloudConnection::RegCloudConnection()
	: ReceiveBuffer( REGCLOUD_BUFFERLEN ), ParseBuffer()
{
	// Create Socket
	ISockSystem* SocketSystem = ISockSystem::Get();

	if( SocketSystem == nullptr )
	{
		printf( "[CRITICAL] Invalid socket system! Online features will not function!\n" );
	}
	else
	{
		Connection = std::shared_ptr< FSock >( SocketSystem->CreateSocket( ISOCK_TYPE::ISOCK_TCP ) );

		if( !Connection )
		{
			printf( "[CRITICAL] Socket system failed to create a new socket for RegCloud!\n" );
		}
		else
		{
			printf( "[RegCloud] Socket created successfully!\n" );
		}
	}

}

RegCloudConnection::~RegCloudConnection()
{
	if( Connection )
	{
		Connection->Close();
		Connection.reset();
	}

	ReceiveBuffer.clear();
	ParseBuffer.clear();

	ISockSystem::Close();
}

RegCloudConnection* RegCloudConnection::Get()
{
	if( SingletonInstance == nullptr )
	{
		SingletonInstance = new (std::nothrow) RegCloudConnection;
	}

	return SingletonInstance;
}

void RegCloudConnection::Shutdown()
{
	if( SingletonInstance )
	{
		delete SingletonInstance;
	}
}

void RegCloudConnection::BeginConnect()
{
	if( !Connection )
	{
		printf( "[CRITICAL] Failed to connect to RegCloud because the socket is null!\n" );
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
		printf( "[CRITICAL] RegCloud failed to resolve hostname.. please check supplied address.\n" );
		return;
	}

	// Bind a callback, and start the async connect
	auto Callback = std::bind( &RegCloudConnection::OnConnect, *this, _1 );
	Connection->ConnectAsync( Callback );

	printf( "[RegCloud] Connecting...\n" );
}

void RegCloudConnection::OnConnect( FSockError ErrorCode )
{
	if( ErrorCode )
	{
		printf( "[ERROR] Failed to connect to RegSys! %s\n", ErrorCode.GetErrorMessage().c_str() );
		return;
	}
	else if( !Connection )
	{
		printf( "[ERROR] Failed to continue receiving data because the socket is null!\n" );
		return;
	}

	printf( "[RegCloud] Connected to RegSys!\n" );

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
		printf( "[Warning] Failed to send data to RegSys because the socket is not connected/null!\n" );
		return false;
	}
	else if( Data.size() == 0 )
	{
		printf( "[Warning] Failed to send data to RegSys because the supplied buffer was empty!\n" );
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


bool RegCloudConnection::SendPacket( FPrivateHeader& Packet, std::function<void( FSockError, unsigned int )> Callback /* = nullptr */ )
{
	if( !IsConnected() )
	{
		printf( "[Warning] Failed to send packet to RegSys because the socket is not connected!\n" );
		return false;
	}
	
	const size_t PacketSize = Packet.GetSize();
	std::vector< uint8 > SerializedPacket( PacketSize );

	// TODO: Determine endianess 
	bool bFlipOrder = false;

	if( !Packet.Serialize( SerializedPacket.begin(), true, bFlipOrder ) )
	{
		printf( "[Warning] Failed to send packet to the server because serialization failed!\n" );
		return false;
	}

	return Send( SerializedPacket, Callback );
	
}


void RegCloudConnection::OnSendDefault( FSockError Error, unsigned int BytesSent )
{
	if( Error )
	{
		printf( "[Warning] Error sending data to RegSys! %s\n", Error.GetErrorMessage().c_str() );
	}
}


void RegCloudConnection::OnReceive( FSockError ErrorCode, unsigned int NumBytes, std::vector<uint8>& Data )
{
	// Check for errors and disconnects
	if( ErrorCode )
	{
		if( ErrorCode.IsDisconnect() )
		{
			printf( "[RegCloud] Disconnected from the server!\n" );
			return;
		}
		else
		{
			printf( "[Warning] An error has occurred while reading data from the server! %s\n", ErrorCode.GetErrorMessage().c_str() );
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
		printf( "[ERROR] Failed to continue receiving data from the server because the connection is null\n" );
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
	FIncomingPacket NewPacket;
	
	while( ConsumeBuffer( NewPacket ) )
	{
		if( DecryptPacket( NewPacket ) && ReadHeader( NewPacket ) )
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
	const size_t PrivateHeaderSize = FPrivateHeader::GetSize();

	// Check if we need to look for a new header, and if theres enough data for one
	if( CurrentHeader.PacketSize <= 0 && BufferSize >= PublicHeaderSize )
	{
		FPublicHeader ParsedHeader;

		if( !ParsedHeader.Serialize( ParseBuffer.begin() ) ||
			ParsedHeader.EncryptionMethod < 0 || ParsedHeader.EncryptionMethod > 2 ||
			ParsedHeader.PacketSize <= 0 )
		{
			printf( "[WARNING] Invalid header found in buffer! Resetting parse buffer...\n" );
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
		printf( "[Warning] RegCloudConnection packet decryption failed! Invalid packet given\n" );
		return false;
	}

	ENetworkEncryption Encryption = ENetworkEncryption( OutPacket.PacketHeader.EncryptionMethod );

	uint8* EncryptionKey	= nullptr;
	uint8* EncryptionIV		= nullptr;

	if( Encryption == ENetworkEncryption::Full )
	{
		if( !IsSecure() )
		{
			printf( "[Warning] Failed to decrypt secure packet because we have not established a session with the server!\n" );
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
		printf( "[Warning] Unknown encryption scheme found! Packet is being marked as invalid\n" );
		return false;
	}

	if( !CryptoLibrary::AesDecrypt( OutPacket.Buffer, EncryptionKey, EncryptionIV ) || OutPacket.Buffer.size() <= PrivateHeaderSize )
	{
		printf( "[Warning] Failed to decrypt incoming packet from the server! Decryption failed/returned no data!\n" );

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
		printf( "[Warning] Invalid command code skimmed off from incoming packet! Marking packet as invalid!" );
		return false;
	}

	OutPacket.CommandCode = ENetworkCommand( CommandCode );
	return true;
}


bool RegCloudConnection::RegisterCallback( std::function<bool( FIncomingPacket& )> Callback, ENetworkCommand CommandCode, std::string Identifier )
{
	if( CallbackExists( Identifier ) )
	{
		printf( "[Warning] Failed to add new callback \"%s\" because one with this Id already exists!\n", Identifier.c_str() );
		return false;
	}
	else if( Identifier.length() < 3 )
	{
		printf( "[Warning] Failed to add new callback \"%s\" because the supplied identifier was not 3 characters or more!\n", Identifier.c_str() );
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
		printf( "[Warning] Failed to remove callback \"%s\" because it was not found!\n", Identifier.c_str() );
		return false;
	}

	CallbackList.erase( Position );
	return true;
}
