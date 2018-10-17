#pragma once

#include <stdint.h>
#include <algorithm>
#include "cocos/cocos2d.h"

using namespace cocos2d;

typedef uint8_t uint8;
typedef int8_t int8;
typedef uint16_t uint16;
typedef int16_t int16;
typedef uint32_t uint32;
typedef int32_t int32;
typedef uint64_t uint64;
typedef int64_t int64;

#define REG_DEFINE_SIZE( Size ) static size_t GetSize() { return Size; }
#define REG_DEFINE_STATIC_SIZE( Size ) static size_t GetStaticSize() { return Size; }
#define REG_CALC_DYN_SIZE() virtual size_t GetDynamicSize() override
#define REG_SERIALIZE_FUNC() virtual bool Serialize( std::vector< uint8 >::iterator Begin, bool bWrite = false, bool bFlipOrder = false ) override
#define REG_SERIALIZE_DYN() virtual bool Serialize( std::vector< uint8 >::iterator Begin, std::vector< uint8 >::iterator End, bool bWrite = false, bool bFlipOrder = false ) override
#define REG_SERIALIZE_HEADER() FPrivateHeader::Serialize( Begin, bWrite, bFlipOrder )
#define REG_SERIALIZE_HEADER_DYN() FPrivateHeaderDynamic::Serialize( Begin, End, bWrite, bFlipOrder )
#define REG_SERIALIZE_NUM( Var, Size, Offset ) FStructHelper::DoSerialize( (void*) &Var, Size, Begin + Offset, bWrite, bFlipOrder )
#define REG_SERIALIZE_STR( Var, Size, Offset ) FStructHelper::DoSerialize( (void*) &Var, Size, Begin + Offset, bWrite, false )
#define REG_SERIALIZE_NUM_ABS( Var, Size, Iter ) FStructHelper::DoSerialize( (void*) &Var, Size, Iter, bWrite, bFlipOrder )
#define REG_SERIALIZE_STR_ABS( Var, Size, Iter ) FStructHelper::DoSerialize( (void*) &Var, Size, Iter, bWrite, false )

/*=============================================================================================================================
	Network Enumerators
=============================================================================================================================*/
enum class ENetworkEncryption
{
	Full = 0,
	Light = 1,
	None = 2
};

enum class LoginResult
{
    ConnectionError,
    Timeout,
    AlreadyLoggedIn,
    InvalidInput,
    InvalidCredentials,
    Success
};

enum class ELoginResponse
{
	Error = 0,
	InvalidCredentials = 1,
	Success = 2
};

enum class ENetworkCommand
{
	Ping = 1,
	Login = 2,
	InvalidPacket = 3,
	KeyExchange = 4,
	Logout = 5,
	Register = 6
};

enum class EInvalidPacketArgument
{
	Error = 0,
	InvalidEncryption = 1,
	RenewLogin = 2,
	InvalidCommand = 3
};

enum class EKeyExchangeArgument
{
	InitialRequest = 0,
	InvalidKey = 1,
	Error = 2,
	Success = 3
};

enum class ELoginResponseArgument
{
	InvalidCredentials = 0,
	Error = 1,
	Success = 2,
	ExistingLogin = 3
};
enum class ENodeConnectionState
{
	OfflineMode,
	ConnectionNotStarted,
	Connected,
	LoggedIn
};

enum class ENodeProcessState
{
	NotStarted,
	InProgress,
	Complete,
	Reset,
};

enum class ELoginResult
{
	Error = 0,
	InvalidCredentials = 1,
	Success = 2,
	ConnectionError = 3,
	InputError = 4,
	TimedOut = 5
};

enum class ERegisterResult
{
	ParamLengths = 0,
	InvalidEmail = 1,
	UsernameTaken = 2,
	EmailTaken = 3,
	TryLater = 4,
	Success = 5,
	SuccessWithInvalidPacket = 6,
	IllegalUsername = 7,
    ConnectionError = 8
};

enum class EConnectResult
{
	ConnectionFailure,
	KeyExchangeError,
	KeyExchangeTimeout,
	InvalidKey,
	OtherError,
	Success
};

