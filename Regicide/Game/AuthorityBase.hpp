//
//    AuthorityBase.hpp
//    Regicide Mobile
//
//    Created: 11/11/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#pragma once

#include "EntityBase.hpp"
#include "Player.hpp"
#include <chrono>
#include "Actions.hpp"
#include "World.hpp"
#include "AuthState.hpp"


namespace Game
{
    class TurnManager;
    
    class AuthorityBase : public EntityBase
    {
        
    public:
        
        AuthorityBase();
        ~AuthorityBase();
        
        virtual void Cleanup();
        
        // GM -> Authority Calls
        // In multiplayer, these calls are sent over the network
        // Since this code is always running on the local machine, the target is always
        // assumed to be the local player. There will be seperate functions for AI
    public:
        
        virtual void SetReady() = 0;
        virtual void SetBlitzCards( const std::vector< uint32_t >& Cards ) = 0;
        virtual void PlayCard( uint32_t In, int Index ) = 0;
        virtual void FinishTurn() = 0;
        virtual void SetAttackers( const std::vector< uint32_t >& Cards ) = 0;
        virtual void SetBlockers( const std::map< uint32_t, uint32_t >& Cards ) = 0;
        virtual void TriggerAbility( uint32_t Card, uint8_t AbilityId ) = 0;
        
        inline AuthState& GetState() { return State; }
        
    protected:
        
        AuthState State;
        
        template< typename T >
        T* GetGameMode();
        
        
    };
    
    template< typename T >
    T* AuthorityBase::GetGameMode()
    {
        static_assert( std::is_base_of< GameModeBase, T >::value, "Cast type must be a subclass of GameModeBase" );
        return World::GetWorld()->GetGameMode< T >();
    }
}
