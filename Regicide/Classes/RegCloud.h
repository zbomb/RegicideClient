#pragma once

#include "RegCloudConnection.h"
#include "rsa.h"
#include "ctr_drbg.h"
#include "NetHeaders.h"
//#include "EventHub.h"
#include "EventDataTypes.h"
#include <time.h>

struct Account
{
    uint32 Identifier;
    std::string Username;
    std::string EmailAddress;
    std::string DisplayName;
    uint64 Coins;
    bool Verified;
    std::time_t CreatedOn;
    std::time_t LastLogin;
};

struct Card
{
    uint16 Identifier;
    uint16 Count;
};

struct Deck
{
    uint32 Identifier;
    std::string DisplayName;
    
    std::vector< Card > Cards;
};

struct Achievement
{
    uint16 Identifier;
    uint16 State;
    float Progress;
};

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
    
    inline asio::io_context& GetContext() { return Connection.GetContext(); }

	bool IsConnected() const;
	bool IsSecure() const { return IsConnected() && SessionState == ENodeProcessState::Complete; }
	bool IsLoggedIn() const { return IsSecure() && LoginState == ENodeProcessState::Complete; }
	ENodeProcessState GetConnectionState() const;

	void Init();
    
	bool OnSessionResponse( FIncomingPacket& Packet );
	bool OnSocketConnection( EventData* EventId );
    bool OnSocketDisconnect( EventData* EventId );

	bool OnInvalidPacket( FIncomingPacket& Packet );

	void Login( std::string Username, std::string PasswordHash );
    void Logout( bool bInformServer = true );
    void Register( std::string& Username, std::string& Password, std::string& DispName, std::string& EmailAddr );

    void ExecuteBackgroundWork( std::function< void() > inWork, std::function< void() > onComplete = nullptr );
    
    bool CancelTimeout( std::string Identifier );
    bool UpdateTimeout( std::string Identifier, uint32 TimeoutFromNow );
    bool TimeoutExists( std::string Identifier );
    void CreateTimeout( std::string Identifier, std::function< void() > Callback );
    void CreateTimeout( std::string Identifier, std::function< void() > Callback, uint32 Milliseconds );
    bool IsTimedOut( std::string Identifier );
    
    bool RegisterPacketCallback( ENetworkCommand Command, std::string CallbackName, std::function< bool( FIncomingPacket& ) > Callback );
    
    bool Reconnect();
    
private:

	void EstablishSession();
	bool GenerateExchangeKeys();
    void ResetSession();

    std::shared_ptr< std::thread > BackgroundThread;
    void BackgroundThreadBody();
    struct BackgroundTask
    {
        std::function< void() > Work;
        std::function< void() > OnComplete;
    };
    std::deque< BackgroundTask > BackgroundQueue;
    bool bKillBackgroundThread = false;
    
    struct Operation
    {
        std::string Identifier;
        std::function< void() > Callback;
        std::chrono::time_point< std::chrono::steady_clock > Timeout;
        bool IsActive = false;
        bool TimedOut = false;
    };
    
    std::mutex WorkMutex;
    std::mutex TimeoutMutex;
    
    std::vector< Operation > CurrentOperations;

	bool HandleLoginResponse( FIncomingPacket& Packet );
    bool HandleRegisterResponse( FIncomingPacket& Packet );
	bool HandleAccountInfo( std::vector< uint8 >& RawAccountInfo );

    std::shared_ptr< Account > LocalAccount;
    std::shared_ptr< std::vector< Card > > LocalCards;
    std::shared_ptr< std::vector< Deck > > LocalDecks;
    std::shared_ptr< std::vector< Achievement > > LocalAchv;
    
    bool bRegisterInProgress = false;
    
    // Timeouts
    void ConnectTimeout();
    void SessionTimeout();
    void LoginTimeout();
    void RegisterTimeout();
    
public:
    

    
};
