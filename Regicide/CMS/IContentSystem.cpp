//
//  IContentSystem.cpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/5/18.
//

#include "IContentSystem.hpp"
#include "AccountManager.hpp"
#include "ContentManager.hpp"
#include "ContentStorage.hpp"


using namespace Regicide;

void IContentSystem::Init()
{
    AccountManager::ForceInit();
    ContentManager::ForceInit();
    ContentStorage::ForceInit();
}

IContentManager* IContentSystem::GetManager()
{
    return ContentManager::GetInstance();
}

IAccountManager* IContentSystem::GetAccounts()
{
    return AccountManager::GetInstance();
}

IContentStorage* IContentSystem::GetStorage()
{
    return ContentStorage::GetInstance();
}

