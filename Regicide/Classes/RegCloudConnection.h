#pragma once

#include <memory>
#include <vector>
#include <deque>
#include <map>
#include <asio.hpp>
#include "NetHeaders.h"

// Settings
#define REGCLOUD_REMOTEADDRESS "34.195.8.40"
#define REGCLOUD_PORT 8800
#define REGCLOUD_DEBUGADDRESS "34.195.8.40"
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
    
    inline asio::io_context& GetContext() { return LinkedContext; }

	bool BeginConnect();
	bool IsConnected() const;
	bool Send( std::vector< uint8 >& Data, std::function< void( asio::error_code, unsigned int ) > Callback = nullptr, ENetworkEncryption EncryptionLevel = ENetworkEncryption::Full, uint32 Flags = 0 );

	template< typename T >
    bool SendPacket( FPrivateHeader& Packet, std::function< void( asio::error_code, unsigned int )> Callback, ENetworkEncryption EncryptionLevel = ENetworkEncryption::Full, uint32 Flags = 0 );

	template< typename T >
    bool SendPacket( FPrivateHeader& Packet, ENetworkEncryption Encryption = ENetworkEncryption::Full, uint32 Flags = 0 );
    
    template< typename T >
    bool SendDynamic( FPrivateHeader& StaticPacket, std::vector< uint8 >& DynamicData, ENetworkEncryption Encryption = ENetworkEncryption::Full, uint32 AddtlFlags = 0 );
    
    template< typename T >
    bool SendDynamic( FPrivateHeader& StaticPacket, std::vector< uint8 >& DynamicData, std::function< void( asio::error_code, unsigned int) > Callback, ENetworkEncryption Encryption = ENetworkEncryption::Full, uint32 AddtlFlags = 0 );

	bool RegisterCallback( ENetworkCommand CommandCode, std::string Identifier, std::function< bool( FIncomingPacket& )> Callback );
	bool CallbackExists( std::string Identifier );
	bool RemoveCallback( std::string Identifier );

	void SetSessionKey( uint8* SessionKey );

	enum ConnectionState { Connected, InProgress, NotStarted, Failed };
	inline ConnectionState GetState() const { return CurrentState; }
    
    void TimeoutConnection( bool bUpdateState );

private:

	ConnectionState CurrentState = ConnectionState::NotStarted;

};


template< typename T >
bool RegCloudConnection::SendPacket( FPrivateHeader& Packet, std::function<void( asio::error_code, unsigned int )> Callback, ENetworkEncryption Encryption, uint32 Flags )
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
    bool bLittleEndian = TestBytes[ 0 ];

    if( !Packet.Serialize( SerializedPacket.begin(), true, !bLittleEndian ) )
	{
		log( "[Warning] Failed to send packet to the server because serialization failed!" );
		return false;
	}

	return Send( SerializedPacket, Callback, Encryption, Flags );
}

template< typename T >
bool RegCloudConnection::SendPacket( FPrivateHeader& Packet, ENetworkEncryption Encryption, uint32 Flags )
{
	return SendPacket< T >( Packet, nullptr, Encryption, Flags );
}

template< typename T >
bool RegCloudConnection::SendDynamic( FPrivateHeader& StaticPacket, std::vector< uint8 >& DynamicData, std::function< void( asio::error_code, unsigned int ) > Callback, ENetworkEncryption Encryption, uint32 AddtlFlags  )
{
    if( !IsConnected() || bKill )
    {
        log( "[Warning] Failed to send packet to RegSys because the socket is not connected!" );
        return false;
    }
    
    const size_t PacketSize = T::GetSize();
    const size_t DynamicSize = DynamicData.size();
    
    std::vector< uint8 > FinalPacket( PacketSize + DynamicSize );
    
    // Determine endian order, for dynamic data, the caller will have to handle endian issues
    // But, all data needs to be sent using little endian
    uint16 TestValue = 0x0001;
    uint8* TestBytes = (uint8*) &TestValue;
    bool bLittleEndian = TestBytes[ 0 ];
    
    // Serialize static portion of data
    if( !StaticPacket.Serialize( FinalPacket.begin(), true, !bLittleEndian ) )
    {
        log( "[Warning] Failed to send dynamic packet to the server because serialization of the static packet failed!" );
        return false;
    }
    
    // Copy dynamic data into packet
    std::copy( DynamicData.begin(), DynamicData.end(), FinalPacket.begin() + PacketSize );
    
    // Set dynamic flag if the caller didnt already
    if( !( AddtlFlags & PACKET_FLAG_DYNAMIC ) )
    {
        AddtlFlags |= PACKET_FLAG_DYNAMIC;
    }
    
    return Send( FinalPacket, Callback, Encryption, AddtlFlags );
}

template< typename T >
bool RegCloudConnection::SendDynamic( FPrivateHeader& StaticPacket, std::vector< uint8 >& DynamicData, ENetworkEncryption Encryption, uint32 AddtlFlags )
{
    return SendDynamic< T >( StaticPacket, DynamicData, nullptr, Encryption, AddtlFlags );
}
