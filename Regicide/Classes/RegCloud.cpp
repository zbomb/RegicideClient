#include <memory>
#include <functional>
#include <chrono>
#include "RegCloud.h"
#include "cocos2d.h"
#include "rsa.h"
#include "cipher.h"
#include "entropy.h"
#include "ctr_drbg.h"
#include "CryptoLibrary.h"
#include "EventHub.h"
#include "EventDataTypes.h"
#include "../snappy/snappy.h"
#include "external/json/document.h"


#define REGCLOUD_EXCHANGE_KEYSIZE 2048

using namespace cocos2d;
using namespace std::placeholders;

RegCloud* s_CloudInstance = nullptr;

RegCloud* RegCloud::Get()
{
	if( !s_CloudInstance )
	{
		s_CloudInstance = new (std::nothrow) RegCloud;

	}

	return s_CloudInstance;
}

void RegCloud::Shutdown()
{
	if( s_CloudInstance )
	{
		delete s_CloudInstance;
	}

	s_CloudInstance = nullptr;
}


RegCloud::RegCloud()
{
	// Initialize RSA Context
	mbedtls_rsa_init( &CryptoContext, MBEDTLS_PADDING_PKCS7, 0 );
	mbedtls_ctr_drbg_init( &RandomContext );

	// Bind Events
    EventHub::Bind( "_Sock_Connect_", std::bind( &RegCloud::OnSocketConnection, this, _1 ), CallbackThread::Network );

	Connection.RegisterCallback( ENetworkCommand::KeyExchange, "regcloud_exchange", std::bind( &RegCloud::OnSessionResponse, this, _1 ) );
	Connection.RegisterCallback( ENetworkCommand::Login, "regcloud_login", std::bind( &RegCloud::HandleLoginResponse, this, _1 ) );
	Connection.RegisterCallback( ENetworkCommand::Register, "regcloud_register", std::bind( &RegCloud::HandleRegisterResponse, this, _1 ) );
	
	BackgroundThread = std::make_shared< std::thread >( std::bind( &RegCloud::BackgroundThreadBody, this ) );
	BackgroundThread->detach();
	
	CreateTimeout( "connect", std::bind( &RegCloud::ConnectTimeout, this ) );
	CreateTimeout( "session", std::bind( &RegCloud::SessionTimeout, this ) );
	CreateTimeout( "Register", std::bind( &RegCloud::RegisterTimeout, this ) );
	CreateTimeout( "Login", std::bind( &RegCloud::LoginTimeout, this ) );
}

RegCloud::~RegCloud()
{
	// Free the RSA Context
	mbedtls_rsa_free( &CryptoContext );
	mbedtls_ctr_drbg_free( &RandomContext );
	
	// Kill Background Worker
	bKillBackgroundThread = true;
	if( BackgroundThread )
	{
		if( BackgroundThread->joinable() )
			BackgroundThread->join();
		
		BackgroundThread.reset();
	}
}

bool RegCloud::RegisterPacketCallback( ENetworkCommand Command, std::string CallbackName, std::function<bool (FIncomingPacket &)> Callback )
{
	return Connection.RegisterCallback( Command, CallbackName, Callback );
}

bool RegCloud::IsConnected() const
{
	return Connection.IsConnected();
}

ENodeProcessState RegCloud::GetConnectionState() const
{
	auto SocketState = Connection.GetState();

	// Determine Socket State
	if( SocketState == RegCloudConnection::NotStarted )
	{
		return ENodeProcessState::NotStarted;
	}
	else if( SocketState == RegCloudConnection::Failed )
	{
		return ENodeProcessState::Reset;
	}
	else if( SocketState == RegCloudConnection::InProgress )
	{
		return ENodeProcessState::InProgress;
	}

	// SocketState is equal to Connected
	if( SessionState == ENodeProcessState::NotStarted )
	{
		return ENodeProcessState::InProgress;
	}
	else
	{
		return SessionState;
	}
}

void RegCloud::Init()
{
	if( !Connection.IsConnected() )
	{
		UpdateTimeout( "connect", 12000 );
		Connection.BeginConnect();
	}
}

bool RegCloud::OnSocketConnection( EventData* inEvent )
{
	// This event means the underlying socket connection was successful
	CancelTimeout( "connect" );
	EstablishSession();
    return true;
}

void RegCloud::ConnectTimeout()
{
	// Inform underlying connection
	Connection.TimeoutConnection( true );

	NumericEventData Data( (int) ConnectResult::ConnectionError );
	EventHub::Execute( "ConnectResult", Data );
}

void RegCloud::SessionTimeout()
{
	Connection.TimeoutConnection( true );
	
	SessionState = ENodeProcessState::Reset;
	NumericEventData Data( (int) ConnectResult::ConnectionError );
	EventHub::Execute( "ConnectResult", Data );
}

