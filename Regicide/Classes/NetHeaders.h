#pragma once
#include <stdint.h>

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
	Ping = 0,
	Login = 1,
	InvalidPacket = 2,
	KeyExchange = 3,
	Logout = 4,
	Register = 5
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

enum class ENodeProcessState
{
	NotStarted,
	InProgress,
	Complete,
	Reset
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

/*--------------------------------------------------------------------------------------
	FPublicHeader
	* The public header structure for packets sent to/from the server over TCP/IP
--------------------------------------------------------------------------------------*/
struct FPublicHeader
{
	int64 PacketSize = 0;	// 0
	int EncryptionMethod = 0;	// 8
	int _reserved_ = 0;	// 12
};									// 16

/*--------------------------------------------------------------------------------------
	FPrivateHeader
	* The private header contained within the encrypted message body of incoming
	  TCP/IP packets containing protected information about the packet
--------------------------------------------------------------------------------------*/
struct FPrivateHeader
{
	int NetworkCommand = 0;	// 0
	int NetworkArgument = 0;	// 4
};									// 8

/*--------------------------------------------------------------------------------------
	FGenericPacket
	* A generic packet structure used to simply send a command code and argument
--------------------------------------------------------------------------------------*/
struct FGenericPacket
{
	int NetworkCommand = 0;	// 0
	int NetworkArgument = 0;	// 4
};									// 8

/*--------------------------------------------------------------------------------------
	FSmallPayloadPacket
	* A generic packet that also contains 128 bytes of binary data that can
	  be used to store a additional data, along with the command and argument
--------------------------------------------------------------------------------------*/
struct FSmallPayloadPacket
{
	int NetworkCommand = 0;	// 0
	int NetworkArgument = 0;	// 4
	uint8 Payload[ 128 ];			// 8
};									// 132

/*--------------------------------------------------------------------------------------
	FPingPacket
	* A packet used for sending pings between the client and server
--------------------------------------------------------------------------------------*/
struct FPingPacket
{
	int NetworkCommand = 0;	// 0
	int NetworkArgument = 0;	// 4
	long TimeStamp = 0;	// 8
};									// 16

/*--------------------------------------------------------------------------------------
	FKeyExchangePacket
	* A packet that gets sent to and from the server during the key exchange process
--------------------------------------------------------------------------------------*/
struct FKeyExchangePacket
{
	int NetworkCommand = 0;	// 0
	int NetworkArgument = 0;	// 4
	uint8 Key[ 512 ];				// 8
};									// 520

/*--------------------------------------------------------------------------------------
	FLoginPacket
	* A packet that gets sent to the server to log in to an account
--------------------------------------------------------------------------------------*/
struct FLoginPacket
{
	int NetworkCommand = 0;	// 0
	int NetworkArgument = 0;	// 4
	uint8 Username[ 128 ];
	uint8 Password[ 32 ];
};

struct FRegisterPacket
{
	int NetworkCommand = 0;
	int NetworkArgument = 0;
	uint8 DisplayName[ 256 ];
	uint8 EmailAddress[ 1024 ];
	uint8 Username[ 128 ];
	uint8 Password[ 32 ];
};

/*--------------------------------------------------------------------------------------
	FIncommingPacket
	* Used to store and pass packets incoming from the cloud
--------------------------------------------------------------------------------------*/
struct FIncomingPacket
{
	FPublicHeader PacketHeader;
	uint8* Buffer;
	size_t BufferSize;
	ENetworkCommand CommandCode;
};

struct FOutgoingPacket
{
	uint8* PacketBody = nullptr;
	size_t PacketSize = 0;
	ENetworkEncryption EncryptionMethod = ENetworkEncryption::Full;
};
