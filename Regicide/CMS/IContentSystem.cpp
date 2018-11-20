//
//    IContentSystem.cpp
//    Regicide Mobile
//
//    Created: 11/5/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
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