void RegCloud::LoginTimeout()
{
	//Connection.TimeoutConnection( false );
	
	LoginState = ENodeProcessState::Reset;
	NumericEventData Data( (int) LoginResult::Timeout );
	EventHub::Execute( "LoginResult", Data );
}

void RegCloud::RegisterTimeout()
{
	Connection.TimeoutConnection( false );
	
	bRegisterInProgress = false;
	NumericEventData Data( (int) ERegisterResult::ConnectionError );
	EventHub::Execute( "RegisterResult", Data );
}

void RegCloud::EstablishSession()
{
	// Send session request to the server
	if( !Connection.IsConnected() )
	{
		log( "[ERROR] Failed to establish session with RegSys because we are disconnected!" );
        NumericEventData Data( (int) ConnectResult::ConnectionError );
        EventHub::Execute< NumericEventData >( "ConnectResult", Data );
		return;
	}
	else if( SessionState == ENodeProcessState::Complete )
	{
		log( "[Warning] (RegSys) Attempt to establish a session while the current session is still valid!" );
		return;
	}
	
	SessionState = ENodeProcessState::InProgress;

	FKeyExchangePacket Packet;
	Packet.NetworkCommand	= (uint32) ENetworkCommand::KeyExchange;
	Packet.NetworkArgument	= (uint32) EKeyExchangeArgument::InitialRequest;

	// Generate RSA key
	if( !GenerateExchangeKeys() )
	{
		SessionState = ENodeProcessState::Reset;
        NumericEventData Data( (int)ConnectResult::ConnectionError );
        EventHub::Execute< NumericEventData >( "ConnectResult", Data );
		return;
	}

	// Export RSA key from mbedtls
	std::vector< uint8 > Modulus( 256 );
	std::vector< uint8 > Exponent( 4 );

	if( mbedtls_rsa_export_raw( &CryptoContext, Modulus.data(), Modulus.size(), NULL, 0, NULL, 0, NULL, 0, Exponent.data(), Exponent.size() ) != 0 )
	{
		log( "[ERROR] RegSys failed to establish session because the exchange key could not be exported!" );

		SessionState = ENodeProcessState::Reset;
		NumericEventData Data( (int) ConnectResult::KeyExchangeError );
        EventHub::Execute< NumericEventData >( "ConnectResult", Data );
		return;
	}

	// Move key data into the packet and clear original key buffers
	try
	{
		std::move( Modulus.begin(), Modulus.end(), Packet.Key );
		std::move( Exponent.begin(), Exponent.end(), Packet.Key + 256 );
	}
	catch( std::exception& Ex )
	{
		log( "[ERROR] RegSys failed to establish session because an exception was thrown while moving key data into the request packet! %s", Ex.what() );

		SessionState = ENodeProcessState::Reset;
		NumericEventData Data( (int) ConnectResult::KeyExchangeError );
        EventHub::Execute< NumericEventData >( "ConnectResult", Data );
		return;
	}

	Modulus.clear();
	Exponent.clear();

	// Finally, send the packet to RegSys to create the session
	if( !Connection.SendPacket< FKeyExchangePacket >( Packet, [ this ]( asio::error_code SendError, unsigned int NumBytes )
	{
		if( SendError )
		{
			log( "[ERROR] Failed to send session request to RegSys! %s", SendError.message().c_str() );

			SessionState = ENodeProcessState::Reset;
			NumericEventData Data( (int) ConnectResult::KeyExchangeError );
            EventHub::Execute< NumericEventData >( "ConnectResult", Data );
		}
		else
		{
			UpdateTimeout( "session", 12000 );
			log( "[RegSys] Sent session request to server!" );
		}
	} ) )
	{
		log( "[ERROR] Failed to send session request to RegSys!" );

		SessionState = ENodeProcessState::Reset;
		NumericEventData Data( (int) ConnectResult::KeyExchangeError );
        EventHub::Execute< NumericEventData >( "ConnectResult", Data );
	}
}

