//
//	AIController.cpp
//	Regicide Mobile
//
//	Created: 11/29/18
//	Updated: 11/29/18
//
//	© 2018 Zachary Berry, All Rights Reserved
//

// AI Flow Diagram
// 1. Build list of all possible IMMEDIATE actions (i.e. blitz)
// 2. Create simulation for each possibility, that has the 'deicsion' already made
// 3. Loop through each possibility, and sync the main simulation to the decisions simulation
// 4. Run a full game simulation (until max depth) with semi-random, semi-rule based decisions
// 5. Rate the final game state
// 6. Keep running additional simulations, on the decisions with the best looking outcomes
// 7. Stop simulations at some pre-determined number (~300)
// 8. Average scores for each decisions simulation
// 9. Choose best option

// Even a totally random monte carlo selection algorithm increases the number of wins, the AI would have based
// on ai research based around similar games. These studies also show, that around 300 simulations are the
// optimal amount, and this gives us some good direction. The difference between the research and our implementation
// is, that we plan to use monte carlo for other parts of the game, besides just playing cards
// We are also going to use it for all major AI decisions. Which will hopfully increase the efficacy

// Now, for the implementaition, we need a generic way to run this algorithm, so we dont have to rewrite it for each
// game phase from scratch, which would be a nightmare to maintain and improve upon.
// This means, we need a way to represent these decisions generically, with a good way to call back to the game engine

// If we had to make a class for each seperate decision, they would be as follows
// Play: std::vector< uint32_t > Cards
// Abilities: std::vector< std::pair< uint32_t, uint8_t > > Abilities
// Attack: std::vector< uint32_t > Cards
// Block: std::vector< std::pair< uint32_t, uint32_t > > Combat
// Blitz: std::vector< uint32_t > Cards

// They all have the vector in common, although block could potentially use a map, but this isnt important
// Some use pairs, some use just entity ids. One of the pairs are 8 bit for the second param, the other pair uses 32 bit
// We could potentially represent this as a std::vector< std::pair< uint32_t, uint32_t > >
// This would be able to cover all possible needed values
// Another solution would be to use templates, with a template< typename T > std::vector< T > Decision

// We need to start writing code to figure out the rest of this. but we will use the first solution in the previous paragraph
// We need a decision class

#include "AIController.hpp"
#include "World.hpp"
#include "CardEntity.hpp"
#include "HandEntity.hpp"
#include "FieldEntity.hpp"
#include "SingleplayerAuthority.hpp"
#include "DeckEntity.hpp"
#include "Numeric.hpp"

using namespace Game;

/*
struct MarshalPossibility
{
    std::vector< CardEntity* > Cards;
    int RemainingMana;
};
 */

AIController::AIController()
: EntityBase( "AIController" )
{
    State = AIState::Init;
}

void AIController::Initialize()
{
    EntityBase::Initialize();
    
    Thread = std::make_shared< std::thread >( std::thread( &AIController::StartThink, this ) );
}


void AIController::Cleanup()
{
    EntityBase::Cleanup();
    
    if( Thread )
    {
        State = AIState::Exit;
        
        if( Thread->joinable() )
        {
            Thread->join();
        }
        
        Thread.reset();
    }
    
    cocos2d::log( "[AI] Thread Shutdown!" );
}


void AIController::Post( std::function< void() > Task, std::function< void( float ) > OnComplete /* = nullptr */ )
{
    if( Task )
    {
        Tasks.push( std::make_pair( Task, OnComplete ) );
    }
}


void AIController::StartThink()
{
    cocos2d::log( "[AI] Thread Initializing..." );
    State = AIState::Idle;
    
    std::chrono::steady_clock::time_point NextTick = std::chrono::steady_clock::now() + std::chrono::milliseconds( 50 );
    while( State != AIState::Exit )
    {
        DoThink();
        RunTasks( NextTick );
        
        std::this_thread::sleep_until( NextTick );
        NextTick += std::chrono::milliseconds( 50 );
    }
    
    ExitThink();
}


void AIController::ExitThink()
{
    cocos2d::log( "[AI] Shutting Down Thread..." );
}


void AIController::DoThink()
{
    
}


