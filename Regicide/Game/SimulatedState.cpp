//
//	SimulatedState.cpp
//	Regicide Mobile
//
//	Created: 12/3/18
//	Updated: 12/3/18
//
//	© 2018 Zachary Berry, All Rights Reserved
//

#include "SimulatedState.hpp"
#include "HandEntity.hpp"
#include "FieldEntity.hpp"
#include "GraveyardEntity.hpp"
#include "DeckEntity.hpp"
#include "World.hpp"
#include "SingleplayerAuthority.hpp"

using namespace Game;

SimulatedState::SimulatedState()
{
    FinalTurn           = 0;
    SimulationStart     = 0;
    WinningPlayer       = nullptr;
}

PlayerState& SimulatedState::GetActivePlayer()
{
    if( pState == PlayerTurn::LocalPlayer )
        return LocalPlayer;
    else
        return Opponent;
}

PlayerState& SimulatedState::GetInactivePlayer()
{
    if( pState == PlayerTurn::LocalPlayer )
        return Opponent;
    else
        return LocalPlayer;
}

bool SimulatedState::CanPlayCard( PlayerState *Owner, CardState *Card )
{
    if( !Owner || !Card )
        return false;
    
    // Check match, turn state and player turn
    if( mState == MatchState::Blitz || ( mState == MatchState::Main && tState == TurnState::Marshal && IsPlayerTurn( Owner->EntId ) ) )
    {
        // Next, we need to check mana
        if( Owner->Mana >= Card->ManaCost && Card->Owner == Owner->EntId )
        {
            return true;
        }
    }
    
    return false;
}

void SimulatedState::PrepareSimulation()
{
    // We need to put cards in local players hand back into deck, shuffle both decks
    // and have the local player redraw the same number of cards
    auto HandSize = LocalPlayer.Hand.size();
    for( auto It = LocalPlayer.Hand.begin(); It != LocalPlayer.Hand.end(); It++ )
    {
        It->Position = CardPos::DECK;
        LocalPlayer.Deck.push_back( *It );
    }
    
    LocalPlayer.Hand.clear();
    ShuffleDeck( std::addressof( LocalPlayer ) );
    ShuffleDeck( std::addressof( Opponent ) );
    
    for( int i = 0; i < HandSize; i++ )
    {
        if( LocalPlayer.Deck.size() == 0 )
        {
            cocos2d::log( "[AI] FATAL ERROR: Failed to redraw player hand after shuffle in simulated state.. pre rollout!" );
            return;
        }
        
        // Get random index
        int Index = cocos2d::random( 0, (int) LocalPlayer.Deck.size() - 1 );
        auto It = LocalPlayer.Deck.begin();
        std::advance( It, Index );
        It->Position = CardPos::HAND;
        LocalPlayer.Hand.push_back( *It );
        LocalPlayer.Deck.erase( It );
    }
}

void SimulatedState::SimulatePlayerBlitz()
{
    // We need to select pseudo-random blitz cards for local player
    // Were going to introduce some rules though, for instance, were going to use at
    // least 50% of mana, and favor saving a couple mana for after the blitz
    int MinMana = LocalPlayer.Mana / 2;
    int TargetMax = LocalPlayer.Mana - 2;
    int UsedMana = 0;
    
    if( TargetMax < MinMana )
        TargetMax = LocalPlayer.Mana;
    
    std::vector< CardState* > Selection;
    std::vector< CardState* > Hand;
    
    for( auto It = LocalPlayer.Hand.begin(); It != LocalPlayer.Hand.end(); It++ )
        Hand.push_back( std::addressof( *It ) );
    
    for( int i = 0; i < Hand.size(); i++ )
    {
        // Choose random index
        int Index = cocos2d::random( 0, (int) Hand.size() - 1 );
        auto It = Hand.begin();
        std::advance( It, Index );
        
        // Check if this card is playable
        if( *It && (*It)->ManaCost + UsedMana <= TargetMax )
        {
            UsedMana += (*It)->ManaCost;
            Selection.push_back( *It );
            Hand.erase( It );
        }
    }
    
    for( auto It = Selection.begin(); It != Selection.end(); It++ )
    {
        for( auto j = LocalPlayer.Hand.begin(); j != LocalPlayer.Hand.end(); j++ )
        {
            if( std::addressof( *j ) == *It )
            {
                int NewMana = LocalPlayer.Mana - j->ManaCost;
                if( NewMana < 0 )
                {
                    cocos2d::log( "[AI] SIM ERROR: Not enough mana for local player blitz" );
                    continue;
                }
                
                LocalPlayer.Mana = NewMana;
                j->Position = CardPos::FIELD;
                LocalPlayer.Field.push_back( *j );
                LocalPlayer.Hand.erase( j );
                break;
            }
        }
    }
}

