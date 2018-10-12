#include "RegCloudConnection.h"
#include "rsa.h"
#include "ctr_drbg.h"
#include "NetHeaders.h"

#pragma once

class RegCloud
{
	friend class RegCloudConnection;

private:

	RegCloudConnection Connection;

	// Context Objects for Encryption/Decryption
	mbedtls_rsa_context CryptoContext;
	mbedtls_ctr_drbg_context RandomContext;

	ENodeProcessState SessionState	= ENodeProcessState::NotStarted;
	ENodeProcessState LoginState	= ENodeProcessState::NotStarted;

public:

	static RegCloud* Get();
	static void Shutdown();

	RegCloud();
	~RegCloud();

	bool IsConnected() const;
	bool IsSecure() const { return IsConnected() && SessionState == ENodeProcessState::Complete; }
	bool IsLoggedIn() const { return IsSecure() && LoginState == ENodeProcessState::Complete; }
	ENodeProcessState GetConnectionState() const;

	inline bool DoesServerEndianMatch() const { return Connection.ShouldFlipByteOrder(); }

	void Init();

	void BindEvent( CloudEvent bindEvent, std::function< void( CloudEvent, unsigned int ) > Callback );
	bool BindListener( CloudEvent targetEvent, std::string UniqueId, std::function< void( CloudEvent, unsigned int ) > Callback );

	// Note: probablyBoundTo is not required, but, it will signal where
	// to look for this listener, and will greatly speed up this method
	bool ListenerExists( std::string UniqueId, CloudEvent probablyBoundTo );

	// Note: probablyBoundTo is not required, but, it will signal where
	// to look for this listener, and will greatly speed up this method
	bool RemoveListener( std::string UniqueId, CloudEvent probablyBoundTo );

	bool OnSessionResponse( FIncomingPacket& Packet );
	void OnSocketConnection( CloudEvent EventId, int Parameter );

	bool OnInvalidPacket( FIncomingPacket& Packet );

	enum LoginResult{ ConnectionError, Timeout, AlreadyLoggedIn, InvalidInput, InvalidCredentials, Success };
	void Login( std::string Username, std::string PasswordHash );

	inline const std::shared_ptr< FAccountInfo > GetLocalAccountInfo() const { return localAccountInfo; }

private:
	
	struct CloudEventListener
	{
		std::string identifier;
		std::function< void( CloudEvent, unsigned int ) > callback;
	};

	std::map< CloudEvent, std::vector< std::function< void( CloudEvent, unsigned int ) > > > CallbackList;
	std::map< CloudEvent, std::vector< CloudEventListener > > eventListeners;

	void ExecuteEvent( CloudEvent bindEvent, int Parameter );
	void FireEvent( CloudEvent eventType, int Parameter );

	void EstablishSession();
	bool GenerateExchangeKeys();

	//bool OnLoginResponse( FIncomingPacket& Packet );

	std::shared_ptr< std::thread > TimeoutThread;
	enum class CurrentOperation { None, Session, Login, Register };
	CurrentOperation Operation = CurrentOperation::None;
	std::mutex TimeoutMutex;
	std::condition_variable TimeoutVar;

	bool HandleLoginResponse( FIncomingPacket& Packet );
	bool
	
	HandleAccountInfo( std::vector< uint8 >& RawAccountInfo );

	std::shared_ptr< FAccountInfo > localAccountInfo;

};