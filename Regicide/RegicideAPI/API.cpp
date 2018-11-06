//
//  API.cpp
//  Regicide
//
//  Created by Zachary Berry on 10/31/18.
//

#define API_DEV_MODE

#include "API.h"
#include "utf8.h"
#include "CryptoLibrary.h"
#include "cocos2d.h"
#include "Types.h"
#include "Numeric.h"
#include "external/json/document.h"
#include "external/json/prettywriter.h"
#include "network/HttpRequest.h"
#include "network/HttpResponse.h"
#include "network/HttpClient.h"
#include <functional>
#include <thread>
#include <future>
#include <chrono>



using namespace Regicide;
using namespace cocos2d;
using namespace cocos2d::network;
using namespace rapidjson;

#ifdef API_DEV_MODE
#define API_KEY "0sqvhV9OWX8QNKQkT9wEC9eAgayesydx5dP8E7kr"
#else
#define API_KEY "<PUBLIC API KEY>"
#endif

#define API_URL "https://api.regicidemobile.com/current/"
#define API_CONNECT_TIMEOUT 7

typedef std::function< void( HttpClient*, HttpResponse* ) > APICallback;

bool APIClient::IsAuthorized()
{
    // Authorized in this context means, we have an AuthToken, were not reffering
    // to being 'logged in'
    return AuthToken.size() > 0;
}

void APIClient::ClearAuthToken()
{
    AuthToken.clear();
}

std::vector< std::string > APIClient::BuildRequestHeader( bool bRequireAuth /* = true */, ContentType Content /* = ContentType::Json */ )
{
    if( bRequireAuth && !IsAuthorized() )
    {
        // Return an empty vector to indicate that we cant make the request
        return std::vector< std::string >();
    }
    
    // The header needs to contain the api key, auth token, and content-type
    std::vector< std::string > Output;
    
    std::stringstream KeyBuilder;
    KeyBuilder << "x-api-key: " << API_KEY;
    
    std::stringstream ContentTypeBuilder;
    ContentTypeBuilder << "Content-Type: ";
    
    if( Content == ContentType::Json )
        ContentTypeBuilder << "application/json";
    else if( Content == ContentType::HTML )
        ContentTypeBuilder << "text/html";
    else if( Content == ContentType::XML )
        ContentTypeBuilder << "application/xml";
    else
        return std::vector< std::string >();
    
    Output.push_back( KeyBuilder.str() );
    Output.push_back( ContentTypeBuilder.str() );
    
    if( bRequireAuth )
    {
        std::stringstream AuthBuilder;
        AuthBuilder << "regicide-auth: " << AuthToken;
        
        Output.push_back( AuthBuilder.str() );
    }
    
    return Output;
}


HttpRequest* APIClient::BuildRequest( const std::string& MethodPath, const std::vector<std::string>& Headers, const std::string& Content, HttpRequest::Type RequestType /* = HttpRequest::Type::POST */  )
{
    // Validate Parameters
    if( MethodPath.size() == 0 || Headers.size() < 2 || Content.size() == 0 )
        return nullptr;
    
    std::stringstream PathBuilder;
    PathBuilder << API_URL << MethodPath;
    std::string FinalPath = PathBuilder.str();
    
    HttpRequest* Request = new HttpRequest();
    Request->setUrl( FinalPath );
    Request->setHeaders( Headers );
    Request->setRequestType( RequestType );
    Request->setRequestData( Content.data(), Content.size() );
    // We wont deal with callbacks here, we want to be able to run these API calls syncronously or asyncronously
    // so we will create seperate methods to handle executing the requests
    
    return Request;
}

using namespace std::placeholders;

ExecuteResult APIClient::ExecuteBlockingRequest( HttpRequest* Request, const uint32_t Timeout, HttpResponse*& outResponse )
{
    outResponse = nullptr;
    
    if( !Request )
        return ExecuteResult::BadRequest;
    
    auto Client = HttpClient::getInstance();
    std::promise<void> Promise;

    Request->setResponseCallback( [ this, &Promise ]( HttpClient* Client, HttpResponse* Response )
                            {
                                LastHttpResponse.reset();
                                LastHttpResponse = std::make_shared<HttpResponse>( *Response );
                                Promise.set_value();
                            } );
    
    Client->setTimeoutForRead( Timeout );
    Client->setTimeoutForConnect( API_CONNECT_TIMEOUT );
    
    Client->sendImmediate( Request );
    
    // Block awaiting result
    Promise.get_future().wait();
    outResponse = LastHttpResponse.get(); // Wont reset until next call to this function
    if( outResponse == nullptr )
        return ExecuteResult::BadResponse;
    
    // Check for some basic error codes
    if( outResponse->isSucceed() )
        return ExecuteResult::Success;
    else if( outResponse->getResponseCode() == 403 )
        return ExecuteResult::AuthError;
    else if( outResponse->getResponseCode() == 400 )
        return ExecuteResult::BadRequest;
    else if( outResponse->getResponseCode() == 429 )
        return ExecuteResult::Throttled;
    else if( outResponse->getResponseCode() == 401 )
        return ExecuteResult::Unauthorized;
    else
        return ExecuteResult::OtherError;

}


