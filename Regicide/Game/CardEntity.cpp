//
//  CardEntity.cpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/10/18.
//

#include "CardEntity.hpp"

using namespace Game;



CardEntity::CardEntity()
    : EntityBase( "card" )
{
}

CardEntity::~CardEntity()
{
    
}

void CardEntity::Cleanup()
{
    bAllowDeckHooks     = false;
    bAllowHandHooks     = false;
    bAllowPlayHooks     = false;
    bAllowDeadHooks     = false;
    
    Power       = 0;
    Stamina     = 0;
    Location    = CardLocation::Deck;
}

bool CardEntity::Load( luabridge::LuaRef& inLua, bool Authority /* = false */ )
{
    if( !inLua.isTable() )
    {
        cocos2d::log( "[Card] ERROR: Failed to load, invalid card table passed into Load!" );
        return false;
    }
    
    // Check validity of the hook table
    if( !inLua[ "Power" ].isNumber() || !inLua[ "Stamina" ].isNumber() || !inLua[ "Name" ].isString() || !inLua[ "Hooks" ].isTable() )
    {
        cocos2d::log( "[Card] ERROR: Failed to load, card table passed into load was missing values" );
        return false;
    }
    
    // Assign Members
    DisplayName     = inLua[ "Name" ].tostring();
    Power           = (uint16) int( inLua[ "Power" ] );
    Stamina         = (uint16) int( inLua[ "Stamina" ] );
    
    bAllowDeckHooks = inLua[ "EnableDeckHooks" ];
    bAllowHandHooks = inLua[ "EnableHandHooks" ];
    bAllowPlayHooks = inLua[ "EnableHandHooks" ];
    bAllowDeadHooks = inLua[ "EnableDeadHooks" ];
    
    // Store Hooks
    *Hooks = inLua[ "Hooks" ];
    
    // Set Authority Flag
    (*Hooks)[ "Authority" ] = Authority;
    
    return true;
    
}

bool CardEntity::ShouldCallHook() const
{
    if( Location == CardLocation::Deck && bAllowDeckHooks )
        return true;
    if( Location == CardLocation::Hand && bAllowHandHooks )
        return true;
    if( Location == CardLocation::Play && bAllowPlayHooks )
        return true;
    if( Location == CardLocation::Dead && bAllowDeadHooks )
        return true;
    
    return false;
}

bool CardEntity::ShouldCallHook( const std::string& HookName )
{
    if( !ShouldCallHook() )
        return false;
    
    // Check if this hook exists
    return ( Hooks && (*Hooks)[ HookName ].isFunction() );
}


bool CardEntity::GetHook( const std::string &HookName, luabridge::LuaRef& outFunc )
{
    if( Hooks )
    {
        if( (*Hooks)[ HookName ].isFunction() )
        {
            outFunc = (*Hooks)[ HookName ];
            return true;
        }
    }
    
    return false;
}
