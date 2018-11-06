#include "CryptoLibrary.h"
#include "cipher.h"
#include "sha256.h"
#include "cocos/base/CCConsole.h"
#include <string>
#include <sstream>
#include <iomanip>
#include "utf8.h"
#include <iostream>
#include "Numeric.h"
#include "API.h"

using namespace cocos2d;

bool CryptoLibrary::AesOperation( std::vector< uint8 >& Data, const uint8 Key[ 32 ], const uint8 IV[ 16 ], AES_OPERATION Operation )
{
	try
	{
		mbedtls_cipher_context_t CipherContext;
		mbedtls_cipher_init( &CipherContext );

		const mbedtls_cipher_info_t* CipherInfo = mbedtls_cipher_info_from_type( MBEDTLS_CIPHER_AES_256_CBC );

		// Ensure we got info about the cipher we need to perform
		if( !CipherInfo )
		{
			return AesError( "getting cipher info" );
		}

		if( mbedtls_cipher_setup( &CipherContext, CipherInfo ) != 0 )
		{
			return AesError( "setting up cipher" );
		}

		if( mbedtls_cipher_set_padding_mode( &CipherContext, MBEDTLS_PADDING_PKCS7 ) != 0 )
		{
			return AesError( "setting the padding mode" );
		}

		if( int Result = mbedtls_cipher_setkey( &CipherContext, Key, 256,
			Operation == AES_OPERATION::ENCRYPT ? mbedtls_operation_t::MBEDTLS_ENCRYPT : mbedtls_operation_t::MBEDTLS_DECRYPT ) != 0 )
		{
			log( "[DEBUG] ERROR SETTING KEY! ERROR CODE: %d", (int) Result );
			return AesError( "setting the key" );
		}

		// Unfortunately, we need to allocate a vector to hold the encrypted data
		size_t OutputSize = Data.size() + mbedtls_cipher_get_block_size( &CipherContext );
		std::vector< uint8 > RawOutput( OutputSize );

		// Perform cipher operation
		int AesResult = mbedtls_cipher_crypt( &CipherContext, IV, 16, Data.data(), Data.size(), RawOutput.data(), &OutputSize );

		if( AesResult == 0 )
		{
			// Move data back into the input vector
			Data.resize( OutputSize, 0 );
			std::move( RawOutput.begin(), RawOutput.begin() + OutputSize, Data.begin() );
			
			return true;
		}
		else if( AesResult == MBEDTLS_ERR_CIPHER_BAD_INPUT_DATA )
		{
			return AesError( "encryption/decryption because of bad input data" );
		}
		else if( AesResult == MBEDTLS_ERR_CIPHER_FULL_BLOCK_EXPECTED )
		{
			return AesError( "encryption/decryption because an incomplete block was provided" );
		}
		else if( AesResult == MBEDTLS_ERR_CIPHER_INVALID_PADDING )
		{
			return AesError( "encryption/decryption because the padding was invalid" );
		}
		else
		{
			return AesError( "encryption/decryption because an unknown error occurred" );
		}
	}
	catch( ... )
	{
		log( "[ERROR] An exception was thrown while performing an AES operation! (%s)", Operation == AES_OPERATION::DECRYPT ? "Decrypt" : "Encrypt" );
		return false;
	}
}


bool CryptoLibrary::AesError( std::string FailurePoint )
{
	log( "[ERROR] AES operation failed! An error has occurred while %s", FailurePoint.c_str() );
	return false;
}


bool CryptoLibrary::AesEncrypt( std::vector< uint8 >& Data, const uint8 Key[ 32 ], const uint8 IV[ 16 ] )
{
	return AesOperation( Data, Key, IV, AES_OPERATION::ENCRYPT );
}


bool CryptoLibrary::AesDecrypt( std::vector< uint8 >& Data, const uint8 Key[ 32 ], const uint8 IV[ 16 ] )
{
	return AesOperation( Data, Key, IV, AES_OPERATION::DECRYPT );
}


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

std::string CryptoLibrary::Base64Encode( std::vector< uint8 > Input )
{
    std::string ret;
    int in_len = Input.size();
    const uint8* bytes_to_encode = Input.data();
    
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

std::vector< uint8 > CryptoLibrary::Base64Decode( std::string Input )
{
    int in_len = Input.size();
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
