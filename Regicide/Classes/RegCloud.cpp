#include <memory>
#include <functional>
#include <chrono>
#include "cocos2d/external/json/rapidjson.h"
#include "RegCloud.h"
#include "cocos2d.h"
#include "rsa.h"
#include "cipher.h"
#include "entropy.h"
#include "ctr_drbg.h"
#include "CryptoLibrary.h"


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
	BindEvent( CloudEvent::_Internal_Connect, std::bind( &RegCloud::OnSocketConnection, this, _1, _2 ) );
	Connection.RegisterCallback( ENetworkCommand::KeyExchange, "regcloud_exchange", std::bind( &RegCloud::OnSessionResponse, this, _1 ) );
	Connection.RegisterCallback( ENetworkCommand::Login, "regcloud_login", std::bind( &RegCloud::HandleLoginResponse, this, _1 ) );

}

RegCloud::~RegCloud()
{
	// Free the RSA Context
	mbedtls_rsa_free( &CryptoContext );
	mbedtls_ctr_drbg_free( &RandomContext );
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
		Connection.BeginConnect();
	}
}


void RegCloud::BindEvent( CloudEvent inEvent, std::function< void( CloudEvent, unsigned int ) > Callback )
{
	std::vector< std::function< void( CloudEvent, unsigned int )> >& EventCallbacks = CallbackList[ inEvent ];
	EventCallbacks.insert( EventCallbacks.end(), Callback );
}

void RegCloud::ExecuteEvent( CloudEvent inEvent, int Parameter )
{
	// Get director ref
	auto dir = Director::getInstance();
	auto sch = dir ? dir->getScheduler() : nullptr;

	if( inEvent != CloudEvent::_Internal_Connect && !sch )
	{
		// We need the scheduler to execute this on the game thread!
		cocos2d::log( "[ERROR] Failed to get scheduler to execute cloud event on game thread!" );
		return;
	}

	if( CallbackList.find( inEvent ) != CallbackList.end() )
	{
		for( auto& CallbackFunc : CallbackList[ inEvent ] )
		{
			// Internal callbacks will be executed on the current asio thread
			// all other callbacks will be executed on a game thread
			if( inEvent == CloudEvent::_Internal_Connect )
			{
				CallbackFunc( inEvent, Parameter );
			}
			else
			{
				sch->performFunctionInCocosThread( [ = ]()
				{
					CallbackFunc( inEvent, Parameter );
				} );
			}
		}
	}
}


void RegCloud::OnSocketConnection( CloudEvent inEvent, int Paramter )
{
	// This event means the underlying socket connection was successful
	EstablishSession();
}


