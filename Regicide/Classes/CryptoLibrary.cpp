#include "CryptoLibrary.h"
#include "cryptolib/cipher.h"
#include "cryptolib/sha256.h"



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

		if( !mbedtls_cipher_setup( &CipherContext, CipherInfo ) )
		{
			return AesError( "setting up cipher" );
		}

		if( !mbedtls_cipher_set_padding_mode( &CipherContext, MBEDTLS_PADDING_PKCS7 ) )
		{
			return AesError( "setting the padding mode" );
		}

		if( !mbedtls_cipher_setkey( &CipherContext, Key, 32,
			Operation == AES_OPERATION::ENCRYPT ? mbedtls_operation_t::MBEDTLS_ENCRYPT : mbedtls_operation_t::MBEDTLS_DECRYPT ) )
		{
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
		printf( "[ERROR] An exception was thrown while performing an AES operation! (%s)\n", Operation == AES_OPERATION::DECRYPT ? "Decrypt" : "Encrypt" );
		return false;
	}
}


bool CryptoLibrary::AesError( std::string FailurePoint )
{
	printf( "[ERROR] AES operation failed! An error has occurred while %s\n", FailurePoint.c_str() );
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

		if( int Result = mbedtls_sha256_starts_ret( &Context, 0 ) )
		{
			printf( "[Cryptography] SHA256 algorithm threw error on start (Code: %d)\n", Result );
			return std::vector< uint8 >( 0 );
		}

		if( int Result = mbedtls_sha256_update_ret( &Context, Data.data(), Data.size() ) )
		{
			printf( "[Cryptography] SHA256 algorithm threw an error while feeding data (Code: %d)\n", Result );
			return std::vector< uint8 >( 0 );
		}

		std::vector< uint8 > Output( 32 );

		if( int Result = mbedtls_sha256_finish_ret( &Context, Output.data() ) )
		{
			printf( "[Cryptography SHA256 algorithm threw an error while completing function (Code %d)\n", Result );
			return std::vector< uint8 >( 0 );
		}

		mbedtls_sha256_free( &Context );

		return Output;
	}
	catch( ... )
	{
		printf( "[ERROR] An error has occurred while computing SHA-256 hash!\n" );
		return std::vector< uint8 >( 0 );
	}
}


void CryptoLibrary::PrintVector( std::vector< uint8 >& Data, std::string Name )
{
	printf( "\n======================= Debug Vector Print =============================\n" );
	printf( "-----> %s   Size: %d \n", Name.c_str(), Data.size() );

	uint32 Index = 0;
	for( auto It = Data.begin(); It != Data.end(); It++, Index++ )
	{
		if( Index >= 16 )
		{
			Index = 0;
			printf( "\n" );
		}

		printf( "%02x   ", Data[ Index ] );
	}
	printf( "\n" );
	printf( "============================== End Vector ================================\n" );
}
