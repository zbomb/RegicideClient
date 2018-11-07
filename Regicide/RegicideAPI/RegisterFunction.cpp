//
//  RegisterFunction.cpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/1/18.
//

#include "API.h"
#include "Utils.h"
#include "utf8.h"
#include "CryptoLibrary.h"
#include "external/json/document.h"
#include "external/json/prettywriter.h"
#include "network/HttpClient.h"
#include "network/HttpResponse.h"
#include "network/HttpRequest.h"


using namespace Regicide;
using namespace cocos2d::network;

typedef utf8::iterator< std::string::iterator > UTFIter;


bool ValidateUsername( std::string Username )
{
    int Length = 0;
    UTFIter UserBegin( Username.begin(), Username.begin(), Username.end() );
    UTFIter UserEnd( Username.end(), Username.begin(), Username.end() );
    
    // Check for control characters, and spaces
    // TODO: Localization
    for( auto Iter = UserBegin; Iter != UserEnd; Iter++ )
    {
        Length++;
        if(( *Iter < 0x0030 ) ||
           ( *Iter > 0x0039 && *Iter < 0x0041 ) ||
           ( *Iter > 0x005A && *Iter < 0x0061 ) ||
           ( *Iter > 0x007A ) )
        {
            return false;
        }
    }
    
    return( Length >= REG_USERNAME_MINLEN && Length <= REG_USERNAME_MAXLEN );
}

bool ValidateDispName( std::string DispName )
{
    int Length = 0;
    UTFIter DispBegin( DispName.begin(), DispName.begin(), DispName.end() );
    UTFIter DispEnd( DispName.end(), DispName.begin(), DispName.end() );
    
    // More flexible than username
    // TODO: Any other characters?
    // TODO: Localization
    for( auto Iter = DispBegin; Iter != DispEnd; Iter++ )
    {
        Length++;
        if( *Iter < 0x0020 ||
           ( *Iter > 0x007E && *Iter < 0x00A1 ) )
        {
            return false;
        }
    }
    
    return( Length >= REG_DISPNAME_MINLEN && Length <= REG_DISPNAME_MAXLEN );
}

bool ValidateEmail( std::string Email )
{
    int Length      = 0;
    bool bAtSymbol  = false;
    UTFIter EmailBegin( Email.begin(), Email.begin(), Email.end() );
    UTFIter EmailEnd( Email.end(), Email.begin(), Email.end() );

    for( auto Iter = EmailBegin; Iter != EmailEnd; Iter++ )
    {
        Length++;
        if( *Iter == 0x0040 )
        {
            bAtSymbol = true;
            break;
        }
    }
            
    return( bAtSymbol && Length >= REG_EMAIL_MINLEN && Length <= REG_EMAIL_MAXLEN );
}

bool ValidatePassword( std::string Password )
{
    UTFIter PassBegin( Password.begin(), Password.begin(), Password.end() );
    UTFIter PassEnd( Password.end(), Password.begin(), Password.end() );
    
    bool bUppercase = false;
    bool bLowercase = false;
    bool bSymbol = false;
    bool bNumber = false;
    int Length = 0;
    
    for( auto Iter = PassBegin; Iter != PassEnd; Iter++ )
    {
        Length++;
        if( *Iter < 0x0030 ||
           ( *Iter > 0x007E && *Iter < 0x00A1 ) )
        {
            return false;
        }
        
        if( !bSymbol &&
           ( *Iter < 0x0030 ||
            ( *Iter > 0x0039 && *Iter < 0x0041 ) ||
            ( *Iter > 0x005A && *Iter < 0x0061 ) ||
            ( *Iter > 0x007A && *Iter < 0x007F ) ) )
        {
            bSymbol = true;
        }
        else if( !bNumber &&
                ( *Iter > 0x002F && *Iter < 0x003A ) )
        {
            bNumber = true;
        }
        else if( !bLowercase &&
                ( *Iter > 0x0040 && *Iter < 0x005B ) )
        {
            bLowercase = true;
        }
        else if( !bUppercase &&
                ( *Iter > 0x0060 && *Iter < 0x007B ) )
        {
            bUppercase = true;
        }
        
        if( bSymbol && bNumber && bLowercase && bUppercase )
            break;
    }
    
    if( !bNumber || !bLowercase || !bUppercase )
    {
        return false;;
    }
    
    return( bNumber && bLowercase && bUppercase && Length >= REG_PASSWORD_MINLEN );
}


bool ReadRegisterResponse( Document* inDoc, RegisterResponse& Output )
{
    if( !inDoc || !inDoc->HasMember( "Result" ) || !inDoc->HasMember( "Token" ) ||
       !inDoc->HasMember( "Account" ) || !(*inDoc)[ "Result" ].IsUint() )
    {
        cocos2d::log( "[RegAPI] Register API Call Failed! Response was missing members" );
        Output.Result = RegisterResult::Error;
        return false;
    }
    
    Output.Result = RegisterResult( (*inDoc)[ "Result" ].GetUint() );
    if( Output.Result != RegisterResult::Success )
    {
        return false;
    }
    
    if( !(*inDoc)[ "Token" ].IsString() || !(*inDoc)[ "Account" ].IsObject() )
    {
        cocos2d::log( "[RegAPI] Register API Call Failed! Server responded with OK but response was missing data" );
        Output.Result = RegisterResult::SuccessBadResponse;
        return false;
    }
    
    Output.AuthToken = (*inDoc)[ "Token" ].GetString();
    if( !Utils::ReadAccount( (*inDoc)[ "Account" ].GetObject(), Output.Account ) )
    {
        cocos2d::log( "[RegAPI] Register API Call Failed! Server responded with OK but the account data was corrupted" );
        Output.Result = RegisterResult::SuccessBadResponse;
        Output.AuthToken = "";
        Output.Account.reset();
        return false;
    }
    
    return true;
}