void AIController::RunTasks( std::chrono::steady_clock::time_point EndBy )
{
    while( State != AIState::Exit && Tasks.size() > 0 && EndBy > std::chrono::steady_clock::now() + std::chrono::milliseconds( 5 ) )
    {
        auto Task = Tasks.front();
        std::function< void() > Func = Task.first;
        std::function< void( float ) > Callback = Task.second;
        
        if( Func )
        {
            auto Start = std::chrono::steady_clock::now();
            Func();
            
            if( Callback )
            {
                std::chrono::duration< float, std::milli > Duration = std::chrono::steady_clock::now() - Start;
                Callback( Duration.count() );
            }
        }
        
        Tasks.pop();
    }
}


void AIController::Push( std::function< void() > Task )
{
    if( Task )
    {
        auto dir = cocos2d::Director::getInstance();
        auto sch = dir ? dir->getScheduler() : nullptr;
        
        if( !sch )
        {
            cocos2d::log( "[AI] Failed to push work back to cocos2d! Couldnt get scheduler!" );
            return;
        }
        
        sch->performFunctionInCocosThread( Task );
    }
}


SingleplayerAuthority* AIController::GetAuthority()
{
    auto Output = dynamic_cast< SingleplayerAuthority* >( GetOwner() );
    CCASSERT( Output, "[AI] Failed to get authority!" );
    
    return Output;
}

bool CheckLists( const std::vector< std::pair< uint32_t, uint32_t > >& First,
                 const std::vector< std::pair< uint32_t, uint32_t > >& Second,
                 bool bMatchOrder )
{
    // Check if the two lists have the same elements, ignoring order
    // Returns true if the same, false if not
    if( First.size() != Second.size() )
        return false;
    
    if( bMatchOrder )
    {
        for( int i = 0; i < First.size(); i++ )
        {
            if( First[ i ].first != Second[ i ].first ||
               First[ i ].second != Second[ i ].second )
            {
                return false;
            }
        }
    }
    else
    {
        for( auto i = First.begin(); i != First.end(); i++ )
        {
            // Check if this element is in the other list
            bool bFound = false;
            for( auto j = Second.begin(); j != Second.end(); j++ )
            {
                if( i->first == j->first && i->second == j->second )
                {
                    bFound = true;
                    break;
                }
            }
            
            // If this element was not found, then the lists are different
            if( !bFound )
                return false;
        }
    }
    
    return true;
}

bool AIController::DecisionExists( const std::vector<std::pair<uint32_t, uint32_t> > &In, bool bMatchOrder )
{
    for( auto It = DecisionList.begin(); It != DecisionList.end(); It++ )
    {
        if( CheckLists( It->Move, In, bMatchOrder ) )
            return true;
    }
    
    return false;
}


void AIController::DoBuildPlay( Decision& Base )
{
    auto& Sim = Base.State;
    auto Player = Sim.GetOpponent();
    CCASSERT( Player, "[AI] Opponent object null!" );
    
    if( Player->Mana <= 0 || Player->Hand.size() == 0 )
        return;
    
    for( auto It = Player->Hand.begin(); It != Player->Hand.end(); It++ )
    {
        // Check if this card can be played
        if( Sim.CanPlayCard( Player, std::addressof( *It ) ) )
        {
            // Check if this combination exists
            std::vector< std::pair< uint32_t, uint32_t > > Cards;
            for( auto j = Base.Move.begin(); j != Base.Move.end(); j++ )
                Cards.push_back( *j );
            
            Cards.push_back( std::make_pair( It->EntId, 0 ) );
            
            if( !DecisionExists( Cards, false ) )
            {
                auto Node = Decision();
                Node.Move = Cards;
                
                // Copy over existing statee
                Node.State.CopyFrom( Base.State );
                
                // Attempt to play new card
                if( !Node.State.PlayCard( Node.State.GetOpponent(), It->EntId ) )
                {
                    cocos2d::log( "[AI] WARNING: Play appeared valid.. but failed upon simulation!" );
                    continue;
                }
                
                Node.Type = MoveType::Play;
                DecisionList.push_back( Node );
                
                DoBuildPlay( Node );
            }
        }
    }
}


