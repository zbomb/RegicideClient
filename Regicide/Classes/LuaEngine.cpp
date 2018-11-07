//
//  LuaEngine.cpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/6/18.
//

#include "LuaEngine.hpp"
#include "Utils.h"

// Include funcs needed for binding
#include "RegicideAPI/APILuaBindings.hpp"
#include "CryptoLibrary.h"
#include "CMS/LuaBindings_CMS.hpp"


using namespace Regicide;

static LuaEngine* lua_s_Singleton = nullptr;
LuaEngine* LuaEngine::GetInstance()
{
    if( lua_s_Singleton == nullptr )
    {
        lua_s_Singleton = new (std::nothrow) LuaEngine();
    }
    
    return lua_s_Singleton;
}

LuaEngine::LuaEngine()
{
    _bIsInit = false;
}

LuaEngine::~LuaEngine()
{
    lua_close( luaState );
    s_Singleton = nullptr;
}

static void lua_Print( const char *Str )
{
    if( !Str )
        return;
    
    cocos2d::log( "%s", Str );
}

static void lua_Include( const std::string& Path, lua_State* L )
{
    if( Path.size() == 0 )
    {
        cocos2d::log( "[Lua] Failed to include file because path was empty!" );
        return;
    }
    
    std::string FullPath = FileUtils::getInstance()->fullPathForFilename( Path );
    auto Result = luaL_dofile( L, FullPath.c_str() );
    
    if( Result != 0 )
    {
        cocos2d::log( "[Lua] Error! Failed to run '%s' 0x%x", FullPath.c_str(), Result );
        cocos2d::log( "[Lua] Error: %s", lua_tostring( L, -1 ) );
    }
    
}

bool LuaEngine::RunScript( const std::string& Path )
{
    lua_Include( Path, luaState );
    return true;
}

void LuaEngine::Init()
{
    CC_ASSERT( !_bIsInit );
    
    // Create lua state
    luaState = luaL_newstate();
    CC_ASSERT( luaState );
    
    // Load standard libs
    luaL_openlibs( luaState );
    
    // Add our namespaces, functions will be registered in the modules
    luabridge::getGlobalNamespace( luaState )
    .beginNamespace( "reg" )
        .beginNamespace( "util" ).endNamespace()
        .beginNamespace( "api" ).endNamespace()
        .beginNamespace( "cms" ).endNamespace()
        .beginNamespace( "crypto" ).endNamespace()
    .endNamespace()
    .addFunction( "__print", lua_Print )
    .addFunction( "include", lua_Include );
    
    luabridge::setGlobal( luaState, true, "CLIENT" );
    luabridge::setGlobal( luaState, false, "SERVER" );
    
    using namespace luabridge;
    auto printf = getGlobal( luaState, "__print" );
    printf( "[Regicide] Initializing Lua Client..." );
    
    // Bind everything..
    Regicide::LuaBind_API( luaState );
    CryptoLibrary::LuaBind( luaState );
    Regicide::LuaBind_CMS( luaState );
    
    // Run Lua entry point
    lua_Include( "core/main.lua", luaState );
}


lua_State* LuaEngine::State()
{
    return luaState;
}
