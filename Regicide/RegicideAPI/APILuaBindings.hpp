//
//  APILuaBindings.hpp
//  Regicide
//
//  Created by Zachary Berry on 11/6/18.
//

#ifndef APILuaBindings_h
#define APILuaBindings_h

#include "LuaHeaders.hpp"
#include "API.h"


namespace Regicide
{
    
    static void LuaBind_API( lua_State* L )
    {
        CC_ASSERT( L );
        
        luabridge::getGlobalNamespace( L )
        .beginNamespace( "reg" ).beginNamespace( "api" )
            // Were not going to do anything for now, since these are probably going to be called
            // from c++ only for the time being, and some of the types are complex (ex. shared_ptr)
        .endNamespace().endNamespace();
        
    }
}


#endif /* APILuaBindings_h */