RegisterResponse APIClient::Register( const RegisterRequest &Request )
{
    RegisterResponse Output = RegisterResponse();
    Output.Result = RegisterResult::Error;
    
    // Validate Parameters
    if( !ValidateUsername( Request.Username ) )
    {
        Output.Result = RegisterResult::InvalidUsername;
        return Output;
    }
    if( !ValidateEmail( Request.EmailAdr ) )
    {
        Output.Result = RegisterResult::InvalidEmail;
        return Output;
    }
    if( !ValidateDispName( Request.DispName ) )
    {
        Output.Result = RegisterResult::InvalidDispName;
        return Output;
    }
    if( !ValidatePassword( Request.Password ) )
    {
        Output.Result = RegisterResult::BadPassHash;
        return Output;
    }
    
    // Hash Password
    auto PassHash = CryptoLibrary::HashPassword( Request.Password, Request.Username );
    
    Document RequestBody;
    Document ResponseBody;
    RequestBody.SetObject();
    RequestBody.AddMember( "Username", StringRef( Request.Username ), RequestBody.GetAllocator() );
    RequestBody.AddMember( "PassHash", StringRef( PassHash ), RequestBody.GetAllocator() );
    RequestBody.AddMember( "DispName", StringRef( Request.DispName ), RequestBody.GetAllocator() );
    RequestBody.AddMember( "Email", StringRef( Request.EmailAdr ), RequestBody.GetAllocator() );
    
    auto ExecRes = ExecutePostMethod( "account/register", RequestBody, ResponseBody, Output.ResponseCode, false );
    
    if( ExecRes != ExecuteResult::Success )
    {
        // Failed to call API
        cocos2d::log( "[RegAPI] Failed to execute API call (Register). Code: %ld", Output.ResponseCode );
        Output.Result = RegisterResult::Error;
        return Output;
    }
    
    // Read the response
    if( ReadRegisterResponse( &ResponseBody, Output ) )
    {
        AuthToken = Output.AuthToken;
    }
    
    return Output;
}


bool APIClient::RegisterAsync( const RegisterRequest &Request, std::function<void (RegisterResponse)> Callback )
{
    // Validate Parameters
    if( !ValidateUsername( Request.Username ) )
    {
        RegisterResponse Output;
        Output.Result = RegisterResult::InvalidUsername;
        Callback( Output );
        return true;
    }
    if( !ValidateEmail( Request.EmailAdr ) )
    {
        RegisterResponse Output;
        Output.Result = RegisterResult::InvalidEmail;
        Callback( Output );
        return true;
    }
    if( !ValidateDispName( Request.DispName ) )
    {
        RegisterResponse Output;
        Output.Result = RegisterResult::InvalidDispName;
        Callback( Output );
        return true;
    }
    if( !ValidatePassword( Request.Password ) )
    {
        RegisterResponse Output;
        Output.Result = RegisterResult::BadPassHash;
        Callback( Output );
        return true;
    }
    
    // Hash Password
    auto PassHash = CryptoLibrary::HashPassword( Request.Password, Request.Username );
    
    Document RequestBody;
    RequestBody.SetObject();
    RequestBody.AddMember( "Username", StringRef( Request.Username ), RequestBody.GetAllocator() );
    RequestBody.AddMember( "PassHash", StringRef( PassHash ), RequestBody.GetAllocator() );
    RequestBody.AddMember( "DispName", StringRef( Request.DispName ), RequestBody.GetAllocator() );
    RequestBody.AddMember( "Email", StringRef( Request.EmailAdr ), RequestBody.GetAllocator() );
    
    return ExecutePostMethodAsync( "account/register", RequestBody, false,
                                  [ this, Callback ]( long StatusCode, Document* ResponseBody, std::string Headers )
                                  {
                                      // Check Http Status Code
                                      RegisterResponse Response;
                                      if( StatusCode != API_STATUS_SUCCESS )
                                      {
                                          cocos2d::log( "[API] Register Async API call failed! Code: %ld", StatusCode );
                                          Response.Result = RegisterResult::Error;
                                      }
                                      else
                                      {
                                          // Read Response
                                          if( ReadRegisterResponse( ResponseBody, Response ) )
                                          {
                                              AuthToken = Response.AuthToken;
                                          }
                                      }
                                      
                                      // Execute callback and cleanup
                                      if( ResponseBody ) { delete ResponseBody; }
                                      Response.ResponseCode = StatusCode;
                                      Callback( Response );
                                      
                                  } );
}
