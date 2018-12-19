//
//	GameState.hpp
//	Regicide Mobile
//
//	Created: 12/1/18
//	Updated: 12/1/18
//
//	© 2018 Zachary Berry, All Rights Reserved
//

#pragma once

#include "Actions.hpp"
#include "ObjectStates.hpp"
#include "RegicideAPI/Account.hpp"
#include "CardEntity.hpp"


namespace Game
{
    class Player;
    

    
    class GameStateBase
    {
    public:
        
        // This interface is for lua callable actions.
        // The copy on the authority will build and run action queues while updating the locally stored game state
        // The copy in the simulator will just update state
        
        // The simulation needs more than a state store, there also needs to be a mechanism to advance turns and what not.
        // If we create a 'Simulation' class, that contains the logic for a simulation, and have the GameState as a member
        // then, we can pass the game state to lua functions, and be able to call actions through that interface
        
        // In the authority, we can subclass this, and alter the behavior to build queues in addition to the normal state activity
        
        // For setup, we will leave that functionality to the authoritystate subclass and AuthorityBase
        // In a simulation, we will just copy from GameStateBase -> GameStateBase so we can make a tidy function for that
        
        GameStateBase();
        
        void CopyFrom( GameStateBase& Other );

        inline PlayerState* GetPlayer() { return &LocalPlayer; }
        inline PlayerState* GetOpponent() { return &Opponent; }
        
        // Lua Interface
        // Player Targeted Actions
        virtual void ShuffleDeck( PlayerState* Target );
        virtual void DrawCard( PlayerState* Target, uint32_t Count );
        virtual void TakeMana( PlayerState* Target, CardState* Origin, uint32_t Amount );
        virtual void GiveMana( PlayerState* Target, CardState* Origin, uint32_t Amount );
        virtual void DamageKing( PlayerState* Target, CardState* Origin, uint32_t Amount );
        virtual void HealKing( PlayerState* Target, CardState* Origin, uint32_t Amount );
        
        // Card Targeted Actions
        virtual void DoCombat( CardState* Target, CardState* Origin, uint32_t Amount, int StaminaChange );
        virtual void DamageCard( CardState* Target, CardState* Origin, uint32_t Amount );
        virtual void HealCard( CardState* Target, CardState* Origin, uint32_t Amount );
        virtual void GiveStamina( CardState* Target, CardState* Origin, uint32_t Amount );
        virtual void TakeStamina( CardState* Target, CardState* Origin, uint32_t Amount );
        
        virtual void OnCardKilled( CardState* Dead );
        virtual bool PlayCard( PlayerState* Player, CardState* Target );
        virtual bool PlayCard( PlayerState* Player, uint32_t Target );
        virtual uint32_t DrawSingle( PlayerState* Target );
        virtual PlayerTurn SwitchPlayerTurn();
        
        void SetStartingPlayer( PlayerTurn In );
        bool FindCard( uint32_t In, CardState*& Out );
        bool FindCard( uint32_t In, PlayerState* Owner, CardState*& Out, bool bFieldOnly = false );
        bool FindPlayer( uint32_t In, PlayerState*& Owner );
        
        bool IsPlayerTurn( uint32_t Target );
        
        // Lua Specific Interface
        PlayerState* GetCardOwner( CardState* Card );
        PlayerState* GetCardOpponent( CardState* Card );
        PlayerState* GetOtherPlayer( PlayerState* Target );
        
        MatchState mState;
        PlayerTurn pState;
        TurnState tState;
        
        int TurnNumber;
        
        // Hooks
        void CallHook( const std::string& HookName );
        
        template< typename T1 >
        void CallHook( const std::string& HookName, T1 Arg1 );
        
        template< typename A1, typename A2 >
        void CallHook( const std::string& HookName, A1 Arg1, A2 Arg2 );
        
        template< typename B1, typename B2, typename B3 >
        void CallHook( const std::string& HookName, B1 Arg1, B2 Arg2, B3 Arg3 );
        
    protected:
        
        PlayerState LocalPlayer;
        PlayerState Opponent;
        PlayerTurn StartingPlayer;
        
