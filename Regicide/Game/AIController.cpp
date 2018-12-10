//
//	AIController.cpp
//	Regicide Mobile
//
//	Created: 11/29/18
//	Updated: 11/29/18
//
//	© 2018 Zachary Berry, All Rights Reserved
//

#include "AIController.hpp"
#include "World.hpp"
#include "CardEntity.hpp"
#include "HandEntity.hpp"
#include "FieldEntity.hpp"
#include "SingleplayerAuthority.hpp"
#include "DeckEntity.hpp"

using namespace Game;

struct MarshalPossibility
{
    std::vector< CardEntity* > Cards;
    int RemainingMana;
};

AIController::AIController()
: EntityBase( "AIController" )
{
    State = AIState::Init;
    
    cocos2d::log( "[AI] CONSTRUCTOR" );
}

void AIController::Initialize()
{
    EntityBase::Initialize();
    
    // Start AI thread
    ThinkThread = std::make_shared< std::thread >( std::thread( &AIController::ThinkEntry, this ) );
}

void AIController::Cleanup()
{
    EntityBase::Cleanup();
    
    if( ThinkThread )
    {
        State = AIState::Exit;
        
        if( ThinkThread->joinable() )
        {
            ThinkThread->join();
        }
        
        ThinkThread.reset();
    }
    
    cocos2d::log( "[AI] Shutdown complete" );
}

void AIController::ThinkEntry()
{
    // Load Needed Info
    cocos2d::log( "[AI] Initializing..." );
    std::this_thread::sleep_for( std::chrono::seconds( 2 ) );
    cocos2d::log( "[AI] Initialized!" );
    
    State = AIState::Idle;
    
    // Start Logic Loop
    std::chrono::steady_clock::time_point NextTick = std::chrono::steady_clock::now() + std::chrono::milliseconds( 50 );
    while( State != AIState::Exit )
    {
        Think();
        RunTasks();
        
        // Wait for next tick
        std::this_thread::sleep_until( NextTick );
        NextTick = std::chrono::steady_clock::now() + std::chrono::milliseconds( 50 );
    }
    
    ThinkExit();
}

void AIController::Think()
{
    // Do stuff..
}

void AIController::RunTasks()
{
    while( Tasks.size() > 0 )
    {
        auto NewTask = Tasks.front();
        if( NewTask )
        {
            NewTask();
        }
        
        Tasks.pop();
    }
}

void AIController::ThinkExit()
{
    cocos2d::log( "[AI] Starting shutdown..." );
    std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
}

void AIController::Post( std::function< void() > inWork )
{
    if( inWork )
    {
        Tasks.push( inWork );
    }
}

void AIController::Push( std::function<void ()> inFunc )
{
    if( inFunc )
    {
        auto Dir = cocos2d::Director::getInstance();
        if( !Dir )
        {
            cocos2d::log( "[AI] Failed to push back function.. director is null" );
            return;
        }
        
        auto Sch = Dir->getScheduler();
        if( !Sch )
        {
            cocos2d::log( "[AI] Failed to push back function.. scheduler is null" );
            return;
        }
        
        Sch->performFunctionInCocosThread( inFunc );
    }
}


void AIController::OnMarshal()
{
    // Sync with game state
    Simulation.Sync();
    cocos2d::log( "[AI] Copied Active Game State" );
    
    // Build a list of all possible 'plays' that are available using the card list we created
    std::vector< DecisionNode > Nodes;
    BuildNodeList( Nodes );
    
    cocos2d::log( "[DEBUG] %d POSSIBLE PLAY CARD POSSIBILITIES", (int) Nodes.size() );
    for( auto It = Nodes.begin(); It != Nodes.end(); It++ )
    {
        std::string Output = "[DEBUG ACTION] ROOT ";
        for( auto i = It->PlayCards.begin(); i != It->PlayCards.end(); i++ )
        {
            Output += " => " + std::to_string( *i );
        }
        
        cocos2d::log( "%s", Output.c_str() );
        cocos2d::log( "%d", (int) It->Abilities.size() );
    }
    
    cocos2d::log( "[DEBUG] There are %d options", (int) Nodes.size() );
    
    // Rate States
    
}

void AIController::OnAttack()
{
    // We already decided what to do, we just need to let the authority know
}

void AIController::OnBlock()
{
    // We need to build a whole new simulation
}


