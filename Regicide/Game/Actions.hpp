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
    
    // Action Base Class
    class Action
    {
    public:
        // The target to call ExecuteAction on
        uint32_t Target;
        
        // The name of the action to run
        std::string ActionName;
        
        // Boolean to track if the action has started
        bool bHasStarted;
        
        Action* Owner;
        
        std::vector< std::unique_ptr< Game::Action > > Children;
        
        Action( const std::string& In, EntityBase* InTarget = nullptr )
        : ActionName( In ), bHasStarted( false ), Owner( nullptr )
        {
            if( InTarget )
                Target = InTarget->GetEntityId();
            else
                Target = 0;
        }
        
        std::unique_ptr< Game::Action >& AddAction( Game::Action* newAction )
        {
            // Using Copy Ellison to move unique_ptr into children
            Children.push_back( std::unique_ptr< Game::Action >( newAction )  );
            auto& ret = Children.back();
            ret->Owner = this;
            return ret;
        }
        
        template< typename T >
        T* CreateAction( Game::EntityBase* InTarget = nullptr )
        {
            // Check type, create new action, move into vector, return reference
            static_assert( std::is_base_of< Game::Action, T >::value, "Template argument must be derived from Action" );
            
            Children.push_back( std::move( std::unique_ptr< T >( new (std::nothrow) T( InTarget ) ) ) );
            
            auto& newEntry = Children.back();
            newEntry->Owner = this;
            return dynamic_cast< T* >( newEntry.get() );
        }
        
        virtual ~Action()
        {
            Children.clear();
        }
        
    };
    
    static uint32_t _nextQueueId = 0;
    class ActionQueue
    {
    public:
        
        uint32_t Identifier;
        std::function< void() > Callback;
        std::vector< std::unique_ptr< Game::Action > > ActionTree;
        uint32_t Counter;
        bool Executing;
        
        ActionQueue()
        : Callback( nullptr ), Identifier( ++_nextQueueId ), Counter( 0 ), Executing( false )
        {}
        
        ActionQueue( std::function< void() > InCallback )
        : Callback( InCallback ), Identifier( ++_nextQueueId ), Counter( 0 ), Executing( false )
        {}
        
        std::unique_ptr< Game::Action >& AddAction( Game::Action* newAction )
        {
            // Using Copy Ellison to move unique_ptr into ActionTree
            ActionTree.push_back( std::unique_ptr< Game::Action >( newAction )  );
            return ActionTree.back();
        }
        
        template< typename T >
        T* CreateAction( Game::EntityBase* InTarget = nullptr )
        {
            // Check type, create new action, move into vector, return reference
            static_assert( std::is_base_of< Game::Action, T >::value, "Template argument must be derived from Action" );
            
            ActionTree.push_back( std::move( std::unique_ptr< T >( new (std::nothrow) T( InTarget ) ) ) );
            
            // We need to get the new unique_ptr we added, get the raw pointer and cast back to desired type
            auto& newEntry = ActionTree.back();
            return dynamic_cast< T* >( newEntry.get() );
        }
        
        void AddActionLua( Game::Action* newAction )
        {
            AddAction( newAction );
        }
        
    private:
        
        uint32_t _CountActionBranch( std::vector< std::unique_ptr< Game::Action > >::iterator It )
        {
            // Were going to count any null pointers, because when we go to execute, we will
            // decrement the action counter when we come across a null action. We dont have a way
            // to check if the action became null AFTER the count, so we will just count everything
            uint32_t Output = 1;
            
            if( *It )
            {
                for( auto ChIt = (*It)->Children.begin(); ChIt != (*It)->Children.end(); ChIt++ )
                {
                    Output += _CountActionBranch( ChIt );
                }
            }
            
            return Output;
        }
        
    public:
        
        // Counts up the number of total actions contained in the action tree
        // 'Counter' gets set to this value
        void CountActions()
        {
            Counter = 0;
            
            for( auto It = ActionTree.begin(); It != ActionTree.end(); It++ )
            {
                Counter += _CountActionBranch( It );
            }
        }
    
    };
    
    // Targets players
    class PlayCardAction : public Action
    {
    public:
        
        PlayCardAction( EntityBase* In = nullptr )
        : Action( "PlayCard", In ), bNeedsMove( true ), bWasSuccessful( true ),
        TargetCard( 0 ), TargetIndex( -1 )
        {}
        
        
        bool bNeedsMove;
        bool bWasSuccessful;
        uint32_t TargetCard;
        int32_t TargetIndex;
        
    };
    
    // Targets players
    class UpdateManaAction : public Action
    {
    public:
        
        UpdateManaAction( EntityBase* In = nullptr )
        : Action( "UpdateMana", In )
        {}
        
        uint32_t UpdatedMana;
    };
    
    // Targets players
    class DrawCardAction : public Action
    {
    public:
        
        DrawCardAction( EntityBase* In = nullptr )
        : Action( "DrawCard", In )
        {}
        
        uint32_t TargetCard;
    };
    
    // Targets card
    class LoadCardAction : public Action
    {
    public:
        
        LoadCardAction( EntityBase* In = nullptr )
        : Action( "LoadCard", In )
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
        
        CoinFlipAction( EntityBase* In = nullptr )
        : Action( "CoinFlip", In )
        {}
        
        PlayerTurn StartingPlayer;
    };
    
    // Generic Action
    class TimedQueryAction : public Action
    {
    public:
        
        TimedQueryAction( EntityBase* In = nullptr )
        : Action( "TimedQuery", In )
        {}
        
        std::chrono::steady_clock::time_point Deadline;
    };
    
    // Generic Action
    class EventAction : public Action
    {
    public:
        
        EventAction( EntityBase* In = nullptr )
        : Action( "Event", In )
        {}
    };
    
    class CardErrorAction : public Action
    {
    public:
        
        CardErrorAction( EntityBase* In = nullptr )
        : Action( "BlitzError", In )
        {}
        
        std::map< uint32_t, uint8_t > Errors;
    };
    
    class TurnStartAction : public Action
    {
    public:
        
        TurnStartAction( EntityBase* In = nullptr )
        : Action( "TurnStart", In )
        {}
        
        PlayerTurn pState;
    };
    
    // Damage
    class DamageAction : public Action
    {
    public:
        
        DamageAction( EntityBase* In )
        : Action( "Damage", In ), StaminaDrain( 0 )
        {}
        
        CardEntity* Target;
        CardEntity* Inflictor;
        uint16_t Damage;
        
        uint16_t StaminaDrain;
    };
    
    // Stamina Drain
    class StaminaDrainAction : public Action
    {
    public:
        
        StaminaDrainAction( EntityBase* In )
        : Action( "StaminaDrain", In )
        {}
        
        CardEntity* Target;
        CardEntity* Inflictor;
        uint16_t Amount;
    };
    
    class WinAction : public Action
    {
    public:
        
        WinAction( EntityBase* In )
        : Action( "Win", In )
        {}
        
        bool bDidWin;
    };
    
    
}