        virtual bool PreHook( const std::string& HookName );
        virtual void PostHook();
        
        void ExecuteOnPlayerCards( PlayerState* Target, std::function< void( CardState* ) > Func );
        void ExecuteOnCards( std::function< void( CardState* ) > Func );
        
    };
    
    template< typename T1 >
    void GameStateBase::CallHook( const std::string& HookName, T1 Arg1 )
    {
        if( !PreHook( HookName ) )
            return;
        
        // Ensure theres an active action queue, we will create one if needed
        auto& CM = CardManager::GetInstance();
        
        // Loop through all cards in game, and call the hook
        ExecuteOnCards( [ & ] ( CardState* Card )
           {
               if( Card )
               {
                   // We need to get info for this card
                   CardInfo Info;
                   PlayerState* Owner = nullptr;
                   
                   if( CM.GetInfo( Card->Id, Info ) && FindPlayer( Card->Owner, Owner ) && Owner )
                   {
                       if( Info.Hooks && ( *Info.Hooks )[ HookName ] && (*Info.Hooks )[ HookName ].isFunction() )
                       {
                           // Call hook using current active queue, card owner, and this card
                           try
                           {
                               ( *Info.Hooks )[ HookName ]( *this, *Card, Arg1 );
                           }
                           catch( std::exception& e )
                           {
                               cocos2d::log( "[State] Failed to run card hook! %s", e.what() );
                           }
                       }
                   }
               }
           } );
        
        PostHook();
    }
    
    template< typename A1, typename A2 >
    void GameStateBase::CallHook( const std::string& HookName, A1 Arg1, A2 Arg2 )
    {
        if( !PreHook( HookName ) )
            return;
        
        // Ensure theres an active action queue, we will create one if needed
        auto& CM = CardManager::GetInstance();
        
        // Loop through all cards in game, and call the hook
        ExecuteOnCards( [ & ] ( CardState* Card )
           {
               if( Card )
               {
                   // We need to get info for this card
                   CardInfo Info;
                   PlayerState* Owner = nullptr;
                   
                   if( CM.GetInfo( Card->Id, Info ) && FindPlayer( Card->Owner, Owner ) && Owner )
                   {
                       if( Info.Hooks && ( *Info.Hooks )[ HookName ] && (*Info.Hooks )[ HookName ].isFunction() )
                       {
                           // Call hook using current active queue, card owner, and this card
                           try
                           {
                               ( *Info.Hooks )[ HookName ]( *this, *Card, Arg1, Arg2 );
                           }
                           catch( std::exception& e )
                           {
                               cocos2d::log( "[State] Failed to run card hook! %s", e.what() );
                           }
                       }
                   }
               }
           } );
        
        PostHook();
    }
    
    template< typename B1, typename B2, typename B3 >
    void GameStateBase::CallHook( const std::string& HookName, B1 Arg1, B2 Arg2, B3 Arg3 )
    {
        if( !PreHook( HookName ) )
            return;
        
        // Ensure theres an active action queue, we will create one if needed
        auto& CM = CardManager::GetInstance();
        
        // Loop through all cards in game, and call the hook
        ExecuteOnCards( [ & ] ( CardState* Card )
           {
               if( Card )
               {
                   // We need to get info for this card
                   CardInfo Info;
                   PlayerState* Owner = nullptr;
                   
                   if( CM.GetInfo( Card->Id, Info ) && FindPlayer( Card->Owner, Owner ) && Owner )
                   {
                       if( Info.Hooks && ( *Info.Hooks )[ HookName ] && (*Info.Hooks )[ HookName ].isFunction() )
                       {
                           // Call hook using current active queue, card owner, and this card
                           try
                           {
                               ( *Info.Hooks )[ HookName ]( *this, *Card, Arg1, Arg2, Arg3 );
                           }
                           catch( std::exception& e )
                           {
                               cocos2d::log( "[State] Failed to run card hook! %s", e.what() );
                           }
                       }
                   }
               }
           } );
        
        PostHook();
    }
    
}
