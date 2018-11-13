//
//  GameModeBase.hpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/10/18.
//

#pragma once
#include "EntityBase.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "GameModeBase.hpp"
#include "AuthorityBase.hpp"


namespace Game
{
    
    class GameModeBase : public EntityBase
    {
    public:
        
        GameModeBase();
        ~GameModeBase();
        
        virtual void Cleanup();
        virtual Player* GetLocalPlayer() = 0;
        virtual Player* GetOpponent() = 0;
        
        virtual void TouchBegan( cocos2d::Touch* inTouch, CardEntity* inCard ) = 0;
        virtual void TouchEnd( cocos2d::Touch* inTouch, CardEntity* inCard ) = 0;
        virtual void TouchMoved( cocos2d::Touch* inTouch ) = 0;
        virtual void TouchCancel( cocos2d::Touch* inTouch ) = 0;

    protected:
        
        World* GetWorld() const;
        
        template< typename T >
        T* GetAuthority() const;
    
    protected:
        
        virtual void Initialize() = 0;
        virtual void PostInitialize() = 0;
        
    };
    
    template< typename T >
    T* GameModeBase::GetAuthority() const
    {
        static_assert( std::is_base_of< AuthorityBase, T >::value, "Cast type must be a subclass of AuthorityBase" );
        auto world = GetWorld();
        auto auth = world->GetAuthority< T >();
        
        // Should we assert?
        if( !auth )
            cocos2d::log( "[FATAL] Failed to get reference to authority from within GameMode!" );

        return auth;
    }
}