void AIController::BuildPlayOptions( MoveType inType )
{
    CC_ASSERT( inType == MoveType::Blitz || inType == MoveType::Play );
    
    DecisionList.clear();
    SimulationCount = 0;
    
    auto Node = Decision();
    Node.Type = inType;
    
    // Sync state to base reality
    auto Auth = GetAuthority();
    Node.State.CopyFrom( Auth->GetState() );

    // This is the base option (no cards played)
    // So were going to add it to the decision list, then start adding play options
    DecisionList.push_back( Node );
    DoBuildPlay( Node );
    
}


void AIController::DoBuildAttack( Decision& Base )
{
    auto& Sim = Base.State;
    auto Player = Sim.GetOpponent();
    CCASSERT( Player, "[AI] Opponent object is null!" );
    
    for( auto It = Player->Field.begin(); It != Player->Field.end(); It++ )
    {
        // If card isnt already attacking, build a node with it
        bool bAttacking = false;
        for( auto j = Sim.BattleMatrix.begin(); j != Sim.BattleMatrix.end(); j++ )
        {
            if( It->EntId == j->first )
            {
                bAttacking = true;
                break;
            }
        }
        
        if( !bAttacking )
        {
            // TODO: Check if this card can actually attack
            std::vector< std::pair< uint32_t, uint32_t > > Cards( Base.Move.begin(), Base.Move.end() );
            Cards.push_back( std::make_pair( It->EntId, 0 ) );
            
            // Check if this combination exists
            if( !DecisionExists( Cards, false ) )
            {
                auto Node = Decision();
                Node.Move = Cards;
                
                // Copy over state
                Node.State.CopyFrom( Sim );
                Node.State.BattleMatrix = Sim.BattleMatrix;
                
                // Add card to battle matrix
                Node.State.BattleMatrix[ It->EntId ] = std::vector< uint32_t >();
                Node.Type = MoveType::Attack;
                
                // Add to list, and continue building
                DecisionList.push_back( Node );
                DoBuildAttack( Node );
            }
        }
    }
}


void AIController::BuildAttackOptions()
{
    DecisionList.clear();
    SimulationCount = 0;
    
    // Add 'no attack' option
    auto Node = Decision();
    Node.Type = MoveType::Attack;
    
    auto Auth = GetAuthority();
    CC_ASSERT( Auth );
    
    Node.State.CopyFrom( Auth->GetState() );
    
    DecisionList.push_back( Node );
    DoBuildAttack( Node );
}

bool CheckOptions( std::map< uint32_t, std::vector< uint32_t > > First, std::map< uint32_t, std::vector< uint32_t > > Second )
{
    // If the number of attakcers are different, then theyre not the same
    if( First.size() != Second.size() )
        return false;
    
    // Loop through each attacker
    for( auto It = First.begin(); It != First.end(); It++ )
    {
        // Check if the other list has this attacker
        if( Second.count( It->first ) == 0 )
        {
            // They dont have the same attackers
            return false;
        }
        else
        {
            // Check if the elements in the vectors are the same
            for( auto j = It->second.begin(); j != It->second.end(); j++ ) // Loop through first vector
            {
                // Check for element in second vector
                bool bFound = false;
                for( auto k = Second[ It->first ].begin(); k != Second[ It->first ].end(); k++ )
                {
                    if( *j == *k )
                    {
                        bFound = true;
                        break;
                    }
                }
                
                // If the element wasnt in the second vector, then the two are different
                if( !bFound )
                    return false;
            }
        }
    }
    
    return true;
}

