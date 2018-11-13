//
//  World.hpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/10/18.
//

#pragma once

#include "EntityBase.hpp"

#include "SingleplayerLauncher.hpp"


namespace Game
{
    class GameModeBase;
    class AuthorityBase;
    
    class World : public EntityBase
    {
        
    public:
        
        World();
        ~World();
        
        virtual void Cleanup();
        static World* GetWorld();
        
        inline GameModeBase* GetGameMode() { return GM; }
        inline AuthorityBase* GetAuthority() { return Auth; }
        
        template< typename T >
        T* GetGameMode();
        
        template< typename T >
        T* GetAuthority();
        
        inline cocos2d::Scene* GetScene() { return LinkedScene; }
        
    protected:
        
        GameModeBase* GM;
        AuthorityBase* Auth;

        static World* CurrentInstance;
    
        friend class SingleplayerLauncher;

    };
    
    template< typename T >
    T* World::GetGameMode()
    {
        static_assert( std::is_base_of< GameModeBase, T >::value, "Cast type must be derived from GameMode!" );
        return static_cast< T* >( GM );
    }
    
    template< typename T >
    T* World::GetAuthority()
    {
        static_assert( std::is_base_of< AuthorityBase, T >::value, "Cast type must be derived from Authority" );
        return static_cast< T* >( Auth );
    }
    
}
