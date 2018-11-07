//
//  Types.h
//  Regicide
//
//  Created by Zachary Berry on 10/31/18.
//
#pragma once

#include <string>
#include "Numeric.h"
#include "Account.h"


namespace Regicide
{
    /*========================================
            Login
     ========================================*/
    enum class LoginResult
    {
        BadRequest = 0,
        InvalidCredentials = 1,
        Success = 2,
        DatabaseError = 3,
        OtherError = 4
    };
    
    struct LoginRequest
    {
        std::string Username;
        std::string Password;
    };
    
    struct LoginResponse
    {
        LoginResult Result;
        std::shared_ptr< UserAccount > Account;
        std::string AuthToken;
        long StatusCode;
    };
    
    /*========================================
            Register
     ========================================*/
    enum class RegisterResult 
    {
        Error = 0,
        InvalidUsername = 1,
        InvalidDispName = 2,
        InvalidEmail = 3,
        UsernameTaken = 4,
        EmailExists = 5,
        BadPassHash = 6,
        Success = 7,
        SuccessBadResponse = 8
    };
    
    struct RegisterRequest
    {
        std::string Username;
        std::string Password;
        std::string DispName;
        std::string EmailAdr;
    };
    
    struct RegisterResponse
    {
        RegisterResult Result;
        std::shared_ptr< UserAccount > Account;
        std::string AuthToken;
        long ResponseCode;
    };
    
    /*========================================
            Logout
     ========================================*/
    enum class LogoutResult
    {
        Error = 0,
        InvalidToken = 1,
        Success = 2
    };
    
    struct LogoutResponse
    {
        LogoutResult Result;
        long StatusCode;
    };
    
    /*========================================
            Verify
     ========================================*/
    struct VerifyResponse
    {
        bool Result;
        long StatusCode;
    };
}