void AIController::DoBuildBlock( Decision &Base )
{
    auto& Sim = Base.State;
    auto Player = Sim.GetOpponent();

    CCASSERT( Player && Attacker, "[AI] Opponent object is null!" );
    
    int FieldSize = (int) Player->Field.size();
    int AttackerSize = (int) Sim.BattleMatrix.size();
    
    if( AttackerSize == 0 || FieldSize == 0 )
        return;
    
    // Were going to come up with 100 random, non-repeating blocking deicisions
    // In the future, we can improve this by trying to suggest slightly better decisions
    // Basically, using some basic rules to manipulate the initial options available to the AI
    
    // We can create some tuned options.. try and select some 1:1 blocker options
    
    int NeededOptions = 100;
    int CreatedOptions = 0;
    int Repeats = 0;
    
    std::vector< std::map< uint32_t, std::vector< uint32_t > > > Output;
    
    std::vector< uint32_t > AllAttackers;
    for( auto It = Sim.BattleMatrix.begin(); It != Sim.BattleMatrix.end(); It++ )
        AllAttackers.push_back( It->first );
    
    std::vector< uint32_t > AllBlockers;
    for( auto It = Player->Field.begin(); It != Player->Field.end(); It++ )
        AllBlockers.push_back( It->EntId );
    
    while( CreatedOptions < NeededOptions && Repeats < 10 )
    {
        std::map< uint32_t, std::vector< uint32_t > > Option;
        for( auto It = AllAttackers.begin(); It != AllAttackers.end(); It++ )
            Option[ *It ] = std::vector< uint32_t >();
        
        std::vector< uint32_t > AvailableBlockers( AllBlockers.begin(), AllBlockers.end() );
        
        // Create a totally random block/attack matchup
        int BlockerCount = FieldSize > 1 ? cocos2d::random( 1, FieldSize ) : 1;
        
        for( int i = 0; i < BlockerCount; i++ )
        {
            // Pick random attacker to match blocker with
            int AttackerIndex = AllAttackers.size() > 1 ? cocos2d::random( 0, (int)AllAttackers.size() - 1 ) : 0;
            uint32_t Attacker = AllAttackers.at( AttackerIndex );
            
            // Now pick random blocker
            int BlockerIndex = AvailableBlockers.size() > 1 ? cocos2d::random( 0, (int) AvailableBlockers.size() - 1 ) : 0;
            auto It = AvailableBlockers.begin();
            std::advance( It, BlockerIndex );
            
            uint32_t Blocker = *It;
            AvailableBlockers.erase( It );
            
            Option[ Attacker ].push_back( Blocker );
        }
        
        // Check if this option already exists
        bool bExists = false;
        for( auto It = Output.begin(); It != Output.end(); It++ )
        {
            if( CheckOptions( *It, Option ) )
            {
                bExists = true;
                break;
            }
        }
        
        // If it doesnt exist, then add to output
        if( !bExists )
        {
            cocos2d::log( "[DEBUG] GENERATED NEW RANDOM CHOICE" );
            Output.push_back( Option );
            CreatedOptions++;
        }
        else
        {
            Repeats++;
            cocos2d::log( "[DEBUG] REPEAT OPTION" );
        }
    }
    
    // Now we built a list of completley random blocking actions, we need to validate these options
    for( auto It = Output.begin(); It != Output.end(); It++ )
    {
        auto Node = Decision();
        Node.Type = MoveType::Block;
        Node.State.CopyFrom( Base.State );
        Node.State.BattleMatrix = Base.State.BattleMatrix;
        
        for( auto j = It->begin(); j != It->end(); j++ )
        {
            for( auto i = j->second.begin(); i != j->second.end(); i++ )
            {
                Node.State.BattleMatrix[ j->first ].push_back( *i );
                Node.Move.push_back( std::make_pair( *i, j->first ) );
            }
        }
        
        DecisionList.push_back( Node );
    }
}


void AIController::BuildBlockOptions()
{
    DecisionList.clear();
    SimulationCount = 0;
    
    auto Node = Decision();
    Node.Type = MoveType::Block;
    
    auto Auth = GetAuthority();
    CC_ASSERT( Auth );
    
    Node.State.CopyFrom( Auth->GetState() );
    Node.State.BattleMatrix = Auth->BattleMatrix;
    
    DecisionList.push_back( Node );
    DoBuildBlock( Node );
}


void AIController::SimulateAll()
{
    // Target 1000 simulated turns total
    // Clamped between 8 and 17 per decision
    int SimulatedTurns = Math::Clamp( 1000 / (int) DecisionList.size(), 8, 17 );
    //int SimulatedTurns = Math::Clamp( (int)( 64.0 / sqrt( (double) DecisionList.size() ) ), 5, 15 );
    cocos2d::log( "[AI] Starting initial simulation round.. simulating %d turns", SimulatedTurns );
    
    for( auto It = DecisionList.begin(); It != DecisionList.end(); It++ )
    {
        //cocos2d::log( "[DEBUG] Starting a simulation..." );
        // Were going to simulate each on a seperate thread
        Simulate( *It, SimulatedTurns );
    }
}