void RegCloud::EstablishSession()
{
	// Send session request to the server
	if( !Connection.IsConnected() )
	{
		log( "[ERROR] Failed to establish session with RegSys because we are disconnected!" );
		ExecuteEvent( CloudEvent::ConnectResult, (unsigned int) ConnectResult::ConnectionError );
		return;
	}
	else if( SessionState == ENodeProcessState::Complete )
	{
		log( "[Warning] (RegSys) Attempt to establish a session while the current session is still valid!" );
		return;
	}

	// Debug
	log( "[DEBUG] Starting key exchange...." );

	SessionState = ENodeProcessState::InProgress;

	FKeyExchangePacket Packet;
	Packet.NetworkCommand	= (uint32) ENetworkCommand::KeyExchange;
	Packet.NetworkArgument	= (uint32) EKeyExchangeArgument::InitialRequest;

	// Generate RSA key
	if( !GenerateExchangeKeys() )
	{
		SessionState = ENodeProcessState::Reset;
		ExecuteEvent( CloudEvent::ConnectResult, (int) ConnectResult::KeyExchangeError );
		return;
	}

	// Export RSA key from mbedtls
	std::vector< uint8 > Modulus( 256 );
	std::vector< uint8 > Exponent( 4 );

	if( mbedtls_rsa_export_raw( &CryptoContext, Modulus.data(), Modulus.size(), NULL, 0, NULL, 0, NULL, 0, Exponent.data(), Exponent.size() ) != 0 )
	{
		log( "[ERROR] RegSys failed to establish session because the exchange key could not be exported!" );

		SessionState = ENodeProcessState::Reset;
		ExecuteEvent( CloudEvent::ConnectResult, (int) ConnectResult::KeyExchangeError );
		return;
	}

	// Move key data into the packet and clear original key buffers
	try
	{
		std::move( Modulus.begin(), Modulus.begin() + 256, Packet.Key );
		std::move( Exponent.begin(), Exponent.begin() + 4, Packet.Key + 256 );
	}
	catch( std::exception& Ex )
	{
		log( "[ERROR] RegSys failed to establish session because an exception was thrown while moving key data into the request packet! %s", Ex.what() );

		SessionState = ENodeProcessState::Reset;
		ExecuteEvent( CloudEvent::ConnectResult, (int) ConnectResult::KeyExchangeError );
		return;
	}

	Modulus.clear();
	Exponent.clear();

	// Finally, send the packet to RegSys to create the session
	if( !Connection.SendPacket< FKeyExchangePacket >( Packet, [ this ]( asio::error_code SendError, unsigned int NumBytes )
	{
		if( SendError )
		{
			log( "[ERROR] Failed to send session request to RegSys! %s", SendError.message() );

			SessionState = ENodeProcessState::Reset;
			ExecuteEvent( CloudEvent::ConnectResult, (int) ConnectResult::KeyExchangeError );
		}
		else
		{
			log( "[RegSys] Sent session request to server!" );
		}
	} ) )
	{
		log( "[ERROR] Failed to send session request to RegSys!" );

		SessionState = ENodeProcessState::Reset;
		ExecuteEvent( CloudEvent::ConnectResult, (int) ConnectResult::KeyExchangeError );
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
	// Deserialize Packet
	FKeyExchangePacket Response;

	if( !FStructHelper::Deserialize< FKeyExchangePacket >( Response, Packet.Buffer.begin(), Packet.Buffer.size() ) )
	{
		log( "[ERROR] Failed to deseraialize session request!" );
		SessionState = ENodeProcessState::Reset;
		ExecuteEvent( CloudEvent::ConnectResult, (int) ConnectResult::KeyExchangeError );

		return true;
	}

	auto Result = Response.NetworkArgument;
	if( Result != (uint32) EKeyExchangeArgument::Success )
	{
		log( "[ERROR] Failed to establish session with RegSys! Server couldnt process our request. Error Code: %d", (int) Result );
		SessionState = ENodeProcessState::Reset;
		ExecuteEvent( CloudEvent::ConnectResult, (int) ConnectResult::KeyExchangeError );

		return true;
	}

	// Decrypt Response
	if( mbedtls_rsa_check_privkey( &CryptoContext ) != 0 )
	{
		log( "[ERROR] Failed to establish session with RegSys! Received valid response but the key needed to decrypt is missing!" );
		SessionState = ENodeProcessState::Reset;
		ExecuteEvent( CloudEvent::ConnectResult, (int) ConnectResult::KeyExchangeError );

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
		ExecuteEvent( CloudEvent::ConnectResult, (int) ConnectResult::KeyExchangeError );

		return true;
	}

	if( OutputSize < 32 )
	{
		log( "[ERROR] Failed to establish session with RegSys! Decrypted request response was less than the minimum size!" );
		SessionState = ENodeProcessState::Reset;
		ExecuteEvent( CloudEvent::ConnectResult, (int) ConnectResult::KeyExchangeError );

		return true;
	}

	// Resize decrypted data buffer, it should be much much smaller than 512
	DecryptedData.resize( OutputSize );

	// Read Session Key, connection will free memory on destructor
	uint8* SessionKey = new uint8[ 32 ];
	std::move( Response.Key, Response.Key + 32, SessionKey );

	Connection.SetSessionKey( SessionKey );

	log( "[DEBUG] Established secure session with RegSys!" );
	SessionState = ENodeProcessState::Complete;
	ExecuteEvent( CloudEvent::ConnectResult, (int) ConnectResult::Success );

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
}

void RegCloud::Login( std::string Username, std::string Password, std::function<void( RegCloud::LoginResult )> Callback )
{
	// Update Callback
	if( Callback )
	{
		LoginCallback = Callback;
	}

	// Check if this operation is already in progress, or complete
	if( LoginState == ENodeProcessState::InProgress )
	{
		log( "[RegSys] Attempt to login, but login was already in progress! Callback updated" );
		return;
	}
	else if( LoginState == ENodeProcessState::Complete )
	{
		log( "[RegSys] Attempt to login, but user is already logged in! Must log out before attempting another login!" );
		LoginCallback( RegCloud::LoginResult::AlreadyLoggedIn );
		return;
	}
	else if( !IsSecure() )
	{
		log( "[RegSys] Attempt to login, but there is no active session! Reconnect and retry!" );
		LoginCallback( RegCloud::LoginResult::ConnectionError );
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
			LoginCallback( RegCloud::LoginResult::InvalidInput );
			return;
		}
	}

	// Ensure data is the correct size
	if( PasswordData.size() != 32 || Username.size() > 128 )
	{
		log( "[RegSys Login] Check inputs. Username must be between 5 and 32 characters." );
		LoginCallback( RegCloud::LoginResult::InvalidInput );
		return;
	}

	// Copy everything into the packet
	FLoginPacket Packet;

	std::copy( Username.begin(), Username.end(), std::addressof( Packet.Username ) );
	std::copy( PasswordData.begin(), PasswordData.end(), std::addressof( Packet.Password ) );

	Packet.NetworkCommand = (uint32) ENetworkCommand::Login;
	Packet.NetworkArgument = 0;

	// Attempt to send packet to service
	if( !Connection.SendPacket< FLoginPacket >( Packet, [ = ]( asio::error_code SendErr, unsigned int NumBytes )
	{
		if( SendErr )
		{
			log( "[RegSys Login] Failed to send login packet to service! Error: %s", SendErr.message() );
			LoginCallback( RegCloud::LoginResult::ConnectionError );
		}
	} ) )
	{
		log( "[RegSys Login] Failed to send login packet to service! Unknown error.." );
		LoginCallback( RegCloud::LoginResult::ConnectionError );
		return;
	}

	// Update state
	LoginState = ENodeProcessState::InProgress;

	// Begin Timeout timer
	TimeoutThread = std::make_shared<std::thread>( [ this ]()
	{
		std::unique_lock< std::mutex >( TimeoutMutex );
		if( !TimeoutVar.wait_for( TimeoutMutex, std::chrono::milliseconds( 12000 ), [ this ]()
		{
			return LoginState != ENodeProcessState::InProgress;
		} ) )
		{
			// Operation timed out
			LoginState = ENodeProcessState::Reset;
			log( "[RegSys Login] Error! Login operation timed-out!" );
			LoginCallback( RegCloud::LoginResult::Timeout );
		}
	} );
	
}


