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



// ALL CODE BELOW IS DEPRECATED!!

/*
CardState* DoLookup( std::deque< CardState >& Container, uint32_t Id )
{
    for( auto It = Container.begin(); It != Container.end(); It++ )
    {
        if( It->EntId == Id )
        {
            return std::addressof( *It );
        }
    }
    
    return nullptr;
}


CardState* SimulatedPlayer::LookupCard( uint32_t Id )
{
    auto cDeck = DoLookup( Deck, Id );
    if( cDeck ) { return cDeck; }
    auto cHand = DoLookup( Hand, Id );
    if( cHand ) { return cHand; }
    auto cField = DoLookup( Field, Id );
    if( cField ) { return cField; }
    auto cGrave = DoLookup( Graveyard, Id );
    if( cGrave ) { return cGrave; }
    
    return nullptr;
}




SimulatedState::SimulatedState()
{
}

bool DoKill( SimulatedPlayer* inPlayer, CardState* inCard )
{
    if( !inPlayer || !inCard )
        return false;
    
    inCard->Position = CardPos::GRAVEYARD;
    inPlayer->Graveyard.push_front( *inCard );
    
    for( auto It = inPlayer->Field.begin(); It != inPlayer->Field.end(); It++ )
    {
        if( It->EntId == inCard->EntId )
        {
            inPlayer->Field.erase( It );
            return true;
        }
    }
    
    return false;
}

bool SimulatedState::KillCard( CardState* inCard )
{
    if( !inCard )
        return false;
    
    if( inCard->Owner == LocalPlayer.State.EntId )
    {
        return DoKill( &LocalPlayer, inCard );
    }
    
    if( inCard->Owner == Opponent.State.EntId )
    {
        return DoKill( &Opponent, inCard );
    }
    
    return false;
}

bool SimulatedState::PerformDraw( SimulatedPlayer* inPlayer )
{
    if( !inPlayer )
        return true;
    
    // Check if there is a card to draw
    if( inPlayer->Deck.size() <= 0 )
    {
        // TODO: Win
        return false;
    }
    
    auto& Card = inPlayer->Deck.front();
    Card.Position = CardPos::HAND;
    
    if( inPlayer->bIsOpponent )
        inPlayer->Hand.push_front( Card );
    else
        inPlayer->Hand.push_back( Card );
    
    inPlayer->Deck.pop_front();
    return true;
}

SimulatedPlayer* SimulatedState::LookupPlayer( uint32_t Id )
{
    if( LocalPlayer.State.EntId == Id )
    {
        return std::addressof( LocalPlayer );
    }
    else if( Opponent.State.EntId == Id )
    {
        return std::addressof( Opponent );
    }
    
    return nullptr;
}


CardState* SimulatedState::LookupCard( uint32_t Id )
{
    auto Card = LocalPlayer.LookupCard( Id );
    if( Card )
        return Card;
    
    auto OpCard = Opponent.LookupCard( Id );
    if( OpCard )
        return OpCard;
    
    return nullptr;
}

SimulatedPlayer* SimulatedState::GetActivePlayer()
{
    if( pState == PlayerTurn::LocalPlayer )
        return &LocalPlayer;
    else
        return &Opponent;
}

SimulatedPlayer* SimulatedState::GetInactivePlayer()
{
    if( pState == PlayerTurn::LocalPlayer )
        return &Opponent;
    else
        return &LocalPlayer;
}

bool SimulatedState::PlayCard( CardState* inCard )
{
    auto Player = GetActivePlayer();
    if( !Player || !inCard )
        return false;
    
    if( mState != MatchState::Main || tState != TurnState::Marshal )
        return false;
    
    for( auto It = Player->Hand.begin(); It != Player->Hand.end(); It++ )
    {
        if( It->EntId == inCard->EntId )
        {
            // Check if player has enough mana
            if( Player->State.Mana < It->ManaCost )
            {
                cocos2d::log( "[Simulator] Failed to play card.. not enough mana!" );
                return false;
            }
            
            Player->State.Mana -= It->ManaCost;
            inCard->Position = CardPos::FIELD;
            Player->Field.push_back( *inCard );
            Player->Hand.erase( It );
            
            // TODO: Call Hook
            
            return true;
        }
    }
    
    cocos2d::log( "[Simulator] Failed to play card.. couldnt find in active players hand" );
    return false;
    
}

bool SimulatedState::CanTriggerAbility( CardState* inCard, uint8_t inAbility )
{
    if( mState != MatchState::Main || tState != TurnState::Marshal )
    {
        cocos2d::log( "[Sim DEBUG] Bad Match State" );
        return false;
    }
    
    auto Player = GetActivePlayer();
    if( !Player || !inCard )
    {
        cocos2d::log( "[Sim DEBUG] No Player/Card" );
        return false;
    }
    
    if( inCard->Owner != Player->State.EntId )
    {
        cocos2d::log( "[Sim DEBUG] Wrong Player turn" );
        return false;
    }
    
    auto& CM = CardManager::GetInstance();
    CardInfo Info;
    
    if( !CM.GetInfo( inCard->Id, Info ) || Info.Abilities.count( inAbility ) <= 0 )
    {
        cocos2d::log( "[Sim DEBUG] Missing Info" );
        return false;
    }
    
    auto& Ability = Info.Abilities.at( inAbility );
    if( Ability.ManaCost > Player->State.Mana || Ability.StaminaCost > inCard->Stamina )
    {
        return false;
    }
    
    if( Ability.CheckFunc && Ability.CheckFunc->isFunction() )
    {
        try
        {
            // RAN CHECK FUNC
        }
        catch( std::exception& e )
        {
            cocos2d::log( "[Lua] Simulator Check Func Error: %s", e.what() );
            return false;
        }
    }
    
    if( !Ability.MainFunc || !Ability.MainFunc->isFunction() )
    {
        cocos2d::log( "[Sim DEBUG] No Main Func" );
        return false;
    }
    
    return true;
}

bool SimulatedState::TriggerAbility( CardState* inCard, uint8_t inAbility )
{
    auto Player = GetActivePlayer();
    if( !Player || !inCard )
        return false;
    
    if( !CanTriggerAbility( inCard, inAbility ) )
    {
        cocos2d::log( "[Simulator] Couldnt trigger ability!" );
        return false;
    }
    
    auto& CM = CardManager::GetInstance();
    CardInfo Info;
    if( !CM.GetInfo( inCard->Id, Info ) )
    {
        return false;
    }
    
    auto& Ability = Info.Abilities.at( inAbility );
    
    // Take Mana and Stamina
    Player->State.Mana -= Ability.ManaCost;
    inCard->Stamina -= Ability.StaminaCost;
    
    try
    {
        Context.SetState( *inCard );
        ( *Ability.MainFunc )( Context );
    }
    catch( std::exception& e )
    {
        cocos2d::log( "[Lua] Simulator Ability Error: %s", e.what() );
        return false;
    }
    
    return true;
}

bool SimulatedState::SetAttackers( std::vector< CardState* > Attackers )
{
    if( mState != MatchState::Main || tState != TurnState::Attack )
    {
        cocos2d::log( "[Simulator] Set Attackers called outside of attack phase" );
        return false;
    }
    
    auto Player = GetActivePlayer();
    if( !Player )
        return false;
    
    // All cards must be on the field
    BattleMatrix.clear();
    
    for( auto It = Attackers.begin(); It != Attackers.end(); It++ )
    {
        if( !( *It ) )
        {
            cocos2d::log( "[Simulator] Null card found in attack request" );
            BattleMatrix.clear();
            return false;
        }
        
        // Ensure this card is owned by the players who turn it is, and that its on the field
        if( (*It)->Position != CardPos::FIELD || (*It)->Owner != Player->State.EntId )
        {
            cocos2d::log( "[Simulator] Invalid attacker selected.. card not on active players field" );
            BattleMatrix.clear();
            return false;
        }
        
        if( BattleMatrix.count( (*It)->EntId ) > 0 )
        {
            cocos2d::log( "[Simulator] Invalid attacker selected.. card is already in matrix" );
            BattleMatrix.clear();
            return false;
        }
        
        // Add to matrix
        BattleMatrix[ (*It)->EntId ] = std::vector< uint32_t >();
        
    }
    
    cocos2d::log( "[Simulator] Battle Matrix Attackers Set!" );
    return true;
}

bool SimulatedState::SetBlockers( std::map< CardState*, CardState* > Blockers )
{
    auto Player = GetActivePlayer();
    auto Opponent = GetInactivePlayer();
    
    if( !Player || !Opponent )
        return false;
    
    // Clear Matrix
    for( auto It = BattleMatrix.begin(); It != BattleMatrix.end(); It++ )
        It->second.clear();
    
    // Check if everything is valid
    for( auto It = Blockers.begin(); It != Blockers.end(); It++ )
    {
        // Validate Attacker
        if( !It->second || BattleMatrix.count( It->second->EntId ) <= 0 )
        {
            cocos2d::log( "[Simulator] Invalid 'Attacker' set in BlockerMatrix!" );
            return false;
        }
        
        // Validate Blocker
        if( !It->first || It->first->Owner != Opponent->State.EntId || It->first->Position != CardPos::FIELD )
        {
            cocos2d::log( "[Simulator] Invalid 'Blocker' set in BlockerMatrix" );
            return false;
        }
        
        BattleMatrix[ It->second->EntId ].push_back( It->first->EntId );
    }
    
    cocos2d::log( "[Simulator] Battle Matrix Finalized!" );
    return true;
}

void SimulatedState::SimulateTurn()
{
    if( mState == MatchState::Blitz )
        FinishBlitz();
    else if( mState == MatchState::Main )
        StartPreTurn();
    else
    {
        cocos2d::log( "[Simulator] Failed to simulate turn.. match state invalid" );
    }
}


void SimulatedState::FinishBlitz()
{
    if( mState != MatchState::Blitz )
    {
        cocos2d::log( "[Simulator] Failed to finish blitz.. match state is outside blitz" );
        return;
    }
    
    cocos2d::log( "[Simulator] Finishing Blitz" );
    mState = MatchState::Main;
    StartPreTurn();
}

void SimulatedState::StartPreTurn()
{
    mState = MatchState::Main;
    tState = TurnState::PreTurn;
    
    cocos2d::log( "[Simulator] Starting PreTurn.." );
    
    // TODO: Hooks
    
    // Perform Draw
    if( !PerformDraw( GetActivePlayer() ) )
    {
        // Do Win
    }
    
    // Move Into Marshal
    StartMarshal();
}

void SimulatedState::StartMarshal()
{
    mState = MatchState::Main;
    tState = TurnState::Marshal;
    
    cocos2d::log( "[Simulator] Starting Marshal..." );
    
    if( SimulateMarshal )
        SimulateMarshal();
    else
        cocos2d::log( "[Simulator] Failed to simulate marshal.. no handler set" );
    
    StartAttack();
}

void SimulatedState::StartAttack()
{
    mState = MatchState::Main;
    tState = TurnState::Attack;
    
    cocos2d::log( "[Simulator] Starting Attack.." );
    
    if( SimulateAttack )
        SimulateAttack();
    else
        cocos2d::log( "[Simulator] Failed to simulate attack.. no handler set" );
    
    StartBlock();
}

void SimulatedState::StartBlock()
{
    mState = MatchState::Main;
    tState = TurnState::Block;
    
    cocos2d::log( "[Simulator] Starting Block.." );
    
    if( SimulateBlock )
        SimulateBlock();
    else
        cocos2d::log( "[Simulator] Failed to simulate block.. no handler set" );
    
    StartDamage();
}

void SimulatedState::StartDamage()
{
    mState = MatchState::Main;
    tState = TurnState::Damage;
    
    auto Player = GetActivePlayer();
    auto Opponent = GetInactivePlayer();
    if( !Opponent || !Player )
    {
        cocos2d::log( "[Simulator] ERROR: Couldnt get opponent to deal damage" );
        BattleMatrix.clear();
        FinishTurn();
        return;
    }
    
    cocos2d::log( "[Simulator] Starting Damage.." );
    
    for( auto It = BattleMatrix.begin(); It != BattleMatrix.end(); It++ )
    {
        // Lookup Card
        CardState* Attacker = nullptr;
        for( auto fIt = Player->Field.begin(); fIt != Player->Field.end(); fIt++ )
        {
            if( fIt->EntId == It->first )
            {
                Attacker = std::addressof( *fIt );
                break;
            }
        }
        
        if( !Attacker )
        {
            cocos2d::log( "[Simulator] Invalid Attacker found in battle matrix!" );
            continue;
        }
        
        // Have blockers absorb damage
        int Power = Attacker->Power;
        for( auto bIt = It->second.begin(); bIt != It->second.end(); bIt++ )
        {
            CardState* Blocker = nullptr;
            for( auto fIt = Opponent->Field.begin(); fIt != Opponent->Field.end(); fIt++ )
            {
                if( fIt->EntId == *bIt )
                {
                    Blocker = std::addressof( *fIt );
                    break;
                }
            }
            
            if( !Blocker)
            {
                cocos2d::log( "[Simulator] Null blocker found in matrix" );
                continue;
            }
            
            int DamageAmount = Power <= Blocker->Power ? Power : Blocker->Power;
            
            Power           -= DamageAmount;
            Blocker->Power  -= DamageAmount;
            
            if( Blocker->Power <= 0 )
            {
                // Kill Blocker
                KillCard( Blocker );
            }
            
            if( Power <= 0 )
            {
                // Kill Attacker, Stop Inner Loop
                Attacker->Power = 0;
                KillCard( Attacker );
                break;
            }
        }
        
        // If theres remaining power, damage the opponent
        if( Power > 0 )
        {
            cocos2d::log( "[Simulator] Damaging Opponent..." );
            Opponent->State.Health -= Power;
            
            if( Opponent->State.Health <= 0 )
            {
                cocos2d::log( "[Simulator] Opponent Died!" );
                // TODO: Game Win!
            }
            
            // Update Attacker Power
            Attacker->Power = Power;
        }
    }
    
    FinishTurn();
}

void SimulatedState::FinishTurn()
{
    mState = MatchState::Main;
    tState = TurnState::PostTurn;
    
    cocos2d::log( "[Simulator] Finishing Turn.." );
    
    if( OnTurnResults )
        OnTurnResults();
    else
        cocos2d::log( "[Simulator] No turn results callback set!" );
    
    if( pState == PlayerTurn::LocalPlayer )
        pState = PlayerTurn::Opponent;
    else
        pState = PlayerTurn::LocalPlayer;
    
    if( TurnHalf )
        TurnNumber++;
    
    TurnHalf = !TurnHalf;
    
}

void SimulatedState::CopyTo( SimulatedState& Other )
{
    Other.mState = mState;
    Other.pState = pState;
    Other.tState = tState;
    
    Other.LocalPlayer   = LocalPlayer;
    Other.Opponent      = Opponent;
    
    // Were using ent id's in the battle matrix so we can easily copy it around
    // without loosing references/pointers to the original object
    Other.BattleMatrix = BattleMatrix;
    
    Other.TurnHalf = TurnHalf;
    Other.TurnNumber = TurnNumber;

}

void CopyCards( std::deque< CardState >& To, CardIter Start, CardIter End )
{
    for( auto It = Start; It != End; It++ )
    {
        if( *It )
        {
            To.push_back( (*It)->GetState() );
        }
    }
}

void CopyPlayer( SimulatedPlayer& inPlayer, Player* Target )
{
    if( !Target )
    {
        cocos2d::log( "[Simulator] Failed to copy player from game state.. player was null" );
        return;
    }
    
    inPlayer.Deck.clear();
    inPlayer.Hand.clear();
    inPlayer.Field.clear();
    inPlayer.Graveyard.clear();
    
    auto Hand = Target->GetHand();
    if( Hand )
        CopyCards( inPlayer.Hand, Hand->Begin(), Hand->End() );
    
    auto Field = Target->GetField();
    if( Field )
        CopyCards( inPlayer.Field, Field->Begin(), Field->End() );
    
    auto Deck = Target->GetDeck();
    if( Deck )
        CopyCards( inPlayer.Deck, Deck->Begin(), Deck->End() );
    
    auto Grave = Target->GetGraveyard();
    if( Grave )
        CopyCards( inPlayer.Graveyard, Grave->Begin(), Grave->End() );
    
    inPlayer.bIsOpponent    = Target->IsOpponent();
    inPlayer.State          = Target->GetState();
}


void SimulatedState::Sync()
{
    auto World = Game::World::GetWorld();
    auto GM = World ? World->GetGameMode() : nullptr;
    auto Auth = World ? World->GetAuthority< SingleplayerAuthority >() : nullptr;
    
    if( !Auth || !GM )
    {
        cocos2d::log( "[Simulator] Failed to sync with active game.. couldnt get game state" );
        return;
    }
    
    // Copy Player States and Cards
    CopyPlayer( LocalPlayer, GM->GetState().GetPlayer() );
    CopyPlayer( Opponent, GM->GetState().GetOpponent() );
    
    // Copy Battle Matrix
    BattleMatrix.clear();
    
    // Copy Match/Turn State
    auto State = Auth->GetState();
    
    mState = State.mState;
    tState = State.tState;
    pState = State.pState;
    
    TurnNumber = State->TurnNumber;
    
    for( auto It = Auth->BattleMatrix.begin(); It != Auth->BattleMatrix.end(); It++ )
    {
        if( It->first )
        {
            std::vector< uint32_t > Blockers;
            for( auto bIt = It->second.begin(); bIt != It->second.end(); bIt++ )
            {
                if( (*bIt ) )
                {
                    Blockers.push_back( (*bIt)->GetEntityId() );
                }
            }
            
            BattleMatrix[ It->first->GetEntityId() ] = Blockers;
        }
    }
    
}
*/