void AIController::CalculateReward( Decision& Target )
{
    // Some factors for calculating the reward
    // How mnay turns were simulated
    // If either player won
    // How many cards are left in deck
    // How many cards are on field and their power
    // How many cards are in hand
    // Mana
    // Health
    int SimulatedTurns = Simulation.GetSimulatedTurns();
    auto Winner = Simulation.GetWinner();
    
    auto& LocalPlayer = Simulation.LocalPlayer;
    auto& AIPlayer = Simulation.Opponent;
    
    int PlayerDeckSize = (int) LocalPlayer.Deck.size();
    int AIDeckSize = (int) AIPlayer.Deck.size();
    
    int PlayerFieldSize = (int) LocalPlayer.Field.size();
    int AIFieldSize = (int) AIPlayer.Field.size();
    
    int PlayerFieldPower = 0;
    for( auto It = LocalPlayer.Field.begin(); It != LocalPlayer.Field.end(); It++ )
        PlayerFieldPower += It->Power;
    
    int AIFieldPower = 0;
    for( auto It = AIPlayer.Field.begin(); It != AIPlayer.Field.end(); It++ )
        AIFieldPower += It->Power;
    
    int PlayerHealth = LocalPlayer.Health;
    int AIHealth = AIPlayer.Health;
    
    int PlayerMana = LocalPlayer.Mana;
    int AIMana = AIPlayer.Mana;

    // Now that we have all the variables needed to calculate reward, lets create a formula
    // First, we should look at who won, because if the player or opponent won in one or this turn,
    // then the rating should be a full 1 or 0
    float WinRating = 0.f;
    
    if( Winner )
    {
        if( Winner == std::addressof( LocalPlayer ) )
            WinRating = -1.f;
        else if( Winner == std::addressof( AIPlayer ) )
            WinRating = 1.f;
        
        WinRating *= Math::Clamp( (float)( 16 - SimulatedTurns ) / 15.f, 0.f, 1.f );
    }
    
    // Now lets calculate a -1:1 rating for the field state
    // Were going to make this rating somewhat relative to the other player
    // for instance, if there is 10 cards on field, and theyre all the AI's cards
    // that would score out to a 1.0
    // On the other hand, if theres 20 cards on field, and each player has 10, it would be a 0.0
    // But, if theres less than 4 cards on field, this rating will be scaled down
    int TotalFieldCount = AIFieldSize + PlayerFieldSize;
    float FieldCountRating = Math::SDiv< float >( AIFieldSize - PlayerFieldSize, TotalFieldCount );
    
    if( TotalFieldCount < 4 )
    {
        FieldCountRating *= ( TotalFieldCount / 5.f );
    }
    
    int TotalFieldPower = PlayerFieldPower + AIFieldPower;
    float FieldPowerRating = Math::SDiv< float >( AIFieldPower - PlayerFieldPower, TotalFieldPower );
    
    if( TotalFieldPower < 6 )
    {
        FieldPowerRating *= ( TotalFieldPower / 6.f );
    }
    
    int TotalDeckCount = PlayerDeckSize + AIDeckSize;
    float DeckCountRating = Math::SDiv< float >( AIDeckSize - PlayerDeckSize, TotalDeckCount );
    
    // For the health, were going to rate the state relative to starting health
    float HealthRating = Math::Clamp( ( AIHealth - PlayerHealth ) / 20.f, -1.f, 1.f );
    
    int TotalMana = PlayerMana + AIMana;
    float ManaRating = Math::SDiv< float >( AIMana - PlayerMana, TotalMana );
    if( TotalMana < 5 )
    {
        ManaRating *= ( (float) TotalMana / 5.f );
    }
    
    
    // Now that we calculated the total scores of all aspects of the state, we need to average the results
    // Some aspects are going to be weighed heavier than others, for instance, if the 'Win' rating is high
    // were going to ignore the other scores, since thats by far the most important
    // First off, were going to add up all 'non-win' scores
    float TotalNonWin = FieldCountRating * 1.f + FieldPowerRating * 1.7f + HealthRating * 2.7f + ManaRating * 0.65f;
    
    // Check if deck count rating is 'important'
    bool bDeckImportant = Math::Abs( DeckCountRating ) > 0.8f;
    TotalNonWin += bDeckImportant ? DeckCountRating * 1.75f : DeckCountRating * 0.85f;
    
    // Determine the total possible range, and scale back to -1 to 1
    float TotalRange = 1.f + 1.7f + 2.7f + 0.65f;
    if( bDeckImportant )
        TotalRange += 1.75f;
    else
        TotalRange += 0.85f;
    
    
    // Weigh in Win rating and Non win ratings, on a scale from -1 to 1
    float Rating = ( Math::Clamp( TotalNonWin / TotalRange, -1.f, 1.f ) + WinRating * 2.f ) / 3.f;
    
    // Convert from -1:1 range to 0:1
    float FinalRating = ( Rating / 2.f ) + 0.5f;
    
    /*
    cocos2d::log( "[DEBUG] Fields( AI Count: %d  Op Count: %d  AI Power: %d  Op Power: %d )", AIFieldSize, PlayerFieldSize, AIFieldPower, PlayerFieldPower );
    cocos2d::log( "[DEBUG] Field Count: %f  Field Power: %f", FieldCountRating, FieldPowerRating );
    cocos2d::log( "[DEBUG] AI Health: %d  Op Health: %d  AI Mana: %d  Op Mana: %d", AIHealth, PlayerHealth, AIMana, PlayerMana );
    cocos2d::log( "[DEBUG] Health: %f  Mana: %f", HealthRating, ManaRating );
    cocos2d::log( "[DEBUG] AI Deck: %d  Op Deck: %d  Rating: %f", AIDeckSize, PlayerDeckSize, DeckCountRating );
    cocos2d::log( "[Sim] Rated simulation results at %f", ScaledRating );
     */
    Target.Scores.push_back( FinalRating );
}