bool RegCloud::HandleLoginResponse( FIncomingPacket& Packet )
{
	FGenericPacket ResponsePacket;

	if( !FStructHelper::Deserialize< FGenericPacket >( ResponsePacket, Packet.Buffer.begin(), Packet.Buffer.size() ) )
	{
		log( "[RegSys Login] Error! Received invalid login response from the server!" );
		if( LoginCallback )
		{
			LoginCallback( RegCloud::LoginResult::ConnectionError );
		}
		else
		{
			log( "[RegSys Login] ERROR! No login callback is bound!" );
		}

		// TODO: Clear local account info
		LoginState = ENodeProcessState::Reset;
		return true;
	}

	if( ResponsePacket.NetworkArgument == (uint32) ELoginResponse::Error )
	{
		log( "[RegSys Login] Warning! An error occurred on the server while processing login request!" );
		if( LoginCallback )
			LoginCallback( RegCloud::LoginResult::ConnectionError );
		
		// TODO: Clear local account info
		LoginState = ENodeProcessState::Reset;
		return true;
	}
	else if( ResponsePacket.NetworkArgument == (uint32) ELoginResponse::InvalidCredentials )
	{
		log( "[RegSys Login] Warning! Login attempt failed because the credentials were not correct!" );
		if( LoginCallback )
			LoginCallback( RegCloud::LoginResult::InvalidCredentials );

		// TODO: Clear local account info
		LoginState = ENodeProcessState::Reset;
		return true;
	}
	else
	{
		// Success! We need to deseraialize the full packet now, and handle the parsing of this data
		if( HandleAccountInfo( Packet.Buffer ) )
		{
			log( "[RegSys] Login was successful!" );
			LoginState = ENodeProcessState::Complete;
		}
		else
		{
			log( "[RegSys Login] Failed to process account info!" );

			// TODO: Clear local account info
			LoginState = ENodeProcessState::Reset;
		}

		return true;
	}
}


bool RegCloud::HandleAccountInfo( std::vector< uint8 >& RawAccountInfo )
{
	
}