void SimulatedState::FinishBlitz()
{
    // The cards will have already been played, we just need to
    // jump into the main game loop
    CallHook( "BlitzFinish" );
    PreTurn( pState );
} 

void SimulatedState::PreTurn( PlayerTurn InState )
{
    mState = MatchState::Main;
    pState = InState;
    tState = TurnState::PreTurn;
    
    // Increment turn counter
    if( StartingPlayer == pState )
    {
        if( TurnNumber >= FinalTurn )
        {
            // Decrement back
            OnSimulationFinished();
            return;
        }
        else
        {
            TurnNumber++;
        }
    }
    
    // Draw Card
    auto& Player = GetActivePlayer();
    if( Player.Deck.size() == 0 )
    {
        //cocos2d::log( "[Sim] Win by empty deck" );
        OnSimulationFinished( std::addressof( GetInactivePlayer() ) );
        return;
    }
    
    auto It = Player.Deck.begin();
    It->Position = CardPos::HAND;
    Player.Hand.push_back( *It );
    Player.Deck.erase( It );
    
    auto NewCard = Player.Hand.back();
    CallHook( "OnDraw", std::addressof( Player ), std::addressof( NewCard ) );
    
    // Give 2 Mana
    Player.Mana += 2;
    
    // Advance to marshal
    Marshal();
}

void SimulatedState::Marshal()
{
    mState = MatchState::Main;
    tState = TurnState::Marshal;
    
    // Simulate play choices
    // For now, were going to make a random choice
    auto& Player = GetActivePlayer();
    CallHook( "MarshalStart", std::addressof( Player ) );
    
    int TotalMana = Player.Mana;
    
    // Check if we have enough mana to play any cards at all
    bool bCanPlay = false;
    for( auto It = Player.Hand.begin(); It != Player.Hand.end(); It++ )
    {
        if( It->ManaCost <= TotalMana )
        {
            bCanPlay = true;
            break;
        }
    }
    
    if( !bCanPlay )
    {
        Attack();
        return;
    }
    
    // Pick random cards from hand until we either run out, or hit mana limit
    std::vector< CardState* > HandCopy;
    for( auto It = Player.Hand.begin(); It != Player.Hand.end(); It++ )
        HandCopy.push_back( std::addressof( *It ) );
    
    std::vector< uint32_t > TargetCards;
    int UsedMana = 0;
    
    for( int i = 0; i < HandCopy.size(); i++ )
    {
        auto Index = HandCopy.size() > 1 ? cocos2d::random( 0, (int) HandCopy.size() - 1 ) : 0;
        auto It = HandCopy.begin();
        std::advance( It, Index );
        
        // Check if theres enough mana to play this card
        if( *It && UsedMana + (*It)->ManaCost <= TotalMana )
        {
            TargetCards.push_back( (*It)->EntId );
            UsedMana += (*It)->ManaCost;
            HandCopy.erase( It );
        }
    }
    
    // Choose number of cards to play
    if( !TargetCards.empty() )
    {
        int Count = cocos2d::random( 0, (int) TargetCards.size() );
        
        // If we have enough mana to play more than 2 cards, we should at least play 1
        if( TargetCards.size() > 2 && Count < 1 )
            Count = 1;
        
        for( int i = 0; i < Count; i++ )
        {
            auto It = TargetCards.begin();
            std::advance( It, i );
            
            if( !PlayCard( std::addressof( Player ), *It ) )
            {
                cocos2d::log( "[Sim] Warning: Should have been able to simulate playing this card but it failed!" );
            }
            else
            {
                CallHook( "PlayCard", std::addressof( Player ), *It );
            }
        }
    }
    
    Ability();
}