enum class EDisconnectReason
{
	PingTimeout,
	ClosedByHost,
	ClosedByClient,
	OtherError
};

enum class CloudEvent
{
	None,
	ConnectBegin,
	ConnectResult,
	_Internal_Connect,
	Disconnect,
	LoginBegin,
	LoginComplete,
	RegisterBegin,
	RegisterComplete
};

enum class ConnectResult
{
	ConnectionError,
	KeyExchangeError,
	Success,
	OtherError
};

enum class EEndianOrder
{
	BigEndian = 0,
	LittleEndian = 1
};


/*=============================================================================================================================
	Network Structs
=============================================================================================================================*/

struct FRegStruct
{
public:
	virtual ENetworkEncryption EncryptionLevel() const
	{
		return ENetworkEncryption::Full;
	}
};

struct FSerializableStruct : FRegStruct
{
public:

	virtual bool Serialize( std::vector< uint8 >::iterator Begin, bool bWrite = false, bool bFlipOrder = false ) = 0;
};

struct FSerializableDynamicStruct : FRegStruct
{
public:
	
	virtual bool Serialize( std::vector< uint8 >::iterator Begin, std::vector< uint8 >::iterator End, bool bWrite = false, bool bFlipOrder = false ) = 0;
};

class FStructHelper
{

public:
	
	static bool DoSerialize( void* Var, size_t VarSize, std::vector< uint8 >::iterator BufferPos, bool bWrite, bool bFlipOrder )
	{
		if( Var == nullptr || VarSize <= 0 )
		{
			return false;
		}

		uint8* VarStart = reinterpret_cast< uint8* >( Var );

		if( bFlipOrder )
		{
			if( bWrite )
			{
				std::reverse_copy( VarStart, VarStart + VarSize, BufferPos );
			}
			else
			{
				std::reverse_copy( BufferPos, BufferPos + VarSize, VarStart );
			}
		}
		else
		{
			if( bWrite )
			{
				std::copy( VarStart, VarStart + VarSize, BufferPos );
			}
			else
			{
				std::copy( BufferPos, BufferPos + VarSize, VarStart );
			}
		}

		return true;
	}

	template< typename T >
	static bool Deserialize( FSerializableStruct& Structure, std::vector< uint8 >::iterator DataStart, const size_t DataSize )
	{
		// Quick check if there is enough data for this operation
		if( T::GetSize() > DataSize )
		{
			return false;
		}

		// Check if we need to flip bytes from the server
		//RegCloud* Cloud = RegCloud::Get();
		//bool bShouldFlip = Cloud ? Cloud->DoesServerEndianMatch() : false;

		// Deserialize the structure, taking into account mismatching endianess
		return Structure.Serialize( DataStart, false, false );
	}

	template< typename T >
	static bool Serialize( FSerializableStruct& Structure, std::vector< uint8 >::iterator DataStart, const size_t DataSize )
	{
		// Quick bounds check
		if( T::GetSize() > DataSize )
		{
			return false;
		}

		// Ensure were serializing using little endian
		//RegCloud* Cloud = RegCloud::Get();
		//bool bShouldFlip = Cloud ? Cloud->DoesServerEndianMatch() : false;

		// Were not going to flip byte order while serializing, the server will flip bytes if needed
		return Structure.Serialize( DataStart, true, false );
	}

	template< typename T >
	static bool Serialize( FSerializableDynamicStruct& Structure, std::vector< uint8 >::iterator DataStart, const size_t DataSize )
	{
		// Check to ensure theres enough data in the vector for the entire dynamic structure
		if( T::GetDynamicSize() > DataSize )
		{
			return false;
		}

		//RegCloud* Cloud = RegCloud::Get();
		//bool bShouldFlip = Cloud ? Cloud->DoesServerEndianMatch() : false;

		return Structure.Serialize( DataStart, DataStart + DataSize, true, false );
	}