void AIController::Simulate( Decision& Target, int Turns )
{
    // Copy decision state into simulator
    Simulation.CopyFrom( Target.State );
    Simulation.BattleMatrix = Target.State.BattleMatrix;
    
    // Run simulation on this target
    // We need a way to 'rate' each simulation, on how prefferable it is
    // The range for this rating is [0:1]
    Simulation.PrepareSimulation();
    
    // Since we implemented the base decision by this point, we need to ensure the round state gets advanced
    // then, the simulation will continue from where we left off automatically, until it hits the desired depth
    // or either player wins the game
    if( Target.Type == MoveType::Blitz )
    {
        // Simulate the other players blitz before starting the simulation
        Simulation.SimulatePlayerBlitz();
    }
    
    // If were in the play card phase, we need to simulate the ability phase before running the simulation
    if( Target.Type == MoveType::Play )
    {
        // TODO: Simulate Abilities
    }
    
    // Calculate the number of turns to simulate
    // The more possible decisions, the less turns we will simulate
    // Were going to clamp the turns to simulate between 5 and 20
    Simulation.RunSimulation( Turns );
    
    // Now we need to calculate the 'rating' of the resulting game state
    CalculateReward( Target );
}

Decision* AIController::GetOptionToSimulate()
{
    if( DecisionList.empty() )
        return nullptr;
    
    // Find the option that maximizes formula
    float BestScore = -100.f;
    Decision* BestDecision = nullptr;
    float C = 0.2f;
    float lnCount = 2 * log( SimulationCount );
    
    for( auto It = DecisionList.begin(); It != DecisionList.end(); It++ )
    {
        // To calculate the reward from each decision we use the following formula
        // float num = ln( total_plays ) / decision_plays;
        // float root = C * sqrt( num );
        // float result = avg_reward + root;
        // Where C = constant value between 0 and 1, where 1 is a uniform search, and 0 is a selective search
        float AvgResult = 0.f;
        for( auto j = It->Scores.begin(); j != It->Scores.end(); j++ )
            AvgResult += *j;
        
        AvgResult /= (float) It->Scores.size();
        
        int SimCount = (int) It->Scores.size();
        if( SimCount == 0 )
            SimCount = 1;
        
        float Result = AvgResult + ( C * sqrt( lnCount / SimCount ) );
        if( Result > BestScore )
        {
            BestScore = Result;
            BestDecision = std::addressof( *It );
        }
    }
    
    return BestDecision;
}

