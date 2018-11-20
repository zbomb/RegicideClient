//
//    CryptoLibrary.cpp
//    Regicide Mobile
//
//    Created: 10/9/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#include "CryptoLibrary.hpp"
#include "cryptolib/sha256.h"
#include "base/CCConsole.h"
#include <string>
#include <sstream>
#include <iomanip>
#include "utf8/utf8.h"
#include <iostream>
#include "Numeric.hpp"
#include "RegicideAPI/API.hpp"
#include "LuaHeaders.hpp"

using namespace cocos2d;

std::vector< uint8 > CryptoLibrary::SHA256( std::vector< uint8 >& Data )
{
	try
	{
		mbedtls_sha256_context Context;
		mbedtls_sha256_init( &Context );

		if( int Result = mbedtls_sha256_starts_ret( &Context, 0 ) != 0 )
		{
			log( "[Cryptography] SHA256 algorithm threw error on start (Code: %d)", Result );
			return std::vector< uint8 >( 0 );
		}

		if( int Result = mbedtls_sha256_update_ret( &Context, Data.data(), Data.size() ) != 0 )
		{
			log( "[Cryptography] SHA256 algorithm threw an error while feeding data (Code: %d)", Result );
			return std::vector< uint8 >( 0 );
		}

		std::vector< uint8 > Output( 32 );

		if( int Result = mbedtls_sha256_finish_ret( &Context, Output.data() ) != 0 )
		{
			log( "[Cryptography SHA256 algorithm threw an error while completing function (Code %d)", Result );
			return std::vector< uint8 >( 0 );
		}

		mbedtls_sha256_free( &Context );

		return Output;
	}
	catch( ... )
	{
		log( "[ERROR] An error has occurred while computing SHA-256 hash!" );
		return std::vector< uint8 >( 0 );
	}
}


std::string CryptoLibrary::SHA256( std::string& Data )
{
    try
    {
        mbedtls_sha256_context Context;
        mbedtls_sha256_init( &Context );
        
        if( int Result = mbedtls_sha256_starts_ret( &Context, 0 ) != 0 )
        {
            log( "[Cryptography] SHA256 algorithm threw error on start (Code: %d)", Result );
            return std::string();
        }
        
        if( int Result = mbedtls_sha256_update_ret( &Context, (uint8*)Data.data(), Data.size() ) != 0 )
        {
            log( "[Cryptography] SHA256 algorithm threw an error while feeding data (Code: %d)", Result );
            return std::string();
        }
        
        std::string Output( ' ', 32 );
        
        if( int Result = mbedtls_sha256_finish_ret( &Context, (uint8*)Output.data() ) != 0 )
        {
            log( "[Cryptography SHA256 algorithm threw an error while completing function (Code %d)", Result );
            return std::string();
        }
        
        mbedtls_sha256_free( &Context );
        
        return Output;
    }
    catch( ... )
    {
        log( "[ERROR] An error has occurred while computing SHA-256 hash!" );
        return std::string();
    }
}


void CryptoLibrary::PrintVector( const std::vector< uint8 >& Data, std::string Name )
{
	log( "\n======================= Debug Vector Print =============================" );
	log( "-----> %s   Size: %d", Name.c_str(), (uint)Data.size() );

	std::ostringstream Output;

	uint32_t Index = 0;
	for( auto It = Data.begin(); It != Data.end(); It++, Index++ )
	{
		if( Index >= 16 )
		{
			Index = 0;
			Output << "\n";
		}

		Output << "0X" << std::hex << (int) *It << "   ";
	}

	Output << "\n";
	Output << "============================== End Vector ================================";

    log( "%s", Output.str().c_str() );
}

static const std::string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

static inline bool is_base64( unsigned char c )
{
    return( isalnum( c ) || ( c == '+' ) || ( c == '/' ) );
}


std::string Base64EncodePtr( const uint8* bytes_to_encode, size_t in_len )
{
    std::string ret;
    
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    
    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            for(i = 0; (i <4) ; i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }
    
    if (i)
    {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\0';
        
        char_array_4[0] = ( char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        
        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];
        
        while((i++ < 3))
            ret += '=';
    }
    
    return ret;
}

std::string Base64EncodeStr( std::string Input )
{
    return Base64EncodePtr( (uint8*) Input.data(), Input.size() );
}


std::string CryptoLibrary::Base64Encode( std::vector< uint8 > Input )
{
    return Base64EncodePtr( Input.data(), Input.size() );
}

std::string Base64DecodeStr( std::string Input )
{
    auto in_len = Input.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;
    
    while (in_len-- && ( Input[in_] != '=') && is_base64(Input[in_])) {
        char_array_4[i++] = Input[in_]; in_++;
        if (i ==4) {
            for (i = 0; i <4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);
            
            char_array_3[0] = ( char_array_4[0] << 2       ) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) +   char_array_4[3];
            
            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }
    
    if (i) {
        for (j = 0; j < i; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);
        
        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        
        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }
    
    return ret;
}

std::vector< uint8 > CryptoLibrary::Base64Decode( std::string Input )
{
    auto ret = Base64DecodeStr( Input );
    return std::vector< uint8 >( ret.begin(), ret.end() );
}


std::string CryptoLibrary::HashPassword( const std::string& Password, const std::string& Username )
{
    // We dont want to perform validation here but, we need to have
    // enough data to performing the salting, we will leave utf8 checking
    // and length contraints to the caller
    if( Password.size() < Regicide::REG_PASSWORD_MINLEN || Username.size() < Regicide::REG_USERNAME_MINLEN )
        return std::string();
    
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
            return std::string();
        }
    }
    
    return CryptoLibrary::Base64Encode( PasswordData );
}

// Explicit parameters, the cryptolibrary method has overloads
std::string __lua_sha256( std::string Input )
{
    return CryptoLibrary::SHA256( Input );
}

std::string __lua_hash_password( std::string Password, std::string Username )
{
    return CryptoLibrary::HashPassword( Password, Username );
}

std::string __lua_base64_enc( std::string Input )
{
    return Base64EncodeStr( Input );
}

std::string __lua_base64_dec( std::string Input )
{
    return Base64DecodeStr( Input );
}


void CryptoLibrary::LuaBind( class lua_State *L )
{
    CC_ASSERT( L );
    
    using namespace luabridge;
    
    getGlobalNamespace( L )
    .beginNamespace( "reg" ).beginNamespace( "crypto" )
        .addFunction( "SHA256", &__lua_sha256 )
        .addFunction( "HashPassword", &__lua_hash_password )
        .addFunction( "Base64Encode", &__lua_base64_enc )
        .addFunction( "Base64Decode", &__lua_base64_dec )
    .endNamespace().endNamespace();
    
}
