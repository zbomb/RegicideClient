//
//	AIController.hpp
//	Regicide Mobile
//
//	Created: 11/29/18
//	Updated: 11/29/18
//
//	© 2018 Zachary Berry, All Rights Reserved
//

#pragma once

#include "EntityBase.hpp"
#include <thread>
#include <memory>
#include "AppDelegate.hpp"
#include "Player.hpp"
#include "SimulatedState.hpp"


// Note: Most of this is currently DEPRECATED and requires a rewrite!

namespace Game
{
    /*
    enum AIState
    {
        Init,
        Idle,
        Marshal,
        Attack,
        Block,
        Exit
    };
    
    struct DecisionNode
    {
        std::vector< uint32_t > PlayCards;
        std::vector< std::pair< uint32_t, uint8_t > > Abilities;
        SimulatedState State;
        std::vector< float > Scores;
        bool Valid;
    };
     */

    class AIController : public EntityBase
    {
    public:
        
        AIController();
        
        /*
        virtual void Initialize() override;
        virtual void Cleanup() override;
        
        void StartMarshal();
        void StartAttack();
        void StartBlock();
        
        void Post( std::function< void() > inWork );
        inline AIDifficulty GetDifficulty() const { return Difficulty; }
        inline Player* GetTarget() { return Target; }
        
    protected:
        
        AIState State;
        AIDifficulty Difficulty;
        
        SimulatedState Simulation;
        Player* Target;
        
        std::shared_ptr< std::thread > ThinkThread;
        std::queue< std::function< void() > > Tasks;
        
        void ThinkEntry();
        void Think();
        void RunTasks();
        void ThinkExit();
        
        void OnMarshal();
        void OnAttack();
        void OnBlock();
        
        void Push( std::function< void() > inFunc );
        
        // Simulation
        void BuildAbilityOptions( std::vector< DecisionNode >& Out, DecisionNode& ThisNode );
        bool ResetSimulation( DecisionNode& In );
        void BuildPlayOptions( std::vector< DecisionNode >& Out, DecisionNode& ThisNode );
        void BuildNodeList( std::vector< DecisionNode >& Out );
        
        bool ShouldExitLoop( DecisionNode& In );
        bool IsGoodOption( DecisionNode& In );
        bool ShouldPlayCard( CardState& In, SimulatedState& State );
        bool ShouldTriggerAbility( Ability& In, SimulatedState& State );
        void SimulateMarshal();
        void SimulateAttack();
        void SimulateBlock();
        void OnSimulatedTurnComplete();
        
        friend class SingleplayerLauncher;
         */
    };

}
