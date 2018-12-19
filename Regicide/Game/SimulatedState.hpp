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
#include "CardEntity.hpp"
#include "Player.hpp"
#include "GameStateBase.hpp"


namespace Game
{
    class SimulatedState : public GameStateBase
    {
        
    public:
        
        SimulatedState();
        
        bool CanPlayCard( PlayerState* Owner, CardState* Card );
        void PrepareSimulation();
        void SimulatePlayerBlitz();
        void FinishBlitz();
        void RunSimulation( int MaxTurns );
        void OnSimulationFinished( PlayerState* Winner = nullptr );
        
        void PreTurn( PlayerTurn InState );
        void Marshal();
        void Ability();
        void Attack();
        void Block();
        void Damage();
        void PostTurn();
        
        inline PlayerState* GetWinner() { return WinningPlayer; }
        inline int GetSimulatedTurns() const { return TurnNumber - SimulationStart; }
        
    protected:
        
        PlayerState& GetActivePlayer();
        PlayerState& GetInactivePlayer();
        int FinalTurn;
        int SimulationStart;
        
        PlayerState* WinningPlayer;
        
        std::map< uint32_t, std::vector< uint32_t > > BattleMatrix;
        
        friend class AIController;
    };
    
}
