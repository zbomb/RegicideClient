//
//    APILuaBindings.hpp
//    Regicide Mobile
//
//    Created: 11/6/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#ifndef APILuaBindings_h
#define APILuaBindings_h

#include "LuaHeaders.hpp"
#include "API.hpp"


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
