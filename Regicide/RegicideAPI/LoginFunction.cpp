//
//  LoginFunction.cpp
//  Regicide
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

bool ValidateParameters( std::string Username, std::string Password )
{
    auto UsernameLen = utf8::distance( Username.begin(), Username.end() );
    auto PasswordLen = utf8::distance( Password.begin(), Password.end() );
    
    if( UsernameLen < Regicide::REG_USERNAME_MINLEN || UsernameLen > Regicide::REG_USERNAME_MAXLEN || PasswordLen < Regicide::REG_PASSWORD_MINLEN )
        return false;
    
    if( Password.size() < Regicide::REG_PASSWORD_MINLEN )
        return false;
    
    return true;
}


bool ReadLoginResponse( Document* Response, LoginResponse& Output )
{
    // Parse Response
    if( !Response->HasMember( "Result" ) || !Response->HasMember( "AuthToken" ) ||
       !Response->HasMember( "Account" ) || !(*Response)[ "Result" ].IsInt() )
    {
        Output.Result = LoginResult::OtherError;
        return false;
    }
    
    // Get Login Result, if it wasnt successful then return
    Output.Result = LoginResult( (*Response)[ "Result" ].GetInt() );
    
    if( Output.Result != LoginResult::Success )
    {
        return false;
    }
    
    if( !(*Response)[ "AuthToken" ].IsString() || !(*Response)[ "Account" ].IsObject() )
    {
        cocos2d::log( "[RegAPI] Login was successful, but auth token/account info was missing from result!" );
        Output.Result = LoginResult::OtherError;
        return false;
    }
    
    // Response appears good, login was successful, so read the rest of the response
    Output.AuthToken = (*Response)[ "AuthToken" ].GetString();
    
    if( !Regicide::Utils::ReadAccount( (*Response)[ "Account" ].GetObject(), Output.Account ) )
    {
        cocos2d::log( "[RegAPI] Login was successful, but the account couldnt be parsed from the response" );
        Output.Result = LoginResult::OtherError;
        Output.AuthToken = "";
        Output.Account.reset();
        return false;
    }
    
    return true;
}


LoginResponse APIClient::Login( const LoginRequest& Request )
{
    LoginResponse Output = LoginResponse();
    Output.Result       = LoginResult::InvalidCredentials;
    
    // Validate Parameters
    if( !ValidateParameters( Request.Username, Request.Password ) )
    {
        return Output;
    }
    
    // Hash Password
    std::string FinalPass = CryptoLibrary::HashPassword( Request.Password, Request.Username );
    if( FinalPass.empty() )
    {
        cocos2d::log( "[RegAPI] Login API Call Failed! Couldnt hash password!" );
        Output.Result = LoginResult::OtherError;
        return Output;
    }
    
    // Build Request Body With Json
    Document Body;
    Body.SetObject();
    
    Body.AddMember( "Username", StringRef( Request.Username ), Body.GetAllocator() );
    Body.AddMember( "PassHash", StringRef( FinalPass ), Body.GetAllocator() );
    
    Document Response;
    
    // Perform API Call
    auto Result = ExecutePostMethod( "account/login", Body, Response, Output.StatusCode, false, true );
    
    if( Result != ExecuteResult::Success )
    {
        cocos2d::log( "[RegAPI] Login API call failed! Check logs for error source. Code: %ld", Output.StatusCode );
        return Output;
    }
    
    // Process Results
    if( ReadLoginResponse( &Response, Output ) )
    {
        AuthToken = Output.AuthToken;
    }
    
    return Output;
}


bool APIClient::LoginAsync( const LoginRequest &Request, std::function<void (LoginResponse)> Callback )
{
    if( !ValidateParameters( Request.Username, Request.Password ) )
    {
        LoginResponse Response;
        Response.Result = LoginResult::InvalidCredentials;
        
        Callback( Response );
        return true;
    }
    
    std::string FinalPass = CryptoLibrary::HashPassword( Request.Password, Request.Username );
    
    // Build Request Body With Json
    Document Body;
    Body.SetObject();
    
    Body.AddMember( "Username", StringRef( Request.Username ), Body.GetAllocator() );
    Body.AddMember( "PassHash", StringRef( FinalPass ), Body.GetAllocator() );
    
    return ExecutePostMethodAsync( "account/login", Body, false,
              [ this, Callback ]( long StatusCode, Document* ResponseBody, std::string Headers )
              {
                  LoginResponse Response;
                  if( StatusCode != API_STATUS_SUCCESS )
                  {
                      cocos2d::log( "[RegAPI] Async Login API call failed! StatusCode: %ld", StatusCode );
                      Response.Result = LoginResult::OtherError;
                  }
                  else
                  {
                      if( ReadLoginResponse( ResponseBody, Response ) )
                      {
                          AuthToken = Response.AuthToken;
                      }
                  }

                  if( ResponseBody ) { delete ResponseBody; }
                  Response.StatusCode = StatusCode;
                  Callback( Response );
                  
              }, 8, true );
}
