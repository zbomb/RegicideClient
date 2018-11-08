//
//  LuaBindings_CMS.hpp
//  Regicide
//
//  Created by Zachary Berry on 11/6/18.
//

#ifndef LuaBindings_CMS_h
#define LuaBindings_CMS_h

#include <string>
#include "IContentSystem.hpp"
#include "LuaHeaders.hpp"

bool __lua_cms_WriteFile( const std::string& Path, const std::string& Data )
{
    return Regicide::IContentSystem::GetStorage()->WriteFile( Path, Data );
}

std::string __lua_cms_ReadFile( const std::string& Path )
{
    return Regicide::IContentSystem::GetStorage()->ReadFileStr( Path );
}

bool __lua_cms_FileExists( const std::string& Path )
{
    return Regicide::IContentSystem::GetStorage()->FileExists( Path );
}

bool __lua_cms_DeleteFile( const std::string& Path )
{
    return Regicide::IContentSystem::GetStorage()->DeleteFile( Path );
}

bool __lua_cms_IsLoggedIn()
{
    return Regicide::IContentSystem::GetAccounts()->IsLoginStored();
}

luabridge::LuaRef __lua_cms_GetAccount( lua_State* L )
{
    luabridge::LuaRef Output( L );
    
    auto act = Regicide::IContentSystem::GetAccounts();
    if( !act->IsLoginStored() )
        return Output;
    
    auto User = act->GetLocalAccount();
    if( !User )
        return Output;
    
    Output = luabridge::newTable( L );
    Output[ "Username" ] = User->Info.Username;
    Output[ "DisplayName" ] = User->Info.DisplayName;
    Output[ "Coins" ] = User->Info.Coins;
    Output[ "Email" ] = User->Info.Email;
    Output[ "Verified" ] = User->Info.bVerified;
    Output[ "AuthToken" ] = User->AuthToken;
    Output[ "Cards" ] = luabridge::newTable( L );
    Output[ "Decks" ] = luabridge::newTable( L );
    Output[ "Achievements" ] = luabridge::newTable( L );
    
    for( auto& C : User->Cards )
    {
        Output[ "Cards" ][ C.Id ] = C.Ct;
    }
    
    int Index = 1; // Lua uses a '1' based index
    for( auto& D : User->Decks )
    {
        Output[ "Decks" ][ Index ] = luabridge::newTable( L );
        Output[ "Decks" ][ Index ][ "Id" ] = D.Id;
        Output[ "Decks" ][ Index ][ "Name" ] = D.Name;
        Output[ "Decks" ][ Index ][ "Cards" ] = luabridge::newTable( L );
        
        for( auto& C : D.Cards )
        {
            Output[ "Decks" ][ Index ][ "Cards" ][ C.Id ] = C.Ct;
        }
        
        Index++;
    }
    
    int AchIndex = 1;
    for( auto& A : User->Achievements )
    {
        Output[ "Achievements" ][ AchIndex ] = luabridge::newTable( L );
        Output[ "Achievements" ][ AchIndex ][ "Id" ] = A.Id;
        Output[ "Achievements" ][ AchIndex ][ "State" ] = A.State;
        Output[ "Achievements" ][ AchIndex ][ "Complete" ] = A.Complete;
        
        AchIndex++;
    }
    
    return Output;
}

// Account accessors
luabridge::LuaRef __lua_GetUsername( lua_State* L )
{
    luabridge::LuaRef Output( L );
    auto act = Regicide::IContentSystem::GetAccounts();
    
    if( !act->IsLoginStored() )
        return Output;
    
    Output = act->GetLocalAccount()->Info.Username;
    return Output;
}

luabridge::LuaRef __lua_GetEmail( lua_State* L )
{
    luabridge::LuaRef Output( L );
    auto act = Regicide::IContentSystem::GetAccounts();
    
    if( !act->IsLoginStored() )
        return Output;
    
    Output = act->GetLocalAccount()->Info.Email;
    return Output;
}

luabridge::LuaRef __lua_GetCoins( lua_State* L )
{
    luabridge::LuaRef Output( L );
    auto act = Regicide::IContentSystem::GetAccounts();
    
    if( !act->IsLoginStored() )
        return Output;
    
    Output = act->GetLocalAccount()->Info.Coins;
    return Output;
}

luabridge::LuaRef __lua_GetDispName( lua_State* L )
{
    luabridge::LuaRef Output( L );
    auto act = Regicide::IContentSystem::GetAccounts();
    
    if( !act->IsLoginStored() )
        return Output;
    
    Output = act->GetLocalAccount()->Info.DisplayName;
    return Output;
}


namespace Regicide
{
    static void LuaBind_CMS( lua_State* L )
    {
        CC_ASSERT( L );
        
        using namespace luabridge;
        
        getGlobalNamespace( L )
        .beginNamespace( "reg" ).beginNamespace( "cms" )
            .addFunction( "WriteFile", &__lua_cms_WriteFile )
            .addFunction( "ReadFile", &__lua_cms_ReadFile )
            .addFunction( "FileExists", &__lua_cms_FileExists )
            .addFunction( "DeleteFile", &__lua_cms_DeleteFile )
            .addFunction( "IsLoggedIn", &__lua_cms_IsLoggedIn )
            .addFunction( "GetAccount", &__lua_cms_GetAccount )
            .addFunction( "GetUsername", &__lua_GetUsername )
            .addFunction( "GetEmail", &__lua_GetEmail )
            .addFunction( "GetDisplayName", &__lua_GetDispName )
            .addFunction( "GetCoins", &__lua_GetCoins )
        .endNamespace().endNamespace();
    }
}


#endif /* LuaBindings_CMS_h */
