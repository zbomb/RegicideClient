//
//    Actions.hpp
//    Regicide Mobile
//
//    Created: 11/15/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#pragma once
#include <string>
#include <vector>
#include <chrono>
#include "EntityBase.hpp"

namespace Game
{
    enum class MatchState
    {
        PreMatch,
        CoinFlip,
        Blitz,
        Main,
        PostMatch
    };
    
    enum class TurnState
    {
        PreTurn,
        Marshal,
        Attack,
        Block,
        Damage,
        PostTurn,
        None
    };
    
    enum PlayerTurn
    {
        LocalPlayer,
        Opponent,
        None
    };
    
    class Action
    {
    public:
        
        std::string Name;
        
        Action( const std::string& In )
        : Name( In )
        {}
        
        virtual ~Action()
        {}
        
    };
    
    class ParallelAction : public Action
    {
    public:
        
        std::vector< std::unique_ptr< Game::Action > > Actions;
        int Counter;
        
        ParallelAction()
        : Action( "Parallel" ), Counter( 0 )
        {}
        
        template< typename T >
        T* CreateAction()
        {
            static_assert( std::is_base_of< Game::Action, T >::value, "Template argument must be derived from Action!" );
            
            Actions.push_back( std::move( std::unique_ptr< T >( new (std::nothrow) T() ) ) );
            return dynamic_cast< T* >( Actions.back().get() );
        }
    };
    
    static uint32_t _nextQueueId = 0;
    class ActionQueue
    {
    public:
        
        uint32_t Identifier;
        uint32_t Position;
        std::function< void() > Callback;
        std::vector< std::unique_ptr< Game::Action > > Actions;
        
        ActionQueue()
        : Callback( nullptr ), Identifier( ++_nextQueueId ), Position( 0 )
        {}
        
        ActionQueue( std::function< void() > InCallback )
        : Callback( InCallback ), Identifier( ++_nextQueueId ), Position( 0 )
        {}
        
        template< typename T >
        T* CreateAction()
        {
            // Check type, create new action, move into vector, return reference
            static_assert( std::is_base_of< Game::Action, T >::value, "Template argument must be derived from Action" );
            
            Actions.push_back( std::move( std::unique_ptr< T >( new (std::nothrow) T() ) ) );
            
            // We need to get the new unique_ptr we added, get the raw pointer and cast back to desired type
            auto& newEntry = Actions.back();
            return dynamic_cast< T* >( newEntry.get() );
        }
    
    };
    
    // Targets players
    class PlayCardAction : public Action
    {
    public:
        
        PlayCardAction()
        : Action( "PlayCard" ), bNeedsMove( true ), bWasSuccessful( true ),
        TargetCard( 0 ), TargetIndex( -1 ), TargetPlayer( 0 )
        {}
        
        // Todo: Simplify to only TargetPlayer, TargetCard and TargetIndex (index is basically a passthrough)
        
        bool bNeedsMove;
        bool bWasSuccessful;
        uint32_t TargetCard;
        int32_t TargetIndex;
        uint32_t TargetPlayer;
    };
    
    // Targets players
    class UpdateManaAction : public Action
    {
    public:
        
        UpdateManaAction()
        : Action( "UpdateMana" )
        {}
        
        uint32_t TargetPlayer;
        int Amount;
    };
    
    // Targets players
    class DrawCardAction : public Action
    {
    public:
        
        DrawCardAction()
        : Action( "DrawCard" )
        {}
        
        uint32_t TargetPlayer;
        uint32_t TargetCard;
    };
    
    // Targets card
    class LoadCardAction : public Action
    {
    public:
        
        LoadCardAction()
        : Action( "LoadCard" )
        {}
        
        // TODO: Decide if we should send json or a c-struct
        uint32_t CardId;
        uint16_t Power;
        uint16_t Stamina;
        std::string Texture;
        std::string FullTexture;
    };
    
    // Targets GM
    class CoinFlipAction : public Action
    {
    public:
        
        CoinFlipAction()
        : Action( "CoinFlip" )
        {}
        
        uint32_t Player;
    };
    
    // Generic Action
    class TimedQueryAction : public Action
    {
    public:
        
        TimedQueryAction()
        : Action( "TimedQuery" )
        {}
        
        std::chrono::steady_clock::time_point Deadline;
    };
    
    // Generic Action
    class EventAction : public Action
    {
    public:
        
        EventAction()
        : Action( "Event" )
        {}
    };
    
    class CardErrorAction : public Action
    {
    public:
        
        CardErrorAction()
        : Action( "BlitzError" )
        {}
        
        std::map< uint32_t, uint8_t > Errors;
    };
    
    class TurnStartAction : public Action
    {
    public:
        
        TurnStartAction()
        : Action( "TurnStart" )
        {}
        
        uint32_t Player;
    };
    
    // Damage
    class DamageAction : public Action
    {
    public:
        
        DamageAction()
        : Action( "Damage" )
        {}
        
        uint32_t Target;
        uint32_t Inflictor;
        int UpdatedPower;
        uint16_t Damage;
    };
    
    class UpdateStaminaAction : public Action
    {
    public:
        
        UpdateStaminaAction()
        : Action( "UpdateStamina" )
        {}
        
        uint32_t Target;
        uint32_t Inflictor;
        uint16_t Amount;
        int UpdatedAmount;
    };
    
    class WinAction : public Action
    {
    public:
        
        WinAction()
        : Action( "Win" )
        {}
        
        uint32_t Player;
    };
    
    class CombatAction : public Action
    {
    public:
        
        CombatAction()
        : Action( "Combat" )
        {}
        
        uint32_t Attacker;
        uint32_t Blocker;
        int AttackerPower;
        int BlockerPower;
        uint16_t Damage;
    };
    
    class PlayerEventAction : public Action
    {
    public:
        
        PlayerEventAction()
        : Action( "PlayerEvent" )
        {}
        
        uint32_t Player;
    };
    
    class CardListEvent : public Action
    {
    public:
        
        CardListEvent()
        : Action( "CardList" )
        {}
        
        std::vector< uint32_t > Cards;
    };
    
}
