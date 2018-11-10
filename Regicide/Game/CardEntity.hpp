//
//  CardEntity.hpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/10/18.
//

#pragma once

#include "EntityBase.hpp"
#include "cocos2d.h"
#include "LuaEngine.hpp"

namespace Game
{
    enum class CardLocation
    {
        Deck,
        Hand,
        Play,
        Dead
    };
    
    
    class CardEntity : public EntityBase
    {
        
    public:
        
        CardEntity();
        ~CardEntity();
        
        std::string DisplayName;
        uint16 CardId;
        
        uint16 Power;
        uint16 Stamina;
        
        bool bAllowDeckHooks;
        bool bAllowHandHooks;
        bool bAllowPlayHooks;
        bool bAllowDeadHooks;
        
        CardLocation Location;
        
        cocos2d::Sprite* Sprite;
        std::shared_ptr< luabridge::LuaRef > Hooks;
        
        bool Load( luabridge::LuaRef& inLua, bool Authority = false );
        bool ShouldCallHook() const;
        bool ShouldCallHook( const std::string& HookName );
        bool GetHook( const std::string& HookName, luabridge::LuaRef& outFunc );
        
    protected:
        
        virtual void Cleanup();
    };
}