void AIController::StartMarshal()
{
    // Update State
    State = AIState::Marshal;
    
    // Run marshal function
   // In this function, we will determine the best option for the entire turn
    
    Post( std::bind( &AIController::OnMarshal, this ) );
}

void AIController::StartAttack()
{
    State = AIState::Attack;
    Post( std::bind( &AIController::OnAttack, this ) );
}

void AIController::StartBlock()
{
    State = AIState::Block;
    Post( std::bind( &AIController::OnBlock, this ) );
}


bool AIController::ShouldPlayCard( CardState& In, SimulatedState& State )
{
    return true;
}

bool AIController::ShouldTriggerAbility( Ability &In, SimulatedState& State )
{
    return true;
}


bool CheckForMatch( std::vector< uint32_t >& First, std::vector< uint32_t >& Second )
{
    if( First.size() != Second.size() )
        return false;
    
    for( auto i = First.begin(); i != First.end(); i++ )
    {
        bool bFound = false;
        for( auto j = Second.begin(); j != Second.end(); j++ )
        {
            if( (*i) == (*j) )
            {
                bFound = true;
                break;
            }
        }
        
        if( !bFound )
            return false;
    }
    
    return true;
}

bool CheckForMatch( std::vector< std::pair< uint32_t, uint8_t > >& First, std::vector< std::pair< uint32_t, uint8_t > >& Second )
{
    if( First.size() != Second.size() )
        return false;
    
    for( auto i = First.begin(); i != First.end(); i++ )
    {
        bool bFound = false;
        for( auto j = Second.begin(); j != Second.end(); j++ )
        {
            if( i->first == j->first && i->second == j->second )
            {
                bFound = true;
                break;
            }
        }
        
        if( !bFound )
            return false;
    }
    
    return true;
}

void AIController::BuildAbilityOptions( std::vector<DecisionNode> &Out, DecisionNode &ThisNode )
{
    // Check for any abilities we can trigger
    auto& Player    = ThisNode.State.Opponent;
    auto& CM        = CardManager::GetInstance();
    
    for( auto It = Player.Field.begin(); It != Player.Field.end(); It++ )
    {
        CardInfo Info;
        if( CM.GetInfo( It->Id, Info ) )
        {
            for( auto i = Info.Abilities.begin(); i != Info.Abilities.end(); i++ )
            {
                // Determine if this ability can be triggered
                if( ThisNode.State.CanTriggerAbility( std::addressof( *It ), i->second.Index ) )
                {
                    // Create new decision node
                    auto Node = DecisionNode();
                    
                    // Copy current node into new node
                    for( auto j = ThisNode.PlayCards.begin(); j != ThisNode.PlayCards.end(); j++ )
                        Node.PlayCards.push_back( *j );
                    
                    for( auto j = ThisNode.Abilities.begin(); j != ThisNode.Abilities.end(); j++ )
                        Node.Abilities.push_back( *j );
                    
                    // Add new ability
                    Node.Abilities.push_back( std::make_pair( It->EntId, i->second.Index ) );
                    
                    // Check if this ability combination already exists
                    bool bExists = false;
                    for( auto j = Out.begin(); j != Out.end(); j++ )
                    {
                        if( CheckForMatch( j->PlayCards, Node.PlayCards ) &&
                           CheckForMatch( j->Abilities, Node.Abilities ) )
                        {
                            bExists = true;
                            break;
                        }
                    }
                    
                    if( !ResetSimulation( Node ) )
                    {
                        cocos2d::log( "[AI] Failed to simulate ability when building ability options!" );
                        return;
                    }
                    else
                    {
                        // Add to list
                        if( !bExists && IsGoodOption( Node ) )
                            Out.push_back( Node );
                        
                        if( ShouldExitLoop( Node ) )
                            break;
                        
                        BuildAbilityOptions( Out, Node );
                    }
                }
            }
        }
        else
        {
            cocos2d::log( "[AI] Failed to get card info to check for ability possibilities" );
        }
    }
}

