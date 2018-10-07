#pragma once
#include <vector>
#include "NetHeaders.h"

typedef std::vector< uint8 >::iterator ByteIter;

class CryptoLibrary
{
	
public:

	template< typename T >
	static T* DeserializePacket( ByteIter Start, ByteIter End );

	template< typename T >
	static std::vector< uint8 > SerializePacket( const T& Input );

	template< typename T >
	static bool DeserializeCopy( ByteIter Start, ByteIter End, T& Output );

private:

	enum AES_OPERATION { ENCRYPT, DECRYPT };
	static bool AesOperation( std::vector< uint8 >& Data, const uint8 Key[ 32 ], const uint8 IV[ 16 ], AES_OPERATION Operation );
	static bool AesError( std::string FailurePoint );

public:

	static bool AesEncrypt( std::vector< uint8 >& Data, const uint8 Key[ 32 ], const uint8 IV[ 16 ] );
	static bool AesDecrypt( std::vector< uint8 >& Data, const uint8 Key[ 32 ], const uint8 IV[ 16 ] );

	static std::vector< uint8 > SHA256( std::vector< uint8 >& Data );

	static void PrintVector( std::vector< uint8 >& Data, std::string Name );
	
};


template< typename T >
T* CryptoLibrary::DeserializePacket( ByteIter Start, ByteIter End )
{
	static_assert( End >= Start, "Invalid byte iterator start/end provided" );

	// Ensure we have enough data to create this object
	size_t InputSize = End - Start;
	if( InputSize < sizeof( T ) )
	{
		printf( "[ERROR] Deserialization error! There was not enough data supplied to deseraialize into the desired type!\n" );
		return nullptr;
	}
	else if( !std::is_trivially_copyable< T >::value )
	{
		printf( "[ERROR] Deserialization error! Specified type is not copyable!" );
		return nullptr;
	}

	T* Output = new T();
	
	uint8* ObjStart = reinterpret_cast< uint8* >( std::addressof( *Output ) );
	std::copy( Start, End, ObjStart );

	return Output;
}

template< typename T >
bool CryptoLibrary::DeserializeCopy( ByteIter Start, ByteIter End, T& Output )
{
	static_assert( End >= Start, "Invalid byte iterator start/end provided!" );

	size_t InputSize = End - Start;
	if( InputSize < sizeof( T ) )
	{
		printf( "[ERROR] Deserialization error! There was not enough data supplied to deseraialize into the desired type!\n" );
		return false;
	}

	uint8* ObjStart = reinterpret_cast< uint8* >( std::addressof( Output ) );
	std::copy( Start, End, ObjStart );

	return true;
}

template< typename T > 
std::vector< uint8 > CryptoLibrary::SerializePacket( const T& Input )
{
	if( !Input )
	{
		printf( "[ERROR] Failed to serialize packet because the supplied data was null!\n" );
		return std::vector< uint8 >( 0 );
	}

	std::vector< uint8 > Output( sizeof( T ) );
	const uint8* ObjStart = reinterpret_cast< const uint8* >( std::addressof( Input ) );
	const uint8* ObjEnd = ObjStart + sizeof( T );

	std::copy( ObjStart, ObjEnd, Output.begin() );

	return Output;
}

