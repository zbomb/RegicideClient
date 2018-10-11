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

	bool OnSessionResponse( FIncomingPacket& Packet );
	void OnSocketConnection( CloudEvent EventId, int Parameter );

	bool OnInvalidPacket( FIncomingPacket& Packet );

	enum LoginResult{ ConnectionError, Timeout, AlreadyLoggedIn, InvalidInput, InvalidCredentials, Success };
	void Login( std::string Username, std::string PasswordHash, std::function< void( RegCloud::LoginResult ) > Callback );

private:

	std::map< CloudEvent, std::vector< std::function< void( CloudEvent, unsigned int ) > > > CallbackList;
	void ExecuteEvent( CloudEvent bindEvent, int Parameter );

	void EstablishSession();
	bool GenerateExchangeKeys();

	std::function< void( LoginResult ) > LoginCallback;
	//bool OnLoginResponse( FIncomingPacket& Packet );

	std::shared_ptr< std::thread > TimeoutThread;
	enum class CurrentOperation { None, Session, Login, Register };
	CurrentOperation Operation = CurrentOperation::None;
	std::mutex TimeoutMutex;
	std::condition_variable TimeoutVar;

};