void SimulatedState::Ability()
{
    mState = MatchState::Main;
    tState = TurnState::Marshal;

    // TODO: Build list of possible abilities.. pick random (valid) combination
    
    Attack();
}

void SimulatedState::Attack()
{
    mState = MatchState::Main;
    tState = TurnState::Attack;
    
    // Choose number of cards to attack with
    auto& Player = GetActivePlayer();
    CallHook( "AttackStart", std::addressof( Player ) );
    
    auto& Opponent = GetInactivePlayer();
    int AttackCount = 0;
    
    // If the opponent doesnt have any cards on field, we will attack with all we have
    if( Opponent.Field.size() == 0 )
        AttackCount = (int) Player.Field.size();
    else
    {
        // If we have a lot more power on field than the opponent, we will attack harder
        int PlPower = 0;
        int OpPower = 0;
        for( auto It = Player.Field.begin(); It != Player.Field.end(); It++ )
            PlPower += It->Power;
        for( auto It = Opponent.Field.begin(); It != Opponent.Field.end(); It++ )
            OpPower += It->Power;
        
        int PlFieldCount = (int) Player.Field.size();
        
        // If we have enough attack to win the game, then attack with everything
        if( PlPower > OpPower + Opponent.Health )
        {
            AttackCount = PlFieldCount;
        }
        else if( PlPower > OpPower )
        {
            AttackCount = cocos2d::random( PlFieldCount / 2, PlFieldCount );
        }
        else
        {
            AttackCount = PlFieldCount > 0 ? cocos2d::random( 0, PlFieldCount ) : 0;
        }
    }
    
    
    BattleMatrix.clear();
    
    // Copy card pointers into new vector
    std::vector< CardState* > Field;
    for( auto It = Player.Field.begin(); It != Player.Field.end(); It++ )
    {
        Field.push_back( std::addressof( *It ) );
    }
    
    // Pick random cards to attack with from vector
    while( !Field.empty() && BattleMatrix.size() < AttackCount )
    {
        // Pick random index
        int Index = Field.size() > 1 ? cocos2d::random( 0, (int) Field.size() - 1 ) : 0;
        auto It = Field.begin();
        std::advance( It, Index );
        
        // Add this card to the attack vector
        if( *It )
        {
            BattleMatrix.insert( std::make_pair( (*It)->EntId, std::vector< uint32_t >() ) );
        }
        
        Field.erase( It );
    }
    
    // Advance to block
    Block();
}

void SimulatedState::Block()
{
    mState = MatchState::Main;
    tState = TurnState::Block;
    
    auto& Player = GetInactivePlayer();
    
    // TODO: Call Lua Hook
    
    // Pick cards to block with
    // First, build list of possible blockers
    std::vector< CardState* > Blockers;
    for( auto It = Player.Field.begin(); It != Player.Field.end(); It++ )
    {
        Blockers.push_back( std::addressof( *It ) );
    }
    
    // Next, loop through battle matrix and pick a random blocker for each attacker
    for( auto It = BattleMatrix.begin(); It != BattleMatrix.end(); It++ )
    {
        if( Blockers.empty() )
            break;
        
        int Index = Blockers.size() > 1 ? cocos2d::random( 0, (int) Blockers.size() - 1 ) : 0;
        auto Block = Blockers.begin();
        std::advance( Block, Index );
        
        // Add to battle matrix
        It->second.push_back( (*Block)->EntId );
        
        // Pop from blocker list
        Blockers.erase( Block );
    }
    
    // Advance to damage
    Damage();
}