	template< typename T >
	static bool Deserialize( FSerializableDynamicStruct& Structure, std::vector< uint8 >::iterator DataStart, const size_t DataSize )
	{
		// Since the structure size can vary, we dont know how large the structure is until we actually deserialize
		// But, we do know there has to be enough data for the static members, so we can check that now, if data is 
		// missing, it will be caught during deserialization
		if( T::GetStaticSize() > DataSize )
		{
			return false;
		}

		//RegCloud* Cloud = RegCloud::Get();
		//bool bShouldFlip = Cloud ? Cloud->DoesServerEndianMatch() : false;

		return Structure.Serialize( DataStart, DataStart + DataSize, false, false );
	}
};


/*--------------------------------------------------------------------------------------
	FPublicHeader
	* The public header structure for packets sent to/from the server over TCP/IP
--------------------------------------------------------------------------------------*/
struct FPublicHeader : public FSerializableStruct
{
	int64 PacketSize = 0;	// 0
	int32 EncryptionMethod = 0;	// 8
	uint32 EndianOrder = 1;

	static size_t GetSize()
	{
		return 16;
	}

	virtual bool Serialize( std::vector< uint8 >::iterator Begin, bool bWrite = false, bool bFlipOrder = false ) override
	{
		return( FStructHelper::DoSerialize( (void*)&PacketSize, 8, Begin, bWrite, bFlipOrder ) &&
				FStructHelper::DoSerialize( (void*)&EncryptionMethod, 4, Begin + 8, bWrite, bFlipOrder ) &&
				FStructHelper::DoSerialize( (void*)&EndianOrder, 4, Begin + 12, bWrite, false ) );
	}

	virtual ENetworkEncryption EncryptionLevel() const override
	{
		return ENetworkEncryption::None;
	}

	// Were putting this function here, so if the public header is ever updated, then we can correct the alignment
	static void GetEndianBytes( std::vector< uint8 >::iterator DataBegin, std::vector< uint8 >::iterator VarBegin )
	{
		std::copy( DataBegin + 12, DataBegin + 16, VarBegin );
	}
	
};									// 16

/*--------------------------------------------------------------------------------------
	FPrivateHeader
	* The private header contained within the encrypted message body of incoming
	  TCP/IP packets containing protected information about the packet
--------------------------------------------------------------------------------------*/
struct FPrivateHeader : FSerializableStruct
{
	uint32 NetworkCommand = 0;	// 0
	uint32 NetworkArgument = 0;	// 4

	static size_t GetSize()
	{
		return 8;
	}

	virtual bool Serialize( std::vector< uint8 >::iterator Begin, bool bWrite = false, bool bFlipOrder = false ) override
	{
		return( FStructHelper::DoSerialize( (void*)&NetworkCommand, 4, Begin, bWrite, bFlipOrder ) &&
				FStructHelper::DoSerialize( (void*)&NetworkArgument, 4, Begin + 4, bWrite, bFlipOrder ) );
	}
};	// 8

struct FPrivateHeaderDynamic : FSerializableDynamicStruct
{
	uint32 NetworkCommand = 0;
	uint32 NetworkArgument = 0;

	REG_DEFINE_STATIC_SIZE( 8 );
	
	// Base class member, in derived classes, use REG_CALC_DYN_SIZE()
	virtual size_t GetDynamicSize()
	{
		return 8;
	}

	REG_SERIALIZE_DYN()
	{
		return( REG_SERIALIZE_NUM( NetworkCommand, 4, 0 ) &&
				REG_SERIALIZE_NUM( NetworkArgument, 4, 4 ) );
	}
};

/*--------------------------------------------------------------------------------------
	FGenericPacket
	* A generic packet structure used to simply send a command code and argument
--------------------------------------------------------------------------------------*/
struct FGenericPacket : FPrivateHeader
{
};

/*--------------------------------------------------------------------------------------
	FSmallPayloadPacket
	* A generic packet that also contains 128 bytes of binary data that can
	  be used to store a additional data, along with the command and argument
--------------------------------------------------------------------------------------*/
struct FSmallPayloadPacket : FPrivateHeader
{
	uint8 Payload[ 128 ];			// 8

	static size_t GetSize()
	{
		return 136;
	}

	virtual bool Serialize( std::vector< uint8 >::iterator Begin, bool bWrite = false, bool bFlipOrder = false ) override
	{
		return( FPrivateHeader::Serialize( Begin, bWrite, bFlipOrder ) &&
				FStructHelper::DoSerialize( (void*)&Payload, 128, Begin + 8, bWrite, false ) );
	}
};									// 13

