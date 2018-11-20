//
//    LuaEngine.hpp
//    Regicide Mobile
//
//    Created: 11/6/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#pragma once

#include "LuaHeaders.hpp"
#include "LuaBridge/LuaBridge.h"


LUALIB_API void luaL_openlibs (lua_State *L);

namespace Regicide
{
    
    class LuaEngine
    {
        
    public:
        
        static LuaEngine* GetInstance();
        lua_State* State();
        void Init();
        
        LuaEngine();
        ~LuaEngine();
        
        bool RunString( const std::string& Code );
        bool RunScript( const std::string& Path );
        bool ExecuteHook( const std::string& Name, luabridge::LuaRef& Data );
        
    private:
        
        lua_State* luaState;
        bool _bIsInit;
        
    };
    
}
