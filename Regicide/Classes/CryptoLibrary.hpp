//
//    CryptoLibrary.hpp
//    Regicide Mobile
//
//    Created: 10/9/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#pragma once
#include <vector>
#include "Numeric.hpp"


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

public:

	static std::vector< uint8 > SHA256( std::vector< uint8 >& Data );
    static std::string SHA256( std::string& Data );

	static void PrintVector( const std::vector< uint8 >& Data, std::string Name );
    
    static std::string Base64Encode( std::vector< uint8 > Input );
    static std::vector< uint8 > Base64Decode( std::string Input );
    
    static std::string HashPassword( const std::string& Password, const std::string& Username );
    
    static void LuaBind( class lua_State* L );
};


template< typename T >
T* CryptoLibrary::DeserializePacket( ByteIter Start, ByteIter End )
{
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

