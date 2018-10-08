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


/*=============================================================================================================================
	Network Structs
=============================================================================================================================*/

class FStructHelper
{

public:
	
	static bool Serialize( void* Var, size_t VarSize, std::vector< uint8 >::iterator BufferPos, bool bWrite, bool bFlipOrder )
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
};

struct FSerializableStruct
{
public:

	virtual bool Serialize( std::vector< uint8 >::iterator Begin, bool bWrite = false, bool bFlipOrder = false ) = 0;
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
		return( FStructHelper::Serialize( (void*)&PacketSize, 8, Begin, bWrite, bFlipOrder ) &&
				FStructHelper::Serialize( (void*)&EncryptionMethod, 4, Begin + 8, bWrite, bFlipOrder ) &&
				FStructHelper::Serialize( (void*)&EndianOrder, 4, Begin + 12, bWrite, bFlipOrder ) );
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
		return( FStructHelper::Serialize( (void*)&NetworkCommand, 4, Begin, bWrite, bFlipOrder ) &&
				FStructHelper::Serialize( (void*)&NetworkArgument, 4, Begin + 4, bWrite, bFlipOrder ) );
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
				FStructHelper::Serialize( (void*)&Payload, 128, Begin + 8, bWrite, false ) );
	}

};									// 13

/*--------------------------------------------------------------------------------------
	FPingPacket
	* A packet used for sending pings between the client and server
--------------------------------------------------------------------------------------*/
struct FPingPacket : FPrivateHeader
{
	uint64 TimeStamp = 0;	// 4

	static size_t GetSize()
	{
		return 16;
	}

	virtual bool Serialize( std::vector< uint8 >::iterator Begin, bool bWrite = false, bool bFlipOrder = false ) override
	{
		return( FPrivateHeader::Serialize( Begin, bWrite, bFlipOrder ) &&
				FStructHelper::Serialize( (void*)&TimeStamp, 8, Begin + 8, bWrite, bFlipOrder ) );
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
				FStructHelper::Serialize( (void*) &Key, 512, Begin + 8, bWrite, false ) );
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
				FStructHelper::Serialize( (void*) &Username, 128, Begin + 8, bWrite, false ) &&
				FStructHelper::Serialize( (void*) &Password, 32, Begin + 136, bWrite, false ) );
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
				FStructHelper::Serialize( (void*) &DisplayName, 256, Begin + 8, bWrite, false ) &&
				FStructHelper::Serialize( (void*) &EmailAddress, 1024, Begin + 264, bWrite, false ) &&
				FStructHelper::Serialize( (void*) &Username, 128, Begin + 1288, bWrite, false ) &&
				FStructHelper::Serialize( (void*) &Password, 32, Begin + 1416, bWrite, false ) );
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

struct FOutgoingPacket
{
	std::vector< uint8 > Buffer;
	ENetworkEncryption EncryptionMethod = ENetworkEncryption::Full;
};