void AIController::FirstRunComplete()
{
    // Now that the first run of simulations are complete, we need to start running a
    // limited number of additional simulations to determine the best decision
    
    // Were going to run
    int SimulatedTurns = 8;
    //int SimulatedTurns = Math::Clamp( (int)( 64.0 / sqrt( (double) DecisionList.size() ) ), 5, 15 );
    cocos2d::log( "[AI] Initial simulation round complete.. running %d turns", SimulatedTurns );
    
    // Now we need to loop through and continue simulating the best options
    for( SimulationCount = 1; SimulationCount <= 300; SimulationCount++ )
    {
        // Pick best option to simulate
        auto Target = GetOptionToSimulate();
        if( !Target )
        {
            cocos2d::log( "[AI] Couldnt find best option to simulate!" );
            continue;
        }
        
        // Run simulation
        Simulate( *Target, SimulatedTurns );
    }
    
    SimulationCount--;
}

Decision* AIController::GetMostSimulated()
{
    Decision* Output = nullptr;
    float HighestScore = 0.f;
    /*
    for( auto It = DecisionList.begin(); It != DecisionList.end(); It++ )
    {
        if( !Output || It->Scores.size() > Output->Scores.size() )
            Output = std::addressof( *It );
    }
    */
    for( auto It = DecisionList.begin(); It != DecisionList.end(); It++ )
    {
        if( !Output )
        {
            Output = std::addressof( *It );
        }
        else
        {
            float Score = 0.f;
            for( auto j = It->Scores.begin(); j != It->Scores.end(); j++ )
                Score += *j;
            Score /= (float) It->Scores.size();
            
            if( Score > HighestScore )
            {
                HighestScore = Score;
                Output = std::addressof( *It );
            }
        }
    }
    return Output;
}


void AIController::Clear()
{
    cocos2d::log( "[AI] Clearing Data..." );
    DecisionList.clear();
    SimulationCount = 0;
    
    State = AIState::Idle;
}


void AIController::ChooseBlitz()
{
    State = AIState::Blitz;
    cocos2d::log( "[AI] Making Blitz Decision..." );
    
    // Build a new decision list, basically identical to PlayCards
    Post(
    [ = ]()
    {
        BuildPlayOptions( MoveType::Blitz );
        
        cocos2d::log( "[AI] There are %d blitz options", (int) DecisionList.size() );
        SimulateAll();
        FirstRunComplete();
        
        auto Auth = GetAuthority();
        CC_ASSERT( Auth );
        
        // Now we need to find the most simulated option
        auto Best = GetMostSimulated();
        if( !Best )
        {
            cocos2d::log( "[AI] Failed to find good blitz decision.. passing!" );
            Auth->AI_SetBlitz( std::vector< uint32_t >() );
        }
        else
        {
            float AvgScore = 0.f;
            for( auto It = Best->Scores.begin(); It != Best->Scores.end(); It++ )
                AvgScore += *It;
            
            float FinalScore = AvgScore / (float) Best->Scores.size();
            
            cocos2d::log( "[AI] Made blitz decision.. Score: %f  Total Simulations: %d  Decision Simulations: %d", FinalScore, SimulationCount, (int) Best->Scores.size() );
            
            // Inform authority that we finished deciding on what to play
            std::vector< uint32_t > TargetCards;
            for( auto It = Best->Move.begin(); It != Best->Move.end(); It++ )
                TargetCards.push_back( It->first );
            
            Auth->AI_SetBlitz( TargetCards );
            
        }
        
        // Now we can clear out all of the members we accumulated while deciding
        Clear();
        
    } );
    
}


