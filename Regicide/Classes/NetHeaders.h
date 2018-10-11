#pragma once
#include <stdint.h>
#include <algorithm>

typedef uint8_t uint8;
typedef int8_t int8;
typedef uint16_t uint16;
typedef int16_t int16;
typedef uint32_t uint32;
typedef int32_t int32;
typedef uint64_t uint64;
typedef int64_t int64;


/*=============================================================================================================================
	Network Enumerators
=============================================================================================================================*/
enum class ENetworkEncryption
{
	Full = 0,
	Light = 1,
	None = 2
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
	IllegalUsername = 7
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

struct FSerializableStruct
{
public:

	virtual bool Serialize( std::vector< uint8 >::iterator Begin, bool bWrite = false, bool bFlipOrder = false ) = 0;
	virtual ENetworkEncryption EncryptionLevel() const
	{
		return ENetworkEncryption::Full;
	}
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
		RegCloud* Cloud = RegCloud::Get();
		bool bShouldFlip = Cloud ? Cloud->DoesServerEndianMatch() : false;

		// Deserialize the structure, taking into account mismatching endianess
		return Structure.Serialize( DataStart, false, bShouldFlip );
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
		RegCloud* Cloud = RegCloud::Get();
		bool bShouldFlip = Cloud ? Cloud->DoesServerEndianMatch() : false;

		// Were not going to flip byte order while serializing, the server will flip bytes if needed
		return Structure.Serialize( DataStart, true, bShouldFlip );
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
};									// 8

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