/*--------------------------------------------------------------------------------------
	FPingPacket
	* A packet used for sending pings between the client and server
--------------------------------------------------------------------------------------*/
struct FPingPacket : FPrivateHeader
{
	int64 TimeStamp = 0;	// 4

	static size_t GetSize()
	{
		return 16;
	}

	virtual bool Serialize( std::vector< uint8 >::iterator Begin, bool bWrite = false, bool bFlipOrder = false ) override
	{
		return( FPrivateHeader::Serialize( Begin, bWrite, bFlipOrder ) &&
				FStructHelper::DoSerialize( (void*)&TimeStamp, 8, Begin + 8, bWrite, bFlipOrder ) );
	}

	virtual ENetworkEncryption EncryptionLevel() const override
	{
		return ENetworkEncryption::None;
	}
};									// 16

/*--------------------------------------------------------------------------------------
	FKeyExchangePacket
	* A packet that gets sent to and from the server during the key exchange process
--------------------------------------------------------------------------------------*/
struct FKeyExchangePacket : FPrivateHeader
{
	uint8 Key[ 512 ];				// 8

	static size_t GetSize()
	{
		return 520;
	}

	virtual bool Serialize( std::vector< uint8 >::iterator Begin, bool bWrite = false, bool bFlipOrder = false ) override
	{
		return( FPrivateHeader::Serialize( Begin, bWrite, bFlipOrder ) &&
				FStructHelper::DoSerialize( (void*) &Key, 512, Begin + 8, bWrite, false ) );
	}

	virtual ENetworkEncryption EncryptionLevel() const override
	{
		return ENetworkEncryption::Light;
	}
};									// 520

/*--------------------------------------------------------------------------------------
	FLoginPacket
	* A packet that gets sent to the server to log in to an account
--------------------------------------------------------------------------------------*/
struct FLoginPacket : FPrivateHeader
{
	uint8 Username[ 128 ];
	uint8 Password[ 32 ];
    
	static size_t GetSize()
	{
		return 168;
	}

	virtual bool Serialize( std::vector< uint8 >::iterator Begin, bool bWrite = false, bool bFlipOrder = false ) override
	{
		return( FPrivateHeader::Serialize( Begin, bWrite, bFlipOrder ) &&
				FStructHelper::DoSerialize( (void*) &Username, 128, Begin + 8, bWrite, false ) &&
				FStructHelper::DoSerialize( (void*) &Password, 32, Begin + 136, bWrite, false ) );
	}
};

struct FRegisterPacket : FPrivateHeader
{
	uint8 DisplayName[ 256 ];
	uint8 EmailAddress[ 1024 ];
	uint8 Username[ 128 ];
	uint8 Password[ 32 ];

	static size_t GetSize()
	{
		return 1448;
	}

	virtual bool Serialize( std::vector< uint8 >::iterator Begin, bool bWrite = false, bool bFlipOrder = false ) override
	{
		return( FPrivateHeader::Serialize( Begin, bWrite, bFlipOrder ) &&
				FStructHelper::DoSerialize( (void*) &DisplayName, 256, Begin + 8, bWrite, false ) &&
				FStructHelper::DoSerialize( (void*) &EmailAddress, 1024, Begin + 264, bWrite, false ) &&
				FStructHelper::DoSerialize( (void*) &Username, 128, Begin + 1288, bWrite, false ) &&
				FStructHelper::DoSerialize( (void*) &Password, 32, Begin + 1416, bWrite, false ) );
	}
};

struct FPlayerCard
{
	uint16 Identifier;
	uint16 Count;
};

struct FPlayerAchievement
{
	uint32 Identifier;
	float Progress;
	int32 State;
	uint32 _reserved_;
};

struct FPlayerDeck
{
	uint32 Identifier;
	uint32 CardCount;
	std::string DisplayName;

	// Static Size = 4 + 4 + 256 = 264
	std::vector< FPlayerCard > Cards;
};

