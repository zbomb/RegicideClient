//
//    LuaEngine.cpp
//    Regicide Mobile
//
//    Created: 11/6/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#include "LuaEngine.hpp"
#include "Utils.hpp"
#include "EventHub.hpp"

// Include funcs needed for binding
#include "RegicideAPI/APILuaBindings.hpp"
#include "CryptoLibrary.hpp"
#include "CMS/LuaBindings_CMS.hpp"
#include "Game/Game_LuaBindings.hpp"


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
    
    cocos2d::log( "[Lua] %s", Str );
}

void lua_Include( const std::string& Path, lua_State* L )
{
    if( Path.size() == 0 )
    {
            cocos2d::log( "[Lua] Failed to include file because path was empty!" );
        return;
    }
    
    std::string FullPath = FileUtils::getInstance()->fullPathForFilename( Path );
    if( FileUtils::getInstance()->isFileExist( FullPath ) )
    {
        auto Result = luaL_dofile( L, FullPath.c_str() );
        
        if( Result != 0 )
        {
                cocos2d::log( "[Lua] Failed to include '%s'! (%d) An error occured while loading the file.\nError: %s", FullPath.c_str(), Result, lua_tostring( L, -1 ) );
        }
    }
    else
    {
        cocos2d::log( "[Lua] Failed to include '%s'! The file doesnt exist", FullPath.c_str() );
    }
    
}

std::vector< std::string > LoadedFiles;
void lua_Require( const std::string& File, lua_State* L )
{
    // To implement the 'require' feature, we need to ensure were not loading the same file twice
    // In vanilla lua, If you call require( "asdf.lua" ) and require( "asdf" ) it will actually load
    // the same file twice, but if the calls were identical it would only include once
    // In our implementation, we will store the absolute path to the file instead, so each file can only
    // be included once, regardless of input
    
    if( File.empty() )
    {
        cocos2d::log( "[Lua] Error: Failed to require file! The given filename was empty" );
        return;
    }
    
    auto file = FileUtils::getInstance();
    std::vector< std::string > LuaDirectoryFiles;
    
    auto ScriptDir = file->fullPathForFilename( "LuaScripts" );
    file->listFilesRecursively( ScriptDir, &LuaDirectoryFiles );
    
    std::string FoundFile;
    
    // Perform Lookup
    for( auto& F : LuaDirectoryFiles )
    {
        std::string Path = F; // Copy, so we can access original
        if( Path.back() == '/' || Path.size() <= ScriptDir.size() + 1 ) // Skip directories, and empty results
            continue;
        
        Path.erase( Path.begin(), Path.begin() + ScriptDir.size() + 1 ); // Get path relative to LuaScripts & trim leading '/'
        if( Path.find( File ) != std::string::npos )
        {
            if( FoundFile.empty() )
                FoundFile = F;
            else
            {
                // If we already found a file, then throw an error
                // But first, we want to convert FoundFile to a local path
                FoundFile.erase( FoundFile.begin(), FoundFile.begin() + ScriptDir.size() + 1 );
                    cocos2d::log( "Failed to require '%s'! Multiple files were found with the specified argument. '%s' and '%s'",
                                 File.c_str(), FoundFile.c_str(), Path.c_str() );
                return;
            }
        }
    }
    
    // Check if anything was found
    if( FoundFile.empty() )
    {
            cocos2d::log( "Failed to require '%s'! Nothing was found using the specified search string", File.c_str() );
        return;
    }
    
    // Check if this was loaded already
    for( auto It = LoadedFiles.begin(); It != LoadedFiles.end(); It++ )
    {
        if( It->compare( FoundFile ) == 0 )
            return;
    }
    
    // Add file to loaded list, and load it!
    LoadedFiles.push_back( FoundFile );
    if( luaL_dofile( L, FoundFile.c_str() ) != 0 )
    {
            cocos2d::log( "Failed to require '%s'! An error occured while loading the file.\nError: %s", FoundFile.c_str(), lua_tostring( L, -1 ) );
    }
}


bool LuaEngine::RunScript( const std::string& Path )
{
    lua_Include( Path, luaState );
    return true;
}

bool LuaEngine::RunString( const std::string& Code )
{
    if( Code.empty() || !luaState )
        return false;
    
    return luaL_dostring( luaState, Code.c_str() ) == 0;
}

int lua_HandleError( lua_State* L )
{
    cocos2d::log( "[Lua] Error: %s", lua_tostring( L, -1 ) );
    return 1;
}

void __lua_callhook_string( const std::string& Hook, const std::string& Data )
{
    EventHub::Execute( Hook, StringEventData( Data ) );
}

void __lua_callhook_null( const std::string& Hook )
{
    EventHub::Execute( Hook, NullEventData() );
}

void __lua_callhook_number( const std::string& Hook, int Data )
{
    EventHub::Execute( Hook, NumericEventData( Data ) );
}

void LuaEngine::Init()
{
    CC_ASSERT( !_bIsInit );
    
    // Create lua state
    luaState = luaL_newstate();
    CC_ASSERT( luaState );
    
    // Load standard libs
    luaL_openlibs( luaState );
    
    // Setup error handling
    lua_atpanic( luaState, &lua_HandleError );
    
    // Add our namespaces, functions will be registered in the modules
    luabridge::getGlobalNamespace( luaState )
    .beginNamespace( "reg" )
        .beginNamespace( "util" ).endNamespace()
        .beginNamespace( "api" ).endNamespace()
        .beginNamespace( "cms" ).endNamespace()
        .beginNamespace( "crypto" ).endNamespace()
    .endNamespace()
    .addFunction( "__print", &lua_Print )
    .addFunction( "__include", &lua_Include )
    .addFunction( "__require", &lua_Require )
    .addFunction( "__c_hook_null", &__lua_callhook_null )
    .addFunction( "__c_hook_string", &__lua_callhook_string )
    .addFunction( "__c_hook_number", &__lua_callhook_number );
    
    luabridge::setGlobal( luaState, false, "SERVER" );
    
    using namespace luabridge;
    auto printf = getGlobal( luaState, "__print" );
    printf( "[Regicide] Initializing Lua Engine..." );
    
    // Bind everything..
    Regicide::LuaBind_API( luaState );
    CryptoLibrary::LuaBind( luaState );
    Regicide::LuaBind_CMS( luaState );
    Regicide::LuaBind_Game( luaState );
    
    // Run Lua entry point
    lua_Include( "core/main.lua", luaState );

}


lua_State* LuaEngine::State()
{
    return luaState;
}

bool LuaEngine::ExecuteHook( const std::string& HookName, luabridge::LuaRef& Data )
{
    if( !luaState )
        return false;
    
    auto hook = luabridge::getGlobal( luaState, "hook" );
    if( !hook.isTable() )
    {
        cocos2d::log( "[Lua] Error! Failed to call hook! _G.hook doesnt exist!" );
        return false;
    }
    
    try
    {
        return hook[ "Call" ]( HookName, Data );
        
    } catch ( std::exception& e )
    {
        cocos2d::log( "[Lua] Error: %s", e.what() );
    }

    return false;
}