bool AIController::ResetSimulation( DecisionNode& In )
{
    // Sync with current game state
    In.State.Sync();
    
    // Ensure Match/Turn state is where we need it
    In.State.mState = MatchState::Main;
    In.State.tState = TurnState::Marshal;
    In.State.pState = PlayerTurn::Opponent;
    
    // Mark node as dirty
    In.Valid = false;
    
    // Play all cards
    for( auto It = In.PlayCards.begin(); It != In.PlayCards.end(); It++ )
    {
        // Find Card
        auto Card = In.State.LookupCard( *It );
        if( !Card )
        {
            cocos2d::log( "[AI] Failed to setup simulation.. card in play list wasnt found!" );
            return false;
        }
        
        // Play Card
        if( !In.State.PlayCard( Card ) )
        {
            cocos2d::log( "[AI] Failed to setup simulation.. card couldnt be played!" );
            return false;
        }
    }
    
    // Trigger all abilities
    for( auto It = In.Abilities.begin(); It != In.Abilities.end(); It++ )
    {
        // Find Card
        auto Card = In.State.LookupCard( It->first );
        if( !Card )
        {
            cocos2d::log( "[AI] Failed to setup simulation.. card in ability list wasnt found" );
            return false;
        }
        
        // Trigger ability
        if( !In.State.TriggerAbility( Card, It->second ) )
        {
            cocos2d::log( "[AI] Failed to setup simulation.. card ability couldnt be triggered!" );
            return false;
        }
    }
    
    // Mark as clean
    In.Valid = true;
    return true;
}

bool AIController::ShouldExitLoop( DecisionNode& In )
{
    if( In.State.TurnNumber == 1 )
    {
        if( In.Abilities.size() > 2 )
            return true;
        
        if( In.State.Opponent.State.Mana < 3 )
            return true;
    }
    
    return false;
}

bool AIController::IsGoodOption( DecisionNode& In )
{
    // Prefer spending mana on turn 1 and when theres more than 4 cards in hand
    if( In.State.TurnNumber == 1 || In.State.Opponent.Hand.size() > 4 )
    {
        if( In.State.Opponent.State.Mana > 4 )
            return false;
    }
    
    return true;
}

void AIController::BuildPlayOptions( std::vector< DecisionNode >& Out, DecisionNode& ThisNode )
{
    auto& Player = ThisNode.State.Opponent;
    if( Player.State.Mana <= 0 || Player.Hand.size() == 0 )
        return;
    
    for( auto It = Player.Hand.begin(); It != Player.Hand.end(); It++ )
    {
        // Check if theres enough mana for this card
        if( It->ManaCost > Player.State.Mana )
            continue;
        
        // Check if this card should be played
        if( !ShouldPlayCard( *It, ThisNode.State ) )
            continue;
        
        // Create new node
        auto Node   = DecisionNode();
        Node.Valid  = false;
        
        for( auto i = ThisNode.PlayCards.begin(); i != ThisNode.PlayCards.end(); i++ )
            Node.PlayCards.push_back( *i );;
        
        // Add this card to node
        Node.PlayCards.push_back( It->EntId );
        
        // Run simulation
        if( !ResetSimulation( Node ) )
        {
            cocos2d::log( "[AI] Failed to run simulation when building play card list" );
            continue;
        }
        
        // Check if this list already exists, without regard to order
        bool bExists = false;
        for( auto i = Out.begin(); i != Out.end(); i++ )
        {
            if( CheckForMatch( i->PlayCards, Node.PlayCards ) )
            {
                bExists = true;
                break;
            }
        }
        
        // If it doesnt exist, then start building ability possibilities and add to list
        if( !bExists )
        {
            if( IsGoodOption( Node ))
            {
                // Add this as option
                Out.push_back( Node );
                
                // Start building ability options
                BuildAbilityOptions( Out, Node );
            }
        }
        
        if( ShouldExitLoop( Node ) )
            continue;
        
        BuildPlayOptions( Out, Node );
    }
}


void AIController::BuildNodeList( std::vector< DecisionNode > &Out )
{
    // Create 'Pass' Option
    auto Node = DecisionNode();
    Node.State.Sync();
    Node.Valid = true;
    
    Out.push_back( Node );
    
    // 'No Cards' with abilities
    BuildAbilityOptions( Out, Node );
    
    // Start Building Nodes Recursivley
    BuildPlayOptions( Out, Node );
}

void AIController::SimulateMarshal()
{

    
}

void AIController::SimulateAttack()
{
    
}

void AIController::SimulateBlock()
{
    
}

void AIController::OnSimulatedTurnComplete()
{
    
}
