#include <memory>
#include <vector>
#include <deque>
#include <map>
#include <asio.hpp>
#include "NetHeaders.h"

#pragma once

// Settings
#define REGCLOUD_REMOTEADDRESS "network.regicidemobile.com"
#define REGCLOUD_PORT 8800
#define REGCLOUD_DEBUGADDRESS "127.0.0.1"
#define REGCLOUD_DEBUG
#define REGCLOUD_BUFFERLEN 2048

class FSockError;

using namespace asio::ip;

class RegCloudConnection
{
private:
	
	struct FNetCallback
	{
		ENetworkCommand Command;
		std::string Identifier;
		std::function< bool( FIncomingPacket& ) > Func;
		
		FNetCallback( ENetworkCommand inCmd, std::string inId, std::function< bool( FIncomingPacket& )> inFunc )
			: Command( inCmd ), Identifier( inId ), Func( inFunc )
		{
		}
	};

	struct FOutgoingPacket
	{
		std::vector< uint8 > Buffer;
		std::function< void( asio::error_code, unsigned int ) > Callback;

		FOutgoingPacket( const size_t BufferSize )
			: Buffer( BufferSize )
		{
		}
	};

	std::vector< uint8 > ReceiveBuffer;
	std::vector< uint8 > ParseBuffer;
	std::map< std::string, FNetCallback > CallbackList;
	FPublicHeader CurrentHeader;

	std::deque< FOutgoingPacket > WriteBuffers;
	bool bWriteInProgress = false;

	void BeginWriting( bool bIsRecursiveCall = false );

	void OnConnect( asio::error_code ErrorCode );
	void OnReceive( asio::error_code ErrorCode, unsigned int NumBytes );
	void OnData( std::vector< uint8 >::iterator DataStart, std::vector< uint8 >::iterator DataEnd );
	void OnSendDefault( asio::error_code Error, unsigned int BytesSent );

	bool ConsumeBuffer( FIncomingPacket& OutPacket );
	bool DecryptPacket( FIncomingPacket& OutPacket );
	bool ReadHeader( FIncomingPacket& OutPacket );

	enum EndianOrder { BigEndian = 0, LittleEndian = 1, NotSet = 2 };
	EndianOrder LocalByteOrder;

	uint8* SessionKey	= nullptr;
	uint8* DefaultKey	= nullptr;
	uint8* DefaultIV	= nullptr;

	asio::io_context LinkedContext;
	std::shared_ptr< tcp::socket > Socket;
	std::shared_ptr< std::thread > IOThread;
	bool bKill = false;

public:

	RegCloudConnection();
	~RegCloudConnection();

	void BeginConnect();
	bool IsConnected() const;
	bool Send( std::vector< uint8 >& Data, std::function< void( asio::error_code, unsigned int ) > Callback = nullptr, ENetworkEncryption EncryptionLevel = ENetworkEncryption::Full );

	template< typename T >
	bool SendPacket( FPrivateHeader& Packet, std::function< void( asio::error_code, unsigned int )> Callback );

	template< typename T >
	bool SendPacket( FPrivateHeader& Packet );

	bool RegisterCallback( ENetworkCommand CommandCode, std::string Identifier, std::function< bool( FIncomingPacket& )> Callback );
	bool CallbackExists( std::string Identifier );
	bool RemoveCallback( std::string Identifier );

	void FireEvent( CloudEvent Event, int Parameter );
	void SetSessionKey( uint8* SessionKey );

	inline bool ShouldFlipByteOrder() const { return LocalByteOrder != EndianOrder::LittleEndian; }

	enum ConnectionState { Connected, InProgress, NotStarted, Failed };
	inline ConnectionState GetState() const { return CurrentState; }

private:

	ConnectionState CurrentState = ConnectionState::NotStarted;


};


template< typename T >
bool RegCloudConnection::SendPacket( FPrivateHeader& Packet, std::function<void( asio::error_code, unsigned int )> Callback )
{
	if( !IsConnected() || bKill )
	{
		log( "[Warning] Failed to send packet to RegSys because the socket is not connected!" );
		return false;
	}

	const size_t PacketSize = T::GetSize();
	std::vector< uint8 > SerializedPacket( PacketSize );

	// Determine Endian Order
	uint16 TestValue = 0x0001;
	uint8* TestBytes = (uint8*) &TestValue;
	EEndianOrder Order = TestBytes[ 0 ] ? EEndianOrder::LittleEndian : EEndianOrder::BigEndian;

	if( !Packet.Serialize( SerializedPacket.begin(), true, false ) )
	{
		log( "[Warning] Failed to send packet to the server because serialization failed!" );
		return false;
	}

	log( "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" );

	return Send( SerializedPacket, Callback, Packet.EncryptionLevel() );
}

template< typename T >
bool RegCloudConnection::SendPacket( FPrivateHeader& Packet )
{
	return SendPacket< T >( Packet, nullptr );
}

