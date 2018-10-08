#pragma once
#include <memory>
#include <vector>
#include <map>
#include "NetHeaders.h"

// Settings
#define REGCLOUD_REMOTEADDRESS "network.regicidemobile.com"
#define REGCLOUD_PORT 880
#define REGCLOUD_DEBUGADDRESS "127.0.0.1"
#define REGCLOUD_DEBUG
#define REGCLOUD_BUFFERLEN 2048

class FSockError;

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

	std::shared_ptr< class FSock > Connection;
	std::vector< uint8 > ReceiveBuffer;
	std::vector< uint8 > ParseBuffer;
	std::map< std::string, FNetCallback > CallbackList;
	FPublicHeader CurrentHeader;
	

	void OnConnect( FSockError ErrorCode );
	void OnReceive( FSockError ErrorCode, unsigned int NumBytes, std::vector< uint8 >& Data );
	void OnData( std::vector< uint8 >& Data );
	void OnSendDefault( FSockError Error, unsigned int BytesSent );

	bool ConsumeBuffer( FIncomingPacket& OutPacket );
	bool DecryptPacket( FIncomingPacket& OutPacket );
	bool ReadHeader( FIncomingPacket& OutPacket );

	enum EndianOrder { BigEndian, LittleEndian };
	EndianOrder ByteOrder;

	uint8* SessionKey	= nullptr;
	uint8* DefaultKey	= nullptr;
	uint8* DefaultIV	= nullptr;

	ENodeProcessState ConnectionState	= ENodeProcessState::NotStarted;
	ENodeProcessState SessionState		= ENodeProcessState::NotStarted;
	ENodeProcessState LoginState		= ENodeProcessState::NotStarted;
	ENodeProcessState RegisterState		= ENodeProcessState::NotStarted;

public:

	RegCloudConnection();
	~RegCloudConnection();

	void BeginConnect();
	bool IsConnected() const;
	bool Send( std::vector< uint8 >& Data, std::function< void( FSockError, unsigned int ) > Callback = nullptr );
	bool SendPacket( FPrivateHeader& Packet, std::function< void( FSockError, unsigned int )> Callback = nullptr );

	bool IsSecure() const { return SessionState == ENodeProcessState::Complete && SessionKey && IsConnected(); }
	bool IsLoggedIn() const { return LoginState == ENodeProcessState::Complete && IsSecure(); }

	bool RegisterCallback( std::function< bool( FIncomingPacket& )> Callback, ENetworkCommand CommandCode, std::string Identifier );
	bool CallbackExists( std::string Identifier );
	bool RemoveCallback( std::string Identifier );
	
	static RegCloudConnection* Get();
	static void Shutdown();
};

