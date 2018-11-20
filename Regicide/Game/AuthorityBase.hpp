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


namespace Game
{
    class TurnManager;
    
    class AuthorityBase : public EntityBase
    {
        
    public:
        
        AuthorityBase();
        ~AuthorityBase();
        
        virtual void Cleanup();
        
        inline Player* GetPlayer()      { return GetWorld()->GetLocalPlayer(); }
        inline Player* GetOpponent()    { return GetWorld()->GetOpponent(); }
        
        // GM -> Authority Calls
        // In multiplayer, these calls are sent over the network
        // Since this code is always running on the local machine, the target is always
        // assumed to be the local player. There will be seperate functions for AI
    public:
        
        virtual void SetReady() = 0;
        virtual void SetBlitzCards( const std::vector< CardEntity* >& Cards ) = 0;
        virtual void PlayCard( CardEntity* In, int Index ) = 0;
        virtual void FinishTurn() = 0;
        virtual void SetAttackers( const std::vector< CardEntity* >& Cards ) = 0;
        virtual void SetBlockers( const std::map< CardEntity*, CardEntity* >& Cards ) = 0;
        virtual void TriggerAbility( CardEntity* Card, uint8_t AbilityId ) = 0;
        
    protected:
        
        MatchState mState;
        TurnState tState;
        PlayerTurn pState;
        
        template< typename T >
        T* GetGameMode();
        
        virtual bool DrawCard( Player* In, uint32_t Count = 1 ) = 0;
        
        
    };
    
    template< typename T >
    T* AuthorityBase::GetGameMode()
    {
        static_assert( std::is_base_of< GameModeBase, T >::value, "Cast type must be a subclass of GameModeBase" );
        return World::GetWorld()->GetGameMode< T >();
    }
}
