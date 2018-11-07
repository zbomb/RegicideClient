//
//  AccountManager.cpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/2/18.
//

#include "AccountManager.hpp"
#include "Utils.h"
#include "ContentStorage.hpp"


using namespace Regicide;
using namespace rapidjson;

static AccountManager* s_Singleton = nullptr;
AccountManager* AccountManager::GetInstance()
{
    if( !s_Singleton )
    {
        s_Singleton = new AccountManager();
        s_Singleton->Init();
    }
    
    return s_Singleton;
}

AccountManager::~AccountManager()
{
    s_Singleton = nullptr;
}

void AccountManager::ForceInit()
{
    if( !s_Singleton )
    {
        s_Singleton = new AccountManager();
        s_Singleton->Init();
    }
}

void AccountManager::Init()
{
    //////////////////////////////////////////////////////////////////////////
    ////////// Load Local Account
    //////////////////////////////////////////////////////////////////////////
    auto Storage = IContentSystem::GetStorage();
    
    if( Storage->FileExists( "data/account.dat" ) )
    {
        std::string AccountData = Storage->ReadFileStr( "data/account.dat" );
        if( AccountData.empty() )
        {
            cocos2d::log( "[AccountManager] No account data stored locally!" );
            return;
        }
        
        Document Account;
        if( Account.Parse( AccountData ).HasParseError() )
        {
            cocos2d::log( "[ERROR] Failed to parse local account file!" );
            LocalAccount.reset();
            return;
        }
        
        // Read the account structure
        if( !Utils::ReadAccount( Account.GetObject(), LocalAccount ) || !LocalAccount )
        {
            cocos2d::log( "[ERROR] Failed to read local account file!" );
            LocalAccount.reset();
        }
        else
        {
            cocos2d::log( "[AccountManager] Loaded locally stored account! '%s'", LocalAccount->Info.Username.c_str() );
        }
    }
}

bool AccountManager::IsLoginStored()
{
    // We need not only a stored local account, but also an auth token
    return LocalAccount && !LocalAccount->AuthToken.empty();
}

std::string AccountManager::GetAuthToken()
{
    if( IsLoginStored() )
        return LocalAccount->AuthToken;
    
    return std::string();
}

void AccountManager::WriteAccount()
{
    if( !LocalAccount )
    {
        cocos2d::log( "[AccountManager] Attempt to write local account, but no account data is present!" );
        return;
    }
    
    // Serialize to string and pass along to ContentVault to store the data for us
    std::string AccountStr;
    Utils::WriteAccount( LocalAccount, AccountStr );
    
    std::vector< uint8 > AccountData( AccountStr.begin(), AccountStr.end() );
    
    auto cs = IContentSystem::GetStorage();
    if( !cs->WriteFile( "data/account.dat", AccountData ) )
    {
        cocos2d::log( "[ERROR] Failed to write local account to storage!" );
    }
}