// ONLY DESERIALIZATION!!!!!
struct FAccountInfo : FPrivateHeaderDynamic
{
	uint32 PlayerId = 0;

	std::string DisplayName;
	std::string EmailAddress;

	uint32 _reserved1_ = 0;

	uint64 Coins = 0;

	// The rest of the data is all variable in size, which means we will have to create a custom function to 
	// deseraialize these packets, since the existing deserialize method is for fixed size packets only.
	// To know where the different data sections end, we need some offset values.
	uint32 CardDataSize = 0;
	uint32 DeckDataSize = 0;
	uint32 AchvDataSize = 0;

	int _reserved2_ = 0;

	/*-------------------------------------------------
		Card Data -> Deck Data -> Acheivment Data
	-------------------------------------------------*/
	std::vector< FPlayerCard > Cards;
	std::vector< FPlayerDeck > Decks;
	std::vector< FPlayerAchievement > Achv;
	
	// Serialization Logic

	// Since this is a dynamic packet, we need to declare the size of the static portion
	REG_DEFINE_STATIC_SIZE( 1320 );

	REG_CALC_DYN_SIZE()
	{
		return GetStaticSize() +
			CardDataSize +
			DeckDataSize +
			AchvDataSize;
	}

	REG_SERIALIZE_DYN()
	{
		if( bWrite )
			throw std::exception();

		char c_DisplayName[ 256 ];
		char c_EmailAddress[ 1024 ];

		if( !REG_SERIALIZE_HEADER_DYN() ||
			!REG_SERIALIZE_NUM( PlayerId, 4, 8 ) ||
			!REG_SERIALIZE_STR( c_DisplayName, 256, 12 ) ||
			!REG_SERIALIZE_STR( c_EmailAddress, 1024, 268 ) ||
			!REG_SERIALIZE_NUM( _reserved1_, 4, 1292 ) ||
			!REG_SERIALIZE_NUM( Coins, 8, 1296 ) ||
			!REG_SERIALIZE_NUM( CardDataSize, 4, 1304 ) ||
			!REG_SERIALIZE_NUM( DeckDataSize, 4, 1308 ) ||
			!REG_SERIALIZE_NUM( AchvDataSize, 4, 1312 ) ||
			!REG_SERIALIZE_NUM( _reserved2_, 4, 1316 ) )
		{
			return false;
		}

		// Create strings
		std::string s_DisplayName;
		std::string s_EmailAddress;

		for( char c : c_DisplayName )
		{
			s_DisplayName.append( 1, c );
			if( c == (char) 0 )
				break;
		}

		for( char c : c_EmailAddress )
		{
			s_EmailAddress.append( 1, c );
			if( c == (char) 0 )
				break;
		}

		DisplayName = s_DisplayName;
		EmailAddress = s_EmailAddress;

		// Ensure we have enough data
		if( Begin + CardDataSize + DeckDataSize + AchvDataSize > End )
		{
			cocos2d::log( "[Reg Serialization] Not enough data to (de)serialize a dynamic packet!" );
			return false;
		}

		// Now for the hard part, we need to handle variable sized fields
		// First, lets get an iterator to the beginning of the variable data blob
		std::vector< uint8 >::iterator CardBegin = Begin + GetStaticSize();
		std::vector< uint8 >::iterator DeckBegin = CardBegin + CardDataSize;
		std::vector< uint8 >::iterator AchvBegin = DeckBegin + DeckDataSize;
		std::vector< uint8 >::iterator DataEnd = Begin + CardDataSize + DeckDataSize + AchvDataSize;

		Cards.clear();
		Decks.clear();
		Achv.clear();

		const size_t CardSize = sizeof( FPlayerCard );
		for( auto Iter = CardBegin; Iter + CardSize < DeckBegin; Iter += CardSize )
		{
			FPlayerCard NewCard;
			if( !REG_SERIALIZE_NUM_ABS( NewCard.Identifier, 2, Iter ) ||
				!REG_SERIALIZE_NUM_ABS( NewCard.Count, 2, Iter + 2 ) )
			{
                cocos2d::log( "[Reg Serialization] Failed to deserialize a card within the account info!" );
				break;
			}

			Cards.push_back( NewCard );
		}

		// Decks are a little more tricky, since each one is of variable size itself
		// The static size of a deck is 264, so we should start out by 
		const size_t DeckStaticSize = 264;
		for( auto Iter = DeckBegin; Iter + DeckStaticSize < AchvBegin; )
		{
			FPlayerDeck NewDeck;

			char DisplayName[ 256 ];

			if( !REG_SERIALIZE_NUM_ABS( NewDeck.Identifier, 4, Iter ) ||
				!REG_SERIALIZE_NUM_ABS( NewDeck.CardCount, 4, Iter + 4 ) ||
				!REG_SERIALIZE_STR_ABS( DisplayName, 256, Iter + 8 ) )
			{
                cocos2d::log( "[Reg Serialization] Failed to deserialize static portion of a deck!" );
				break;
			}

			// Create String
			std::string nameStr;
			for( char c : DisplayName )
			{
				nameStr.append( 1, c );
				if( c == (char) 0 )
					break;
			}

			NewDeck.DisplayName = nameStr;

			// Check if we have enough data for the entire deck
			const size_t TotalCardsSize = NewDeck.CardCount * CardSize;
			if( Iter + DeckStaticSize + TotalCardsSize >= AchvBegin )
			{
                cocos2d::log( "[Reg Serialization] Not enough data to deserialize deck '%s'", NewDeck.DisplayName.c_str() );
				break;
			}

			// Advance Iter
			Iter += DeckStaticSize;
			bool bCardError = false;

			// Deserialize each card
			for( unsigned int Index = 0; Index < NewDeck.CardCount; Index++ )
			{
				FPlayerCard NewCard;
				if( !REG_SERIALIZE_NUM_ABS( NewCard.Identifier, 2, Iter ) ||
					!REG_SERIALIZE_NUM_ABS( NewCard.Count, 2, Iter + 2 ) )
				{
                    cocos2d::log( "[Reg Serialization] Failed to deserialize a card within the deck '%s'", NewDeck.DisplayName.c_str() );

					bCardError = true;
					break;
				}

				// Advance Iterator, and insert card
				Iter += CardSize;
				NewDeck.Cards.push_back( NewCard );
			}

			if( bCardError )
			{
				break;
			}

			Decks.push_back( NewDeck );
		}

		// Time for achievements, these are quite easy
		const size_t AchvSize = sizeof( FPlayerAchievement );
		for( auto Iter = AchvBegin; Iter + AchvSize < DataEnd; Iter += AchvSize )
		{
			FPlayerAchievement NewAchv;

			if( !REG_SERIALIZE_NUM_ABS( NewAchv.Identifier, 4, Iter ) ||
				!REG_SERIALIZE_NUM_ABS( NewAchv.Progress, 4, Iter + 4 ) ||
				!REG_SERIALIZE_NUM_ABS( NewAchv.State, 4, Iter + 8 ) ||
				!REG_SERIALIZE_NUM_ABS( NewAchv._reserved_, 4, Iter + 12 ) )
			{
                cocos2d::log( "[Reg Serialization] Failed to deserialize an achievement!" );
				break;
			}

			Achv.push_back( NewAchv );
		}
        
        return true;
	}
};

// ONLY DESERIALIZATION!
struct FLoginResponse : FAccountInfo
{
};



/*--------------------------------------------------------------------------------------
	FIncommingPacket
	* Used to store and pass packets incoming from the cloud
--------------------------------------------------------------------------------------*/
struct FIncomingPacket
{
	FPublicHeader PacketHeader;
	std::vector< uint8 > Buffer;
	ENetworkCommand CommandCode;
};


namespace Regicide
{
    constexpr auto REG_USERNAME_MINLEN =    5;
    constexpr auto REG_USERNAME_MAXLEN =    32;
    constexpr auto REG_PASSWORD_MINLEN =    5;
    constexpr auto REG_EMAIL_MINLEN    =         5;
    constexpr auto REG_EMAIL_MAXLEN    =         255;
    constexpr auto REG_DISPNAME_MINLEN =    5;
    constexpr auto REG_DISPNAME_MAXLEN =    48;
};