void SimulatedState::Damage()
{
    mState = MatchState::Main;
    tState = TurnState::Damage;
    
    // TODO: Call Lua Hook
    
    // Loop through battle matrix, and perform damage
    auto& AttackingPlayer = GetActivePlayer();
    auto& BlockingPlayer = GetInactivePlayer();
    
    for( auto It = BattleMatrix.begin(); It != BattleMatrix.end(); It++ )
    {
        CardState* Attacker = nullptr;
        if( FindCard( It->first, std::addressof( AttackingPlayer ), Attacker ) && Attacker )
        {
            auto Blockers = It->second;
            bool bAttackerDead = false;
            
            if( Blockers.empty() )
            {
                // Damage Player
                BlockingPlayer.Health -= Attacker->Power;
            }
            else
            {
                // Damage Cards
                int TotalDamage = Attacker->Power;
                
                for( auto j = Blockers.begin(); j != Blockers.end(); j++ )
                {
                    CardState* Blocker = nullptr;
                    if( FindCard( *j, std::addressof( BlockingPlayer ), Blocker ) && Blocker )
                    {
                        // Deal damage
                        int ThisDamage = TotalDamage > Blocker->Power ? Blocker->Power : TotalDamage;
                        Blocker->Power -= ThisDamage;
                        Attacker->Power -= ThisDamage;
                        Blocker->Stamina -= 1;
                        
                        if( Blocker->Power <= 0 || Blocker->Stamina <= 0 )
                        {
                            OnCardKilled( Blocker );
                        }
                        
                        if( Attacker->Power <= 0 || Attacker->Stamina <= 0 )
                        {
                            bAttackerDead = true;
                            OnCardKilled( Attacker );
                            break;
                        }
                        
                        TotalDamage -= ThisDamage;
                        if( TotalDamage <= 0 )
                        {
                            break;
                        }
                    }
                    else
                    {
                        cocos2d::log( "[Sim] Failed to find blocker in battle matrix!" );
                    }
                }
            }
            
            if( !bAttackerDead )
            {
                Attacker->Stamina -= 1;
                if( Attacker->Stamina <= 0 )
                {
                    OnCardKilled( Attacker );
                }
            }
        }
        else
        {
            cocos2d::log( "[Sim] Failed to find attacker in battle matrix!" );
        }
    }
    
    if( BlockingPlayer.Health <= 0 )
    {
        OnSimulationFinished( std::addressof( AttackingPlayer ) );
        BattleMatrix.clear();
        return;
    }
    
    // Advance to post turn
    BattleMatrix.clear();
    PostTurn();
}

void SimulatedState::PostTurn()
{
    mState = MatchState::Main;
    tState = TurnState::PostTurn;
    
    // Advance to next turn
    if( pState == PlayerTurn::LocalPlayer )
        PreTurn( PlayerTurn::Opponent );
    else
        PreTurn( PlayerTurn::LocalPlayer );
}

void SimulatedState::RunSimulation( int MaxTurns )
{
    FinalTurn = TurnNumber + MaxTurns;
    SimulationStart = TurnNumber;
    
    if( MaxTurns <= 0 )
    {
        OnSimulationFinished();
        return;
    }
    
    // Now, we need to start running the actual simulation
    // Before we get to this point, we should be setup and ready to go
    if( mState == MatchState::Blitz )
    {
        FinishBlitz();
    }
    else if( mState == MatchState::Main )
    {
        // Advance to the next turn phase
        if( tState == TurnState::Marshal )
        {
            Attack();
        }
        else if( tState == TurnState::Attack )
        {
            Block();
        }
        else if( tState == TurnState::Block )
        {
            Damage();
        }
        else
        {
            // TODO: Handle Gracefully
            CC_ASSERT( true );
        }
    }
    else
    {
        // TODO: Handle Gracefully
        CC_ASSERT( true );
    }
}


void SimulatedState::OnSimulationFinished( PlayerState* Winner )
{
    WinningPlayer = Winner;
    
    if( Winner )
        cocos2d::log( "[Sim] Simulated game won by %s", Winner->DisplayName.c_str() );
    else
        cocos2d::log( "[Sim] Simulated game resulted in a draw/not finished" );
     
}
