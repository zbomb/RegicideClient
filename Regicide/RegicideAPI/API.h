//
//  API.h
//  Regicide
//
//  Created by Zachary Berry on 10/31/18.
//
#pragma once

#include "Numeric.h"
#include "Types.h"
#include "network/HttpRequest.h"
#include <future>
#include "external/json/document.h"

#define API_DEV_MODE

using namespace rapidjson;
using namespace cocos2d;
using namespace cocos2d::network;

#define API_STATUS_NO_RESPONSE  -1
#define API_STATUS_BAD_RESPONSE -2
#define API_STATUS_THROTTLED    429
#define API_STATUS_BAD_REQUESR  400
#define API_STATUS_AUTH_ERROR   403
#define API_STATUS_UNAUTHORIZED 401
#define API_STATUS_ERROR        500
#define API_STATUS_SUCCESS      200

namespace Regicide
{
    enum class ContentType
    {
        Json,
        XML,
        HTML
    };
    
    enum class ExecuteResult
    {
        BadRequest,
        BadResponse,
        Timeout,
        Unauthorized,
        Throttled,
        AuthError,
        OtherError,
        Success
    };
    
    static class APIClient* s_Singleton = nullptr;

    class APIClient
    {
        
    public:
        /*========================================================
            Singleton Access Pattern
        ========================================================*/
        static APIClient* GetInstance()
        {
            if( !s_Singleton )
                s_Singleton = new APIClient();
            
            return s_Singleton;
        }
        
        LoginResponse Login( const LoginRequest& Request );
        bool LoginAsync( const LoginRequest& Request, std::function<void( LoginResponse )> Callback );
        
        RegisterResponse Register( const RegisterRequest& Request );
        bool RegisterAsync( const RegisterRequest& Request, std::function< void( RegisterResponse )> Callback );
        
        LogoutResponse Logout( const LogoutRequest& Request );
        
        bool IsAuthorized();
        void ClearAuthToken();
        
        ExecuteResult ExecutePostMethod( const std::string& Function, Document& Content, Document& Response, long& StatusCode, bool bIncludeAuth = true );
        bool ExecutePostMethodAsync( const std::string& Function, Document& Content, bool bIncludeAuth, std::function<void( long, Document*, std::string, void* )> Callback, const uint32_t Timeout = 8, void* AsyncState = nullptr );
        
        ~APIClient()
        {
            LastHttpResponse.reset();
        }
        
    private:
        
        std::string AuthToken;
        std::vector< std::string > BuildRequestHeader( bool bRequireAuth = true, ContentType Content = ContentType::Json );
        
        HttpRequest* BuildRequest( const std::string& MethodPath, const std::vector< std::string >& Header, const std::string& Content, HttpRequest::Type RequestType = HttpRequest::Type::POST );
        
        ExecuteResult ExecuteBlockingRequest( HttpRequest* Request, const uint32_t Timeout, HttpResponse*& outResponse );
        
        std::shared_ptr<HttpResponse> LastHttpResponse;        
    };

    constexpr auto REG_USERNAME_MINLEN =    5;
    constexpr auto REG_USERNAME_MAXLEN =    32;
    constexpr auto REG_PASSWORD_MINLEN =    5;
    constexpr auto REG_EMAIL_MINLEN    =         3;
    constexpr auto REG_EMAIL_MAXLEN    =         255;
    constexpr auto REG_DISPNAME_MINLEN =    5;
    constexpr auto REG_DISPNAME_MAXLEN =    48;
}