void AIController::PlayCards()
{
    State = AIState::Marshal;
    cocos2d::log( "[AI] Making Play Decision..." );
    
    // We need to first build a list of all possible play actions
    Post(
         [ = ]()
         {
             BuildPlayOptions( MoveType::Play );
             
             cocos2d::log( "[AI] There are %d play options", (int) DecisionList.size() );
             SimulateAll();
             FirstRunComplete();
             
             auto Auth = GetAuthority();
             CC_ASSERT( Auth );
             
             // Now we need to find the most simulated option
             auto Best = GetMostSimulated();
             if( !Best )
             {
                 cocos2d::log( "[AI] Failed to find good play decision.. passing" );
                 Push( [=]() { Auth->AI_FinishPlay(); } );
             }
             else
             {
                 float AvgScore = 0.f;
                 for( auto It = Best->Scores.begin(); It != Best->Scores.end(); It++ )
                     AvgScore += *It;
                 AvgScore /= (float) Best->Scores.size();
                 
                 cocos2d::log( "[AI] Made play decision.. Score: %f  Total Simulations: %d  Decision Simulations: %d", AvgScore, SimulationCount, (int) Best->Scores.size() );
                 
                 std::vector< uint32_t > Cards;
                 for( auto It = Best->Move.begin(); It != Best->Move.end(); It++ )
                     Cards.push_back( It->first );
                 
                 Push( [=]() { Auth->AI_PlayCards( Cards ); } );
             }
             
             Clear();
         } );
}


void AIController::TriggerAbilities()
{
    State = AIState::Ability;
    cocos2d::log( "[AI] Making Ability Decision..." );
    
    // TODO
}


void AIController::ChooseAttackers()
{
    State = AIState::Attack;
    cocos2d::log( "[AI] Making Attack Decision..." );
    
    Post(
         [ = ]()
         {
             BuildAttackOptions();
             
             cocos2d::log( "[AI] There are %d attack options", (int) DecisionList.size() );
             SimulateAll();
             FirstRunComplete();
             
             auto Auth = GetAuthority();
             CC_ASSERT( Auth );
             
             // Now we need to find the most simulated option
             auto Best = GetMostSimulated();
             if( !Best )
             {
                 cocos2d::log( "[AI] Failed to find good attack decision.. passing" );
                 Push( [=]() { Auth->AI_SetAttackers( std::vector< uint32_t >() ); } );
             }
             else
             {
                 float AvgScore = 0.f;
                 for( auto It = Best->Scores.begin(); It != Best->Scores.end(); It++ )
                     AvgScore += *It;
                 AvgScore /= (float) Best->Scores.size();
                 
                 cocos2d::log( "[AI] Made attack decision.. Score: %f  Total Simulations: %d  Decision Simulations: %d", AvgScore, SimulationCount, (int) Best->Scores.size() );
                 
                 std::vector< uint32_t > Cards;
                 for( auto It = Best->Move.begin(); It != Best->Move.end(); It++ )
                     Cards.push_back( It->first );
                 
                 Push( [=]() { Auth->AI_SetAttackers( Cards ); } );
             }
             
             Clear();
         } );
}


void AIController::ChooseBlockers( std::vector< uint32_t > Attackers )
{
    State = AIState::Block;
    cocos2d::log( "[AI] Making Block Decision..." );
    
    Post(
         [ = ]()
         {
             BuildBlockOptions();
             
             cocos2d::log( "[AI] There are %d block options", (int) DecisionList.size() );
             SimulateAll();
             FirstRunComplete();
             
             auto Auth = GetAuthority();
             CC_ASSERT( Auth );
             
             // Now we need to find the most simulated option
             auto Best = GetMostSimulated();
             if( !Best )
             {
                 cocos2d::log( "[AI] Failed to find good block decision.. passing" );
                 Push( [=]() { Auth->AI_SetBlockers( std::map< uint32_t, uint32_t >() ); } );
             }
             else
             {
                 float AvgScore = 0.f;
                 for( auto It = Best->Scores.begin(); It != Best->Scores.end(); It++ )
                     AvgScore += *It;
                 AvgScore /= (float) Best->Scores.size();
                 
                 cocos2d::log( "[AI] Made block decision.. Score: %f  Total Simulations: %d  Decision Simulations: %d", AvgScore, SimulationCount, (int) Best->Scores.size() );
                 
                 std::map< uint32_t, uint32_t > Cards;
                 for( auto It = Best->Move.begin(); It != Best->Move.end(); It++ )
                 {
                     cocos2d::log( "=== BLOCKER ===> %d (%d)", It->first, It->second );
                     Cards[ It->first ] = It->second;
                 }
                 
                 cocos2d::log( "[DEBUG] THERE WERE %d BLOCKERS SELECTED", (int)Cards.size() );

                 Push( [=]() { Auth->AI_SetBlockers( Cards ); } );
             }
             
             Clear();
         } );
    
}
