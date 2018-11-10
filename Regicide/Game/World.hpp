//
//  World.hpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/10/18.
//

#pragma once

#include "EntityBase.hpp"
#include "GameModeBase.hpp"
#include "SingleplayerLauncher.hpp"


namespace Game
{
    
    class World : public EntityBase
    {
        
    public:
        
        World();
        ~World();
        
        virtual void Cleanup();
        static World* GetWorld();
        
        template< typename T >
        T* CreateGameMode();
        
    protected:
        
        GameModeBase* GM;
        static World* CurrentInstance;
        friend class SingleplayerLauncher;
        
        
    };
    
    template< typename T >
    T* CreateGameMode()
    {
        if( GM )
        {
            cocos2d::log( "[ERROR] Attempt to create new GM when one already exists!" );
            return nullptr;
        }
        
        static_assert( std::is_base_of< GameModeBase, T >()::value, "Supplied game mode type is not a game mode!" );
        
        T* newGM = IEntityManager::GetInstance().CreateEntity< T >();
        
        if( !newGM )
            return nullptr;
        
        GM = static_cast< GameModeBase* >( newGM );
        return newGM;
    }
}