ExecuteResult APIClient::ExecutePostMethod( const std::string& Function, Document& Content, Document& Response, long& StatusCode, bool bIncludeAuth /* = true */ )
{
    // Stringify input json
    StringBuffer Buffer;
    PrettyWriter< StringBuffer > Writer( Buffer );
    Content.Accept( Writer );
    
    std::string Body = Buffer.GetString();
    StatusCode = 0;
    
    // Build Http Request
    auto Headers = BuildRequestHeader( bIncludeAuth, ContentType::Json );
    auto HttpReq = BuildRequest( Function, Headers, Body, HttpRequest::Type::POST );
    
    // Check if the request was built successfully
    if( !HttpReq )
    {
        log( "[RegAPI] Failed to build API call request '%s'", Function.c_str() );
        return ExecuteResult::BadRequest;
    }
    
    // Execute Syncronously
    HttpResponse* Res = nullptr;
    auto Result = ExecuteBlockingRequest( HttpReq, 8, Res );
    
    if( Res )
        StatusCode = Res->getResponseCode();
    else
    {
        // Response null
        log( "[RegAPI] API call response null!" );
        return Result;
    }
    
    // Catch any http errors, nothing related to the api will be caught here, except for missing keys
    if( Result != ExecuteResult::Success )
    {
        log( "[RegAPI] API call failed! Error code: %ld. Error Message: %s", Res->getResponseCode(), Res->getErrorBuffer() );
        HttpReq->release();
        return Result;
    }
    
    // Parse response body
    auto ResponseData = Res->getResponseData();
    log( "%s", Res->getResponseData()->data() );
    if( Response.Parse( ResponseData->data(), ResponseData->size() ).HasParseError() )
    {
        log( "[RegAPI] API call failed! Couldnt parse json response from the server. Code: %d  Offset: %d", Response.GetParseError(), (int)Response.GetErrorOffset() );
        HttpReq->release();
        return ExecuteResult::BadResponse;
    }
    
    // Release initial request, and return
    HttpReq->release();
    return ExecuteResult::Success;
}


bool APIClient::ExecutePostMethodAsync( const std::string &Function, Document &Content, bool bIncludeAuth, std::function<void (long, Document*, std::string, void*)> Callback, const uint32_t Timeout /* = 8 */, void* AsyncState )
{
    // Validate
    if( Function.empty() || ( bIncludeAuth && IsAuthorized() ) )
        return false;
    
    // Stringify input json
    StringBuffer Buffer;
    PrettyWriter< StringBuffer > Writer( Buffer );
    Content.Accept( Writer );
    
    std::string RequestBody = Buffer.GetString();
    auto Headers = BuildRequestHeader( bIncludeAuth, ContentType::Json );
    auto Request = BuildRequest( Function, Headers, RequestBody, HttpRequest::Type::POST );
    
    if( !Request )
    {
        cocos2d::log( "[RegAPI] Failed to build async API request '%s'", Function.c_str() );
        return false;
    }
    
    auto Client = HttpClient::getInstance();
    if( !Client )
    {
        cocos2d::log( "[RegAPI] Failed to get HttpClient for API request '%s'", Function.c_str() );
        Request->release();
        return false;
    }
    
    Request->setResponseCallback( [ this, Callback, AsyncState ]( HttpClient* outClient, HttpResponse* Response )
                                 {
                                     if( !Response )
                                     {
                                         cocos2d::log( "[RegAPI] Response from API call is null!" );
                                         Callback( API_STATUS_NO_RESPONSE, nullptr, std::string(), AsyncState );
                                     }
                                     
                                     auto InitialRequest = Response->getHttpRequest();
                                     if( InitialRequest )
                                         InitialRequest->release();
                                     
                                     auto Headers = Response->getResponseHeader();
                                     std::string HeaderStr( Headers->begin(), Headers->end() );
                                     
                                     auto Data = Response->getResponseData();
#ifdef API_DEV_MODE
                                     if( Data )
                                         cocos2d::log( "%s", Data->data() );
#endif
                                     
                                     long StatusCode = Response->getResponseCode();
                                     if( Response->isSucceed() && Data )
                                     {
                                         // Were all good!
                                         Document* ResponseBody = new Document();
                                         if( ResponseBody->Parse( Data->data(), Data->size() ).HasParseError() )
                                         {
                                             cocos2d::log( "[RegAPI] Response from API call is invalid json!" );
                                             delete ResponseBody;
                                             Callback( API_STATUS_BAD_RESPONSE, nullptr, std::string(), AsyncState );
                                             return;
                                         }
                                         
                                         Callback( StatusCode, ResponseBody, HeaderStr, AsyncState );
                                     }
                                     else
                                     {
                                         Callback( StatusCode, nullptr, HeaderStr, AsyncState );
                                     }
                                 } );
    
    Client->setTimeoutForConnect( API_CONNECT_TIMEOUT );
    Client->setTimeoutForRead( Timeout );
    
    // Send Request
    Client->sendImmediate( Request );
    return true;
}
