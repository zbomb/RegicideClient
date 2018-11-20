//
//    EventDataTypes.hpp
//    Regicide Mobile
//
//    Created: 10/14/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#pragma once

#include "cocos2d.h"
#include "LuaHeaders.hpp"
#include "LuaBridge/LuaBridge.h"

struct EventData
{
    virtual void ToLua( luabridge::LuaRef& Out ) const = 0;
};

struct NullEventData : EventData
{
    virtual void ToLua( luabridge::LuaRef& Out ) const
    {}
};

struct NumericEventData : EventData
{
    int Data;
    
    NumericEventData( int inData )
        : Data( inData )
    {}
    
    virtual void ToLua( luabridge::LuaRef& Out ) const
    {
        Out = Data;
    }
};

struct StringEventData : EventData
{
    std::string Data;
    
    StringEventData( const std::string& inData )
        : Data( inData )
    {}
    
    virtual void ToLua( luabridge::LuaRef& Out ) const
    {
        Out = Data;
    }
};

