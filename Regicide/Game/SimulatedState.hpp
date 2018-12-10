//
//	SimulatedState.hpp
//	Regicide Mobile
//
//	Created: 12/3/18
//	Updated: 12/3/18
//
//	© 2018 Zachary Berry, All Rights Reserved
//

#pragma once
#include "LuaContext.hpp"
#include "CardEntity.hpp"
#include "Player.hpp"
#include "GameStateBase.hpp"


namespace Game
{
    class SimulatedPlayer
    {
    public:
        
        PlayerState State;
        
        std::deque< CardState > Deck;
        std::deque< CardState > Hand;
        std::deque< CardState > Field;
        std::deque< CardState > Graveyard;
        
        bool bIsOpponent;
        
        CardState* LookupCard( uint32_t Id );
        
    };
    
    class SimulatedState;
    class SimulatorContext : public LuaContext
    {
        
    public:
        
        virtual bool DrawCard( uint32_t inPlayer, int ActionType );
        virtual bool DealDamage( uint32_t inTarget, int Amount, int ActionType );
        
        // State Query
        virtual bool IsTurn();
        virtual luabridge::LuaRef GetField( uint32_t inPlayer );
        virtual uint32_t GetOpponent();
        virtual uint32_t GetOwner();
        
        inline SimulatedState* GetSimulator() { return Simulator; }
        void SetSimulator( SimulatedState* In ) { Simulator = In; }
        
    protected:
        
        SimulatedState* Simulator;
        
    };
    
    class SimulatorState : public GameStateBase
    {
        
    };
    
    class SimulatedState
    {
        
    public:
        
        SimulatedState();
        
        // Player States
        SimulatedPlayer LocalPlayer;
        SimulatedPlayer Opponent;
        
        // Game State
        MatchState mState;
        TurnState tState;
        PlayerTurn pState;
        
        bool TurnHalf;
        int TurnNumber;
        
        SimulatedPlayer* LookupPlayer( uint32_t Id );
        CardState* LookupCard( uint32_t Id );
        bool KillCard( CardState* inCard );
        SimulatedPlayer* GetActivePlayer();
        SimulatedPlayer* GetInactivePlayer();
        bool PerformDraw( SimulatedPlayer* Target );
        
        void SimulateTurn();
        void SetMarshalHandler( std::function< void() > In ) { SimulateMarshal = In; }
        void SetAttackHandler( std::function< void() > In ) { SimulateAttack = In; }
        void SetBlockHandler( std::function< void() > In ) { SimulateBlock = In; }
        void SetTurnResultHandler( std::function< void() > In ) { OnTurnResults = In; }
        
        bool PlayCard( CardState* inCard );
        bool TriggerAbility( CardState* inCard, uint8_t inAbility );
        bool CanTriggerAbility( CardState* inCard, uint8_t inAbility );
        bool SetAttackers( std::vector< CardState* > Attackers );
        bool SetBlockers( std::map< CardState*, CardState* > Blockers );
        
        void CopyTo( SimulatedState& Other );
        void Sync();
        
    protected:
        
        void FinishBlitz();
        void StartPreTurn();
        void StartMarshal();
        void StartAttack();
        void StartBlock();
        void StartDamage();
        void FinishTurn();
        
        std::function< void() > SimulateMarshal;
        std::function< void() > SimulateAttack;
        std::function< void() > SimulateBlock;
        std::function< void() > OnTurnResults;
        
        std::map< uint32_t, std::vector< uint32_t > > BattleMatrix;
        
        SimulatorContext Context;
        
    };
    
}