bool RegCloud::GenerateExchangeKeys()
{
	
	mbedtls_entropy_context EntropyContext;
	mbedtls_entropy_init( &EntropyContext );

	// Generate Random Seed
	auto randchar = []() -> char
	{
		const char charset[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";
		const size_t max_index = ( sizeof( charset ) - 1 );
		return charset[ rand() % max_index ];
	};
	std::string seedStr( 32, 0 );
	std::generate_n( seedStr.begin(), 32, randchar );

	if( mbedtls_ctr_drbg_seed( &RandomContext, mbedtls_entropy_func, &EntropyContext, (const unsigned char*) seedStr.data(), seedStr.size() ) != 0 )
	{
		log( "[ERROR] RegSys failed to establish session because an exchange key could not be generated (rng seed failed)" );

		mbedtls_entropy_free( &EntropyContext );
		return false;
	}

	// Generate and verify RSA key
	if( mbedtls_rsa_gen_key( &CryptoContext, mbedtls_ctr_drbg_random, &RandomContext, REGCLOUD_EXCHANGE_KEYSIZE, 65537 ) != 0 )
	{
		log( "[ERROR] RegSys failed to establish session because an exchange key could not be generated (key gen failed)" );

		mbedtls_entropy_free( &EntropyContext );
		return false;
	}

	mbedtls_entropy_free( &EntropyContext );

	return true;
}


bool RegCloud::OnSessionResponse( FIncomingPacket& Packet )
{
	// Check if timed out
	if( IsTimedOut( "session" ) )
	{
		log( "[RegSys] Received session response after it timed out" );
		return true;
	}
	
	// Deserialize Packet
	CancelTimeout( "session" );
	FKeyExchangePacket Response;

	if( !FStructHelper::Deserialize< FKeyExchangePacket >( Response, Packet.Buffer.begin(), Packet.Buffer.size() ) )
	{
		log( "[ERROR] Failed to deseraialize session request!" );
		SessionState = ENodeProcessState::Reset;
		NumericEventData Data( (int) ConnectResult::KeyExchangeError );
        EventHub::Execute( "ConnectResult", Data );
		return true;
	}

	auto Result = Response.NetworkArgument;
	if( Result != (uint32) EKeyExchangeArgument::Success )
	{
		log( "[ERROR] Failed to establish session with RegSys! Server couldnt process our request. Error Code: %d", (int) Result );
		SessionState = ENodeProcessState::Reset;
		NumericEventData Data( (int) ConnectResult::KeyExchangeError );
        EventHub::Execute( "ConnectResult", Data );
		return true;
	}

	// Decrypt Response
	if( mbedtls_rsa_check_privkey( &CryptoContext ) != 0 )
	{
		log( "[ERROR] Failed to establish session with RegSys! Received valid response but the key needed to decrypt is missing!" );
		SessionState = ENodeProcessState::Reset;
		NumericEventData Data( (int) ConnectResult::KeyExchangeError );
        EventHub::Execute( "ConnectResult", Data );
		return true;
	}

	std::vector< uint8 > DecryptedData( 512 ); // Must be the same size as FKeyExchangePacket.Key
	size_t OutputSize = 256;
	int DecryptResult = -1;

	try
	{
		DecryptResult = mbedtls_rsa_rsaes_pkcs1_v15_decrypt( &CryptoContext, mbedtls_ctr_drbg_random, &RandomContext, MBEDTLS_RSA_PRIVATE, &OutputSize, Response.Key, DecryptedData.data(), DecryptedData.size() );
	}
	catch( ... )
	{
		log( "[ERROR] An exception was thrown while decrypting session result from RegSys!" );
	}
	
	if( DecryptResult != 0 )
	{
		log( "[ERROR] Failed to establish session with RegSys! Decryption of response failed! Code: %d", DecryptResult );
		SessionState = ENodeProcessState::Reset;
		NumericEventData Data( (int) ConnectResult::KeyExchangeError );
        EventHub::Execute( "ConnectResult", Data );
		return true;
	}
	
	if( OutputSize < 32 )
	{
		log( "[ERROR] Failed to establish session with RegSys! Decrypted request response was less than the minimum size!" );
		SessionState = ENodeProcessState::Reset;
		NumericEventData Data( (int) ConnectResult::KeyExchangeError );
        EventHub::Execute( "ConnectResult", Data );
		return true;
	}

	// Resize decrypted data buffer, it should be much much smaller than 512
	DecryptedData.resize( OutputSize );

	// Read Session Key, connection will free memory on destructor
	uint8* SessionKey = new uint8[ 32 ];
	std::move( DecryptedData.begin(), DecryptedData.begin() + 32, SessionKey );

	Connection.SetSessionKey( SessionKey );

	log( "[DEBUG] Established secure session with RegSys!" );
	SessionState = ENodeProcessState::Complete;
	NumericEventData Data( (int) ConnectResult::Success );
    EventHub::Execute( "ConnectResult", Data );
	return true;
}


bool RegCloud::OnInvalidPacket( FIncomingPacket& Packet )
{
	FGenericPacket GenericPacket;
    
	if( !FStructHelper::Deserialize< FGenericPacket >( GenericPacket, Packet.Buffer.begin(), Packet.Buffer.size() ) )
	{
		log( "[ERROR] Received message indicating we sent an invalid packet.. but the servers packet was invalid?" );
		return true;
	}

	if( GenericPacket.NetworkArgument == (uint32) EInvalidPacketArgument::Error )
	{
		log( "[Warning] The server informed us that we sent an invalid packet!" );
	}
	else if( GenericPacket.NetworkArgument == (uint32) EInvalidPacketArgument::InvalidCommand )
	{
		log( "[Warning] The server informed us that we sent a packet with an invalid command!" );
	}
	else if( GenericPacket.NetworkArgument == (uint32) EInvalidPacketArgument::InvalidEncryption )
	{
		log( "[Warning] The server informed us that we sent a packet with an invalid encryption scheme" );
	}
	else if( GenericPacket.NetworkArgument == (uint32) EInvalidPacketArgument::RenewLogin )
	{
		log( "[Warning] The server informed us that we must re-login!" );
	}
	else
	{
		log( "[Warning] The server informed us of an invalid packet, but the details are unknown!" );
	}
    
    return true;
}

void RegCloud::Login( std::string Username, std::string Password )
{
	// Check if this operation is already in progress, or complete
	if( LoginState == ENodeProcessState::InProgress )
	{
		log( "[RegSys] Attempt to login, but login was already in progress!" );
		return;
	}
	else if( bRegisterInProgress )
	{
		log( "[RegLogin] Cant login while an account registration is in progress" );
		
		NumericEventData Data( (int) LoginResult::ConnectionError );
		EventHub::Execute( "LoginResult", Data );
		return;
	}
	else if( LoginState == ENodeProcessState::Complete )
	{
		log( "[RegSys] Attempt to login, but user is already logged in! Must log out before attempting another login!" );
		
        NumericEventData Data( (int) LoginResult::AlreadyLoggedIn );
        EventHub::Execute( "LoginResult", Data );
		return;
	}
	
	if( !IsSecure() )
	{
		log( "[RegSys] Attempt to login, but there is no active session! Reconnect and retry!" );
		
        NumericEventData Data( (int) LoginResult::ConnectionError );
        EventHub::Execute( "LoginResult", Data );
		return;
	}

	// Copy password into a buffer, and salt
	std::vector< uint8 > PasswordData( Password.size() + 5 );
	std::copy( Password.begin(), Password.end(), PasswordData.begin() );
	std::copy( Username.rbegin(), Username.rbegin() + 5, PasswordData.end() - 5 );

	// Triple hash the salted password
	for( int i = 0; i < 3; i++ )
	{
		PasswordData = CryptoLibrary::SHA256( PasswordData );
		if( PasswordData.size() == 0 )
		{
			log( "[RegSys Login] An error occurred while hashing! Please re-open login menu and try again!" );
			
            NumericEventData Data( (int) LoginResult::InvalidInput );
            EventHub::Execute( "LoginResult", Data );
            return;
		}
	}

	// Ensure data is the correct size
	if( PasswordData.size() != 32 || Username.size() > 128 )
	{
		log( "[RegSys Login] Check inputs. Username must be between 5 and 32 characters." );
		
        NumericEventData Data( (int) LoginResult::InvalidInput );
        EventHub::Execute( "LoginResult", Data );
        return;
	}

	// Copy everything into the packet
	FLoginPacket Packet = FLoginPacket();

	std::copy( Username.begin(), Username.end(), std::begin( Packet.Username ) );
	std::copy( PasswordData.begin(), PasswordData.end(), std::begin( Packet.Password ));

	Packet.NetworkCommand = (uint32) ENetworkCommand::Login;
	Packet.NetworkArgument = 0;

	// Attempt to send packet to service
	if( !Connection.SendPacket< FLoginPacket >( Packet, [ = ]( asio::error_code SendErr, unsigned int NumBytes )
	{
		if( SendErr )
		{
			log( "[RegSys Login] Failed to send login packet to service! Error: %s", SendErr.message().c_str() );
            NumericEventData Data( (int) LoginResult::ConnectionError );
            EventHub::Execute( "LoginResult", Data );
			CancelTimeout( "Login" );
        }
	} ) )
	{
		log( "[RegSys Login] Failed to send login packet to service! Unknown error.." );
		
        NumericEventData Data( (int) LoginResult::ConnectionError );
        EventHub::Execute( "LoginResult", Data );
        return;
	}

	// Update state
	LoginState = ENodeProcessState::InProgress;
	UpdateTimeout( "Login", 12000 );
}


bool RegCloud::HandleLoginResponse( FIncomingPacket& Packet )
{
	// Check if the operation timed out
	if( IsTimedOut( "Login" ) )
	{
		log( "[RegLogin] Received login response after timeout completed" );
		return true;
	}
	
	// Cancel Timeout
	CancelTimeout( "Login" );
	FGenericPacket ResponsePacket = FGenericPacket();

	if( !FStructHelper::Deserialize< FGenericPacket >( ResponsePacket, Packet.Buffer.begin(), Packet.Buffer.size() ) )
	{
		log( "[RegSys Login] Error! Received invalid login response from the server!" );

		LoginState = ENodeProcessState::Reset;
		
        NumericEventData Data( (int) LoginResult::ConnectionError );
        EventHub::Execute( "LoginResult", Data );
		return true;
	}

	if( ResponsePacket.NetworkArgument == (uint32) ELoginResponse::Error )
	{
		log( "[RegSys Login] Warning! An error occurred on the server while processing login request!" );

		LoginState = ENodeProcessState::Reset;
		
        NumericEventData Data( (int) LoginResult::ConnectionError );
        EventHub::Execute( "LoginResult", Data );
        return true;
	}
	else if( ResponsePacket.NetworkArgument == (uint32) ELoginResponse::InvalidCredentials )
	{
		log( "[RegSys Login] Warning! Login attempt failed because the credentials were not correct!" );

		LoginState = ENodeProcessState::Reset;
		
        NumericEventData Data( (int) LoginResult::InvalidCredentials );
        EventHub::Execute( "LoginResult", Data );
        return true;
	}
	else
	{
		// Success! We need to deseraialize the full packet now, and handle the parsing of this data
		if( HandleAccountInfo( Packet.Buffer ) )
		{
			LoginState = ENodeProcessState::Complete;
			
            NumericEventData Data( (int) LoginResult::Success );
            EventHub::Execute( "LoginResult", Data );
        }
		else
		{
			LoginState = ENodeProcessState::Reset;
			
            NumericEventData Data( (int) LoginResult::ConnectionError );
            EventHub::Execute( "LoginResult", Data );
        }

		return true;
	}
}

bool RegCloud::HandleAccountInfo( std::vector< uint8 >& RawAccountInfo )
{
	// We need to remove header data, then we will handle the dynamic portion
	if( RawAccountInfo.size() < sizeof( FPrivateHeader ) )
	{
		log( "[RegSys] Login Failed! Buffer Size < Header Size?" );
		return false;
	}
	
	RawAccountInfo.erase( RawAccountInfo.begin(), RawAccountInfo.begin() + sizeof( FPrivateHeader ) );
	
	// Decompress
	std::string Output;
	size_t CompressedSize = snappy::Uncompress( (const char*)RawAccountInfo.data(), RawAccountInfo.size(), &Output );

	if( CompressedSize == 0 || Output.size() == 0 )
	{
		log( "[RegSys] Login Failed! Couldnt decompress data!" );
		return false;
	}
	
	// Now we need to parse string with json to get the account data structure
	rapidjson::Document Doc;
	Doc.Parse<0>( Output.c_str() );
	
	// Get Basic Account Info
	if( !Doc.HasMember( "Account" ) || !Doc.HasMember( "Cards" ) )
	{
		log( "[RegSys] Login Failed! Couldnt parse data. Missing basic info or cards" );
		return false;
	}
	
	auto AccountInfo = Doc[ "Account" ].GetObject();
	Account LoadedAccount = Account();
	
	// Ensure we have all the data fields we need
	if( !AccountInfo.HasMember( "Identifier" ) || !AccountInfo.HasMember( "Username" ) || !AccountInfo.HasMember( "EmailAddress" ) ||
	   !AccountInfo.HasMember( "DisplayName" ) || !AccountInfo.HasMember( "Coins" ) || !AccountInfo.HasMember( "Verified" ) )
	{
		log( "[RegSys] Login Failed! Couldnt parse data. AccountInfo was missing fields." );
		return false;
	}
	
	LoadedAccount.Identifier = AccountInfo[ "Identifier" ].GetUint();
	LoadedAccount.Username = AccountInfo[ "Username" ].GetString();
	LoadedAccount.EmailAddress = AccountInfo[ "EmailAddress" ].GetString();
	LoadedAccount.DisplayName = AccountInfo[ "DisplayName" ].GetString();
	LoadedAccount.Coins = AccountInfo[ "Coins" ].GetUint64();
	LoadedAccount.Verified = AccountInfo[ "Verified" ].GetBool();
	
	// Get Card List
	auto CardList = Doc[ "Cards" ].GetArray();
	std::vector< Card > Cards;
	
	for( auto Iter = CardList.begin(); Iter != CardList.End(); Iter++ )
	{
		auto CardObj = Iter->GetObject();
		if( CardObj.HasMember("Id" ) && CardObj.HasMember( "Count" ) )
		{
			Card NewCard = Card();
			NewCard.Identifier = CardObj[ "Identifier" ].GetUint();
			NewCard.Count = CardObj[ "Identifier" ].GetUint();
			
			Cards.push_back( NewCard );
		}
		else
		{
			log( "[RegSys] Warning. While reading account info, an invalid card was found in list" );
		}
	}
	
	// Get Deck List
	auto DeckList = Doc[ "Decks" ].GetArray();
	std::vector< Deck > Decks;
	
	for( auto Iter = DeckList.begin(); Iter != DeckList.end(); Iter++ )
	{
		auto DeckObj = Iter->GetObject();
		
		if( DeckObj.HasMember( "Id" ) && DeckObj.HasMember( "Name" ) && DeckObj.HasMember( "Cards" ) )
		{
			Deck NewDeck = Deck();
			NewDeck.Identifier = DeckObj[ "Id" ].GetUint();
			NewDeck.DisplayName = DeckObj[ "Name" ].GetString();
			
			auto DeckCards = DeckObj[ "Cards" ].GetArray();
			for( auto CardIter = DeckCards.begin(); CardIter != DeckCards.end(); CardIter++ )
			{
				auto CardObj = CardIter->GetObject();
				
				if( CardObj.HasMember( "Id" ) && CardObj.HasMember( "Name" ) )
				{
					Card NewCard = Card();
					NewCard.Identifier = CardObj[ "Id" ].GetUint();
					NewCard.Count = CardObj[ "Count" ].GetUint();
					
					NewDeck.Cards.push_back( NewCard );
				}
				else
				{
					log( "[RegSys] Warning! Invalid card found in deck %s while reading account info!", NewDeck.DisplayName.c_str() );
				}
			}
		}
		else
		{
			log( "[RegSys] Warning! Invalid deck found in account info!" );
		}
	}
	
	// Get Achievement List
	std::vector< Achievement > AchvList;
	
	if( Doc.HasMember( "Achv" ) )
	{
		auto AchvObj = Doc[ "Achv" ].GetArray();
		
		for( auto Iter = AchvObj.begin(); Iter != AchvObj.end(); Iter++ )
		{
			auto Achv = Iter->GetObject();
			
			if( Achv.HasMember( "Id" ) && Achv.HasMember( "State" ) && Achv.HasMember( "Prog" ) )
			{
				Achievement newAchv = Achievement();
				newAchv.Identifier = Achv[ "Id" ].GetUint();
				newAchv.State = Achv[ "State" ].GetInt();
				newAchv.Progress = Achv[ "Prog" ].GetFloat();
				
				AchvList.push_back( newAchv );
			}
		}
	}
	
	// Set Local Members
	LocalAccount 	= std::make_shared< Account >( LoadedAccount );
	LocalCards 		= std::make_shared< std::vector< Card > >( Cards );
	LocalDecks 		= std::make_shared< std::vector< Deck > >( Decks );
	LocalAchv 		= std::make_shared< std::vector< Achievement > >( AchvList );
	
	log( "[Login] Successfully logged in! Welcome back %s", LocalAccount.get()->Username.c_str() );
	
    return true;
}

void RegCloud::ExecuteBackgroundWork( std::function< void() > inWork, std::function< void() > onComplete /* = nullptr */ )
{
	if( !inWork )
	{
		log( "[RegSys] Failed to execute background work because the provided function was null!" );
		return;
	}
	
	BackgroundTask newTask;
	newTask.Work = inWork;
	newTask.OnComplete = onComplete;
	
	WorkMutex.lock();
	BackgroundQueue.push_back( newTask );
	WorkMutex.unlock();
}

bool RegCloud::CancelTimeout( std::string Identifier )
{
	TimeoutMutex.lock();
	for( auto Iter = CurrentOperations.begin(); Iter != CurrentOperations.end(); Iter++ )
	{
		if( Iter->Identifier == Identifier )
		{
			CurrentOperations.erase( Iter );
			TimeoutMutex.unlock();
			return true;
		}
	}
	TimeoutMutex.unlock();
	
	return false;
}

bool RegCloud::UpdateTimeout( std::string Identifier, uint32 NewTimeout )
{
	TimeoutMutex.lock();
	for( auto Iter = CurrentOperations.begin(); Iter != CurrentOperations.end(); Iter++ )
	{
		if( Iter->Identifier == Identifier )
		{
			Iter->Timeout 	= std::chrono::steady_clock::now() + std::chrono::milliseconds( NewTimeout );
			Iter->IsActive 	= true;
			Iter->TimedOut 	= false;
			TimeoutMutex.unlock();
			return true;
		}
	}
	TimeoutMutex.unlock();
	
	return false;
}

bool RegCloud::TimeoutExists( std::string Identifier )
{
	TimeoutMutex.lock();
	for( auto Iter = CurrentOperations.begin(); Iter != CurrentOperations.end(); Iter++ )
	{
		if( Iter->Identifier == Identifier )
		{
			TimeoutMutex.unlock();
			return true;
		}
	}
	TimeoutMutex.unlock();
	
	return false;
}

void RegCloud::CreateTimeout( std::string Identifier, std::function< void() > Callback )
{
	// Check for existing operation
	TimeoutMutex.lock();
	for( auto Iter = CurrentOperations.begin(); Iter != CurrentOperations.end(); Iter++ )
	{
		if( Iter->Identifier == Identifier )
		{
			Iter->Callback = Callback;
			TimeoutMutex.unlock();
			return;
		}
	}
	
	Operation newOperation;
	newOperation.Identifier = Identifier;
	newOperation.Callback = Callback;
	newOperation.IsActive = false;
	newOperation.TimedOut = false;
	
	CurrentOperations.push_back( newOperation );
	TimeoutMutex.unlock();
}

void RegCloud::CreateTimeout( std::string Identifier, std::function< void() > Callback, uint32 Milliseconds )
{
	TimeoutMutex.lock();
	for( auto Iter = CurrentOperations.begin(); Iter != CurrentOperations.end(); Iter++ )
	{
		if( Iter->Identifier == Identifier )
		{
			Iter->Callback = Callback;
			Iter->Timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds( Milliseconds );
			Iter->IsActive = true;
			Iter->TimedOut = false;
			TimeoutMutex.unlock();
			return;
		}
	}
	
	Operation newOperation;
	newOperation.Identifier = Identifier;
	newOperation.Callback = Callback;
	newOperation.Timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds( Milliseconds );
	newOperation.IsActive = true;
	newOperation.TimedOut = false;
	
	CurrentOperations.push_back( newOperation );
	TimeoutMutex.unlock();
}

bool RegCloud::IsTimedOut( std::string Identifier )
{
	TimeoutMutex.lock();
	for( auto Iter = CurrentOperations.begin(); Iter != CurrentOperations.end(); Iter++ )
	{
		if( Iter->Identifier == Identifier )
		{
			bool Result = Iter->TimedOut;
			TimeoutMutex.unlock();
			return Result;
		}
	}
	TimeoutMutex.unlock();
	
	return false;
}

void RegCloud::BackgroundThreadBody()
{
	while( true )
	{
		// Check if we should die
		if( bKillBackgroundThread )
			return;
		
		std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
		
		if( bKillBackgroundThread )
			return;
		
		// Check for any operation timeouts
		TimeoutMutex.lock();
		for( auto Iter = CurrentOperations.begin(); Iter != CurrentOperations.end(); Iter++ )
		{
			if( Iter->IsActive )
			{
				if( Iter->Timeout <= std::chrono::steady_clock::now() )
				{
					// Run the callback and reset this operation
					Iter->Callback();
					Iter->IsActive = false;
					Iter->TimedOut = false;
				}
			}
		}
		TimeoutMutex.unlock();
		
		// Run Background Tasks
		WorkMutex.lock();
		while( !BackgroundQueue.empty() )
		{
			BackgroundTask NextWork = BackgroundQueue.front();
			WorkMutex.unlock();

			NextWork.Work();
			NextWork.OnComplete();
			
			WorkMutex.lock();
			BackgroundQueue.pop_front();
		}
		WorkMutex.unlock();
	}
}

void RegCloud::Register( std::string &Username, std::string &Password, std::string &DispName, std::string &EmailAddr )
{
	// Input should already be validated
	if( bRegisterInProgress )
	{
		log( "[RegLogin] Attempt to register a new account, but a register operation is in progress" );
		return;
	}
	else if( LoginState == ENodeProcessState::InProgress )
	{
		log( "[RegLogin] Attempt to register a new account while login is in progress." );
		NumericEventData Data( (int) ERegisterResult::ConnectionError );
		EventHub::Execute( "RegisterResult", Data );
		return;
	}
	else if( !IsSecure() )
	{
		log( "[RegLogin] Attempt to register a new account, but you are not connected!" );
		NumericEventData Data( (int) ERegisterResult::ConnectionError );
		EventHub::Execute( "RegisterResult", Data );
		return;
	}

	// Quick length checks, on the actual data size, not string length
	auto UserSize = Username.size();
	auto PassSize = Password.size();
	auto DispSize = DispName.size();
	auto EmailSize = EmailAddr.size();
	
	if( UserSize < Regicide::REG_USERNAME_MINLEN || UserSize > 128 ||
	    DispSize < Regicide::REG_DISPNAME_MINLEN || DispSize > 256 ||
	    EmailSize < Regicide::REG_EMAIL_MINLEN || EmailSize > 1024 ||
	    PassSize < Regicide::REG_PASSWORD_MINLEN )
	{
		log( "[RegLogin] Register Failed! Input data size is out of bounds." );
		NumericEventData Data( (int) ERegisterResult::ParamLengths );
		EventHub::Execute( "RegisterResult", Data );
		return;
	}
	
	// Hash Password
	std::vector< uint8 > PasswordData( Password.size() + 5 );
	std::copy( Password.begin(), Password.end(), PasswordData.begin() );
	std::copy( Username.rbegin(), Username.rbegin() + 5, PasswordData.end() - 5 );
	
	for( int i = 0; i < 3; i++ )
	{
		PasswordData = CryptoLibrary::SHA256( PasswordData );
		if( PasswordData.size() != 32 )
		{
			log( "[RegLogin] Registration Failed! Couldnt hash the password properly" );
			NumericEventData Data( (int) ERegisterResult::ParamLengths );
			return;
		}
	}
	
	FRegisterPacket Packet = FRegisterPacket();
	
	// Copy data into the packet
	std::copy( Username.begin(), Username.end(), std::begin( Packet.Username ) );
	std::copy( PasswordData.begin(), PasswordData.end(), std::begin( Packet.Password ) );
	std::copy( DispName.begin(), DispName.end(), std::begin( Packet.DisplayName ) );
	std::copy( EmailAddr.begin(), EmailAddr.end(), std::begin( Packet.EmailAddress ) );
	
	Packet.NetworkCommand = (uint32) ENetworkCommand::Register;
	Packet.NetworkArgument = 0;
	
	if( !Connection.SendPacket< FRegisterPacket >( Packet, [=] ( asio::error_code ErrorCode, unsigned int NumBytes )
	  {
		  if( ErrorCode )
		  {
			  log( "[RegLogin] Register Failed! Couldnt send packet to network.. check connection and retry." );
			  NumericEventData Data( (int) ERegisterResult::ConnectionError );
			  EventHub::Execute( "RegisterResult", Data );
			  
			  // Update State
			  bRegisterInProgress = false;
			  CancelTimeout( "Register" );
		  }
	  } ) )
	{
		log( "[RegLogin] Register Failed! Couldnt send packet to network.. check connection and retry." );
		NumericEventData Data( (int) ERegisterResult::ConnectionError );
		EventHub::Execute( "RegisterResult", Data );
		return;
	}
	
	// Update state
	bRegisterInProgress = true;
	UpdateTimeout( "Register", 12000 );
	
}

bool RegCloud::HandleRegisterResponse( FIncomingPacket& Packet )
{
	// Check if the operation is timed out
	if( IsTimedOut( "Register" ) )
	{
		log( "[RegLogin] Received register response after it timed out" );
		return true;
	}
	// Cancel Timeout
	CancelTimeout( "Register" );
	
	FGenericPacket PacketHeader = FGenericPacket();
	bRegisterInProgress = false;
	
	if( !FStructHelper::Deserialize< FGenericPacket >( PacketHeader, Packet.Buffer.begin(), Packet.Buffer.size() ) )
	{
		log( "[RegLogin] Failed to register new account, response from server was invalid!" );
		NumericEventData Data( (int) ERegisterResult::TryLater );
		EventHub::Execute( "RegisterResult", Data );
		return true;
	}
	
	if( PacketHeader.NetworkArgument != (uint32) ERegisterResult::Success )
	{
		log( "[RegLogin] Failed to register new account, servers responded no!" );
		NumericEventData Data( (int) PacketHeader.NetworkArgument );
		EventHub::Execute( "RegisterResult", Data );
		return true;
	}
	
	if( HandleAccountInfo( Packet.Buffer ) )
	{
		log( "[RegLogin] Successfully created a new account!" );
		
		LoginState = ENodeProcessState::Complete;
		NumericEventData Data( (int) ERegisterResult::Success );
		EventHub::Execute( "RegisterResult", Data );
		NumericEventData loginData( (int) LoginResult::Success );
		EventHub::Execute( "LoginResult", loginData );
	}
	else
	{
		log( "[RegLogin] Created a new account.. but the server response was bad!" );
		
		LoginState = ENodeProcessState::Reset;
		LocalAccount.reset();
		LocalCards.reset();
		LocalDecks.reset();
		LocalAchv.reset();
		
		NumericEventData Data( (int) ERegisterResult::SuccessWithInvalidPacket );
		EventHub::Execute( "RegisterResult", Data );
	}
	
	return true;
}

bool RegCloud::Reconnect()
{
	if( !Connection.IsConnected() )
	{
		UpdateTimeout( "connect", 12000 );
		return Connection.BeginConnect();
	}
	
	return false;
}
