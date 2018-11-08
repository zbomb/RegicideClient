//
//  LuaEngine.hpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/6/18.
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
        
        bool RunScript( const std::string& Path );
        bool ExecuteHook( const std::string& Name, luabridge::LuaRef& Data );
        
    private:
        
        lua_State* luaState;
        bool _bIsInit;
        
    };
    
}
