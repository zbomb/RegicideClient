//
//    SingleplayerAuthority.cpp
//    Regicide Mobile
//
//    Created: 11/11/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#include "SingleplayerAuthority.hpp"
#include "Player.hpp"
#include "DeckEntity.hpp"
#include "HandEntity.hpp"
#include "FieldEntity.hpp"
#include "GraveyardEntity.hpp"
#include "SingleplayerGameMode.hpp"
#include "Actions.hpp"
#include "AIController.hpp"

#define GAME_INITDRAW_COUNT 8

using namespace Game;

SingleplayerAuthority::SingleplayerAuthority()
: AuthorityBase()
{
    _bBlitzComplete = false;
}

SingleplayerAuthority::~SingleplayerAuthority()
{
}

void SingleplayerAuthority::Cleanup()
{
    AuthorityBase::Cleanup();
}

void SingleplayerAuthority::PostInit()
{
    // Call base class method
    EntityBase::PostInit();

    // Setup State, wait for a few seconds before starting the match
    //mState = MatchState::PreMatch;
    State.mState = MatchState::PreMatch;
    
    using namespace std::placeholders;
    WaitOnPlayer( std::bind( &SingleplayerAuthority::StartGame, this, _1, _2 ), 4.f );
    
    // Setup Tick
    cocos2d::Director::getInstance()->getScheduler()->schedule( std::bind( &SingleplayerAuthority::Tick, this, _1 ), this, 0.f, CC_REPEAT_FOREVER, 0.f, false, "SingleplayerAuthTick" );
}

void SingleplayerAuthority::SceneInit( cocos2d::Scene *inScene )
{
    EntityBase::SceneInit( inScene );
}


/*=================================================================================
    Game Flow Logic
 =================================================================================*/


void SingleplayerAuthority::SetReady()
{
    // This function is used to notify the authority that the player is ready
    // to advance the game state. Example: Waiting for an animation, or at start of game
    if( _fWaitCallback )
    {
        // Calc how long it took for the player to call this function
        float Delta = std::chrono::duration_cast< std::chrono::seconds >( std::chrono::steady_clock::now() - _tWaitStart ).count();
        _fWaitCallback( Delta, false );
    }
    
    // Kill timeout timer
    cocos2d::Director::getInstance()->getScheduler()->unschedule( "WaitOnTimeout", this );
}


void SingleplayerAuthority::WaitOnPlayer( std::function< void( float, bool ) > OnReady, float Timeout )
{
    _fWaitCallback = OnReady;
    _tWaitStart = std::chrono::steady_clock::now();
    
    // Start timeout timer
    cocos2d::Director::getInstance()->getScheduler()->schedule( std::bind( OnReady, std::placeholders::_1, true ), this, Timeout, 0, 0.f, false, "WaitOnTimeout" );
}


void SingleplayerAuthority::StartGame( float Delay, bool bTimeout )
{
    cocos2d::log( "[Auth] Starting Game.." );
    State.mState = MatchState::CoinFlip;
    
    // Choose Player
    int RandIndex = cocos2d::random( 0, 1 );
    if( RandIndex <= 0 )
        State.SetStartingPlayer( PlayerTurn::LocalPlayer );
    else
        State.SetStartingPlayer( PlayerTurn::Opponent );
    
    // DEBUG: Force localplayer to start
    State.SetStartingPlayer( PlayerTurn::LocalPlayer );
    
    // Run Action
    auto GM = GetGameMode< GameModeBase >();
    CC_ASSERT( GM );
    
    auto Queue = ActionQueue();
    Queue.Callback = std::bind( &SingleplayerAuthority::CoinFlipFinish, this );
    
    auto Flip = Queue.CreateAction< CoinFlipAction >();
    Flip->PlayerId = GetActivePlayer().EntId;
    
    GM->RunActionQueue( std::move( Queue ) );
}


void SingleplayerAuthority::CoinFlipFinish()
{
    auto GM = GetGameMode< GameModeBase >();
    CC_ASSERT( GM );
    
    // Update Match State
    State.mState = MatchState::Blitz;
    
    // Create Event Action
    auto Queue = ActionQueue();
    auto Blitz = Queue.CreateAction< EventAction >();
    Blitz->Name = "Blitz";
    
    auto Parallel = Queue.CreateAction< ParallelAction >();
    
    auto LocalPlayer = State.GetPlayer();
    auto Opponent = State.GetOpponent();
    
    if( LocalPlayer.Deck.size() <= GAME_INITDRAW_COUNT )
    {
        // Wtf?
        return;
    }
    
    if( Opponent.Deck.size() <= GAME_INITDRAW_COUNT )
    {
        // Wtf?
        return;
    }
    
    // Draw the top X cards
    for( int i = 0; i < GAME_INITDRAW_COUNT; i++ )
    {
        auto PlayerCard = LocalPlayer.Deck.begin();
        auto OpponentCard = Opponent.Deck.begin();
        
        PlayerCard->Position    = CardPos::HAND;
        OpponentCard->Position  = CardPos::HAND;
        PlayerCard->FaceUp      = false;
        OpponentCard->FaceUp    = false;
        
        LocalPlayer.Hand.push_back( *PlayerCard );
        Opponent.Hand.push_back( *PlayerCard );
        
        auto PlDraw = Parallel->CreateAction< DrawCardAction >();
        PlDraw->TargetPlayer = LocalPlayer.EntId;
        PlDraw->TargetCard = PlayerCard->EntId;
        
        auto OpDraw = Parallel->CreateAction< DrawCardAction >();
        OpDraw->TargetPlayer = Opponent.EntId;
        OpDraw->TargetCard = OpponentCard->EntId;
        
        LocalPlayer.Deck.erase( PlayerCard );
        Opponent.Deck.erase( OpponentCard );
    }
    
    // Add Query Action
    auto Query = Queue.CreateAction< TimedQueryAction >();
    Query->Name = "BlitzQuery";
    Query->Deadline = std::chrono::steady_clock::now() + std::chrono::seconds( 20 );
    
    PlayerBlitzSelection.clear();
    OpponentBlitzSelection.clear();
    
    GM->RunActionQueue( std::move( Queue ) );
}

/*========================================================================================
    Local Player Input
========================================================================================*/
void SingleplayerAuthority::SetBlitzCards( const std::vector< uint32_t >& Cards )
{
    if( State.mState != MatchState::Blitz )
    {
        cocos2d::log( "[Authority] Player attempted to set blitz cards outside of blitz round!" );
        return;
    }
    
    // Check if blitz cards were already selected
    if( PlayerBlitzSelection.size() > 0 )
    {
        cocos2d::log( "[Authority] Player attempted to re-select blitz cards!" );
        return;
    }
    
    // We need to validate the selection
    std::map< uint32_t, uint8_t > Errors;
    auto& Player = State.GetPlayer();
    int ManaLeft = Player.Mana;
    
    for( auto It = Cards.begin(); It != Cards.end(); It++ )
    {
        // Card needs to be in players hand
        std::vector< CardState >::iterator Target = Player.Hand.end();
        for( auto i = Player.Hand.begin(); i != Player.Hand.end(); i++ )
        {
            if( i->EntId == *It )
            {
                Target = i;
                break;
            }
        }
        
        // Check if card was found
        if( Target == Player.Hand.end() )
        {
            Errors[ *It ] = PLAY_ERROR_BADCARD;
            continue;
        }
        
        // Check if this card was selected twice
        int Count = 0;
        for( auto i = Cards.begin(); i != Cards.end(); i++ )
        {
            if( *i == *It )
            {
                Count++;
                if( Count > 1 )
                    break;
            }
        }
        
        if( Count > 1 )
        {
            Errors[ *It ] = PLAY_ERROR_INVALID;
            continue;
        }
        
        // Check if theres enough mana
        if( Target->ManaCost > ManaLeft )
        {
            Errors[ *It ] = PLAY_ERROR_BADMANA;
            continue;
        }
        
        ManaLeft -= Target->ManaCost;
        PlayerBlitzSelection.push_back( *Target );
    }
    
    // Check if there were errors
    if( !Errors.empty() )
    {
        PlayerBlitzSelection.clear();
        
        auto GM = GetGameMode< GameModeBase >();
        CC_ASSERT( GM );
        
        auto Queue = ActionQueue();
        auto Error = Queue.CreateAction< CardErrorAction >();
        Error->Errors = Errors;
        
        GM->RunActionQueue( std::move( Queue ) );
    }
}


void SingleplayerAuthority::FinishBlitz()
{
    auto GM = GetGameMode< GameModeBase >();
    CC_ASSERT( GM );
    
    auto Player = State.GetPlayer();
    auto Opponent = State.GetOpponent();
    
    // Play Selected Cards, Advance State
    auto Queue = ActionQueue();
    Queue.Callback = std::bind( &SingleplayerAuthority::StartMatch, this );
    
    auto Parallel = Queue.CreateAction< ParallelAction >();
    
    for( auto It = PlayerBlitzSelection.begin(); It != PlayerBlitzSelection.end(); It++ )
    {
        // Move card onto field in local state
        if( !State.PlayCard( Player, *It ) )
        {
            cocos2d::log( "[Auth] Failed to play blitz card on local state" );
        }
        else
        {
            auto Play = Parallel->CreateAction< PlayCardAction >();
            Play->TargetCard = It->EntId;
            Play->TargetPlayer = Player.EntId;
        }
    }
    
    for( auto It = OpponentBlitzSelection.begin(); It != OpponentBlitzSelection.end(); It++ )
    {
        if( !State.PlayCard( Opponent, *It ) )
        {
            cocos2d::log( "[Auth] Failed to play blitz card on local state" );
        }
        else
        {
            auto Play = Parallel->CreateAction< PlayCardAction >();
            Play->TargetCard = *It;
            Play->TargetPlayer = Opponent.EntId;
        }
    }
    
    auto PlMana = Parallel->CreateAction< UpdateManaAction >();
    PlMana->TargetPlayer = Player.EntId;
    PlMana->Amount = Player.Mana;
    
    auto OpMana = Parallel->CreateAction< UpdateManaAction >();
    OpMana->TargetPlayer = Opponent.EntId;
    OpMana->Amount = Opponent.Mana;
    
    State.SetActiveQueue( &Queue );
    State.CallHook( "BlitzFinish" );
    State.ClearActiveQueue();
    
    GM->RunActionQueue( std::move( Queue ) );
}

void SingleplayerAuthority::StartMatch()
{
    cocos2d::log( "[Auth] Starting match!" );
    
    // Jump Into Main Game Loop
    PreTurn( State.pState );
}

PlayerState& SingleplayerAuthority::GetActivePlayer()
{
    if( State.pState == PlayerTurn::LocalPlayer )
        return State.GetPlayer();
    else
        return State.GetOpponent();
}

PlayerState& SingleplayerAuthority::GetInactivePlayer()
{
    if( State.pState == PlayerTurn::LocalPlayer )
        return State.GetOpponent();
    else
        return State.GetPlayer();
}

void SingleplayerAuthority::PreTurn( PlayerTurn pTurn )
{
    if( State.mState == MatchState::PostMatch )
        return;
    
    // Update State
    State.pState = pTurn;
    State.mState = MatchState::Main;
    State.tState = TurnState::PreTurn;
    
    // Perform Draw
    auto GM = GetGameMode< GameModeBase >();
    CC_ASSERT( GM );
    
    auto& Player = GetActivePlayer();
    
    auto Queue = ActionQueue();
    auto Event = Queue.CreateAction< TurnStartAction >();
    Event->Player = Player.EntId;
    
    auto DrawnCard = State.DrawSingle( Player );
    if( DrawnCard == 0 )
    {
        cocos2d::log( "[Auth] Player ran out of cards!" );
        OnGameWon( Player.EntId );
        return;
    }
    
    // Give Mana
    Player.Mana += 2;
    
    auto UpdateMana = Queue.CreateAction< UpdateManaAction >();
    UpdateMana->TargetPlayer = Player.EntId;
    UpdateMana->Amount = Player.Mana;
    
    // Draw Card
    auto Draw = Queue.CreateAction< DrawCardAction >();
    Draw->TargetPlayer = Player.EntId;
    Draw->TargetCard = DrawnCard;
    
    // Call Lua Hook
    CardState* Target = nullptr;
    if( State.GetCard( DrawnCard, Player, Target) && Target )
    {
        State.SetActiveQueue( &Queue );
        State.CallHook( "OnDraw", Player, *Target );
        State.ClearActiveQueue();
    }
    
    // On callback, advance round state
    Queue.Callback = std::bind( &SingleplayerAuthority::Marshal, this );
    GM->RunActionQueue( std::move( Queue ) );

}

void SingleplayerAuthority::Marshal()
{
    if( State.mState == MatchState::PostMatch )
        return;
    
    cocos2d::log( "[Auth] Marshal Started..." );
    
    // Ensure State Is Updated
    State.tState = TurnState::Marshal;
    State.mState = MatchState::Main;
    
    auto GM = GetGameMode< GameModeBase >();
    CC_ASSERT( GM );
    
    auto Queue = ActionQueue();
    auto Update = Queue.CreateAction< EventAction >();
    Update->Name = "MarshalStart";
    
    // Call Hook
    State.SetActiveQueue( &Queue );
    State.CallHook( "OnMarshal" );
    State.ClearActiveQueue();
    
    GM->RunActionQueue( std::move( Queue ) );
    
    // Allow player to play cards, once the player is unable to perform
    // any actions, then the state will advance automatically
    
    if( State.pState == PlayerTurn::Opponent )
    {
        cocos2d::Director::getInstance()->getScheduler()->schedule( [=]( float d )
        {
            if( AI )
            {
                AI->StartMarshal();
            }
            
        }, this, 1.f, 0, 0.f, false, "DEBUG_AIPlayCard" );
    }
    
}


void SingleplayerAuthority::Attack()
{
    if( State.mState == MatchState::PostMatch )
        return;
    
    cocos2d::log( "[Auth] Attack Started" );
    
    // Update State
    State.tState = TurnState::Attack;
    State.mState = MatchState::Main;
    
    auto GM = GetGameMode< GameModeBase >();
    CC_ASSERT( GM );
    
    auto Queue = ActionQueue();
    auto Update = Queue.CreateAction< EventAction >();
    Update->Name = "AttackStart";
    
    // Call Hook
    State.SetActiveQueue( &Queue );
    State.CallHook( "AttackStart", GetActivePlayer() );
    State.ClearActiveQueue();
    
    GM->RunActionQueue( std::move( Queue ) );

    
    // Allow player/opponent to select attackers, the player must call FinishTurn
    // to advance the round state
    
    if( State.pState == PlayerTurn::Opponent )
    {
        // Attack with one card
        cocos2d::Director::getInstance()->getScheduler()->schedule( [=]( float d )
        {
            if( AI )
            {
                AI->StartAttack();
            }
        }, this, 1.f, 0, 0.f, false, "DEBUG_AI_Attack" );
    }
}

void SingleplayerAuthority::Block()
{
    if( State.mState == MatchState::PostMatch )
        return;
    
    // Completley skip block phase if there are no attackers
    // The damage phase will automatically move into post if the BattleMatrix is empty
    if( BattleMatrix.empty() )
    {
        Damage();
        return;
    }
    
    cocos2d::log( "[Auth] Block Started" );
    
    // Update State
    State.tState = TurnState::Block;
    State.mState = MatchState::Main;
    
    auto GM = GetGameMode< GameModeBase >();
    CC_ASSERT( GM );
    
    auto Queue = ActionQueue();
    
    auto Update = Queue.CreateAction< EventAction >();
    Update->Name = "BlockStart";
    
    // Call Hook
    // Create a table of all attackers
    auto Lua = Regicide::LuaEngine::GetInstance();
    
    if( Lua && Lua->State() )
    {
        auto Table = luabridge::newTable( Lua->State() );
        auto& AttackingPlayer = GetActivePlayer();
        
        int Index = 1;
        for( auto It = BattleMatrix.begin(); It != BattleMatrix.end(); It++ )
        {
            CardState* Attacker = nullptr;
            if( State.GetCard( It->first, AttackingPlayer, Attacker, true ) && Attacker )
            {
                Table[ Index ] = *Attacker;
                Index++;
            }
        }
        
        State.SetActiveQueue( &Queue );
        State.CallHook( "BlockStart", GetInactivePlayer(), Table );
        State.ClearActiveQueue();
    }
    
    GM->RunActionQueue( std::move( Queue ) );
    
    // Allow player/opponent to select blockers, the player must call FinishTurn
    // to advance the round state
    
    // DEBUG
    if( State.pState == PlayerTurn::LocalPlayer )
    {
        cocos2d::Director::getInstance()->getScheduler()->schedule( [=]( float d )
        {
            if( AI )
            {
                AI->StartBlock();
            }
            
        }, this, 0.75f, 0, 0.f, false, "DEBUG_AI_Block" );
    }
    
}

void SingleplayerAuthority::Damage()
{
    if( State.mState == MatchState::PostMatch )
        return;
    
    cocos2d::log( "[Auth] Damage Started" );
    
    // Update State
    State.tState = TurnState::Damage;
    State.mState = MatchState::Main;
    
    if( BattleMatrix.empty() )
    {
        cocos2d::log( "[Auth] No Attackers!" );
        cocos2d::Director::getInstance()->getScheduler()->schedule( [=] ( float d ) { this->PostTurn(); }, this, 0.5f, 0, 0.f, false, "MoveToPost" );
        return;
    }
    
    cocos2d::log( "[Auth] %d Attackers!", (int) BattleMatrix.size() );
    
    auto GM = GetGameMode< GameModeBase >();
    CC_ASSERT( GM );
    
    auto Queue = ActionQueue();
    auto Event = Queue.CreateAction< EventAction >();
    Event->Name = "DamageStart";
    
    auto& AttackingPlayer = GetActivePlayer();
    auto& BlockingPlayer = GetInactivePlayer();
    auto Lua = Regicide::LuaEngine::GetInstance();
    auto L = Lua ? Lua->State() : nullptr;
    
    if( L )
    {
        auto Table = luabridge::newTable( L );
        
        for( auto It = BattleMatrix.begin(); It != BattleMatrix.end(); It++ )
        {
            CardState* Attacker = nullptr;
            
            if( State.GetCard( It->first, AttackingPlayer, Attacker ) && Attacker )
            {
                Table[ *Attacker ] = luabridge::newTable( L );
                int Index = 1;
                
                for( auto BIt = It->second.begin(); BIt != It->second.end(); BIt++ )
                {
                    CardState* Blocker = nullptr;
                    if( State.GetCard( *BIt, BlockingPlayer, Blocker ) && Blocker )
                    {
                        Table[ *Attacker ][ Index ] = *Blocker;
                        Index++;
                    }
                }
            }
        }
        
        State.SetActiveQueue( &Queue );
        State.CallHook( "StartDamage", AttackingPlayer, BlockingPlayer, Table );
        State.ClearActiveQueue();
    }
    
    for( auto It = BattleMatrix.begin(); It != BattleMatrix.end(); It++ )
    {
        auto& Attacker = It->first;
        auto& BlockerList = It->second;
        
        uint16_t TotalAttack = Attacker.Power;
        
        if( BlockerList.empty() )
        {
            auto Damage = Queue.CreateAction< DamageAction >();
            Damage->Inflictor = Attacker.EntId;
            Damage->Target = BlockingPlayer.EntId;
            Damage->Damage = TotalAttack;
            Damage->StaminaDrain = 1;
        }
        else
        {
            // Dole out damage among blockers
            for( auto BIt = BlockerList.begin(); BIt != BlockerList.end(); BIt++ )
            {
                auto& Blocker = *BIt;
                
                uint16_t CardDamage = TotalAttack > Blocker.Power ? Blocker.Power : TotalAttack;
                TotalAttack -= CardDamage;
                
                Attacker.Power = TotalAttack;
                Blocker.Power -= CardDamage;
                
                Blocker.Stamina -= 1;
                Attacker.Stamina -= 1;
                
                auto Combat = Queue.CreateAction< CombatAction >();
                
                Combat->Attacker                = Attacker.EntId;
                Combat->Blocker                 = Blocker.EntId;
                Combat->FinalAttackerPower      = Attacker.Power;
                Combat->FinalBlockerPower       = Blocker.Power;
                Combat->FinalAttackerStamina    = Attacker.Stamina;
                Combat->FinalBlockerStamina     = Blocker.Stamina;
                
                // Check for death
                if( Attacker.Power <= 0 || Attacker.Stamina <= 0 )
                {
                    State.OnCardKilled( Attacker );
                    break;
                }
                
                if( Blocker.Power <= 0 || Blocker.Stamina <= 0 )
                {
                    State.OnCardKilled( Blocker );
                }
            }
            
            // TODO: Call Lua Hook
            
            auto Finish = Queue.CreateAction< EventAction >();
            Finish->Name = "CleanupBoard";
            
            Queue.Callback = std::bind( &SingleplayerAuthority::PostTurn, this );
            GM->RunActionQueue( std::move( Queue ) );
        }
    }
    
    // Clear Battle Matrix
    BattleMatrix.clear();
    
}


void SingleplayerAuthority::AI_PlayCard( uint32_t In, std::function< void() > Callback )
{
    if( State.mState == MatchState::PostMatch )
        return;
    
    auto GM = GetGameMode< GameModeBase >();
    CC_ASSERT( GM );
    
    auto& Opponent = State.GetOpponent();
    
    auto Queue = ActionQueue();
    if( Callback )
        Queue.Callback = Callback;
    
    if( State.PlayCard( Opponent, In ) )
    {
        // Validate Move
        auto Parallel = Queue.CreateAction< ParallelAction >();
        auto Play = Parallel->CreateAction< PlayCardAction >();
        Play->TargetPlayer = Opponent.EntId;
        Play->TargetCard = In;
        Play->bWasSuccessful = true;
        
        auto Mana = Parallel->CreateAction< UpdateManaAction >();
        Mana->TargetPlayer = Opponent.EntId;
        Mana->Amount = Opponent.Mana;
        
        GM->RunActionQueue( std::move( Queue ) );
        
        CardState* Target = nullptr;
        if( State.GetCard( In, Opponent, Target, true ) )
        {
            State.SetActiveQueue( &Queue );
            State.CallHook( "PlayCard", Opponent, *Target );
            State.ClearActiveQueue();
        }
    }
    else
    {
        cocos2d::log( "[Auth] AI attempted to play an invalid card!" );
        
        if( Callback )
            Callback();
    }
}


void SingleplayerAuthority::PlayCard( uint32_t In, int Index )
{
    if( State.mState == MatchState::PostMatch )
        return;
    
    auto GM = GetGameMode< GameModeBase >();
    CC_ASSERT( GM );
    
    auto& Player = State.GetPlayer();
    
    auto Queue = ActionQueue();
    auto Play = Queue.CreateAction< PlayCardAction >();
    Play->TargetPlayer = Player.EntId;
    Play->TargetCard = In;
    Play->TargetIndex = Index;
    
    if( State.PlayCard( Player, In ) )
    {
        Play->bWasSuccessful = true;
        
        auto Mana           = Queue.CreateAction< UpdateManaAction >();
        Mana->TargetPlayer  = Player.EntId;
        Mana->Amount        = Player.Mana;
        
        CardState* Target = nullptr;
        if( State.GetCard( In, Player, Target, true ) )
        {
            State.SetActiveQueue( &Queue );
            State.CallHook( "PlayCard", Player, *Target );
            State.ClearActiveQueue();
        }
    }
    else
    {
        cocos2d::log( "[Auth] Player attempted to play invalid card!" );
    }
    
    GM->RunActionQueue( std::move( Queue ) );
}

void SingleplayerAuthority::PostTurn()
{
    if( State.mState == MatchState::PostMatch )
        return;
    
    cocos2d::log( "[Auth] Post Turn" );
    
    State.tState = TurnState::PostTurn;
    State.mState = MatchState::Main;
    
    auto GM = GetGameMode< GameModeBase >();
    CC_ASSERT( GM );
    
    auto Queue = ActionQueue();
    auto Event = Queue.CreateAction< EventAction >();
    Event->Name = "PostTurnStart";
    
    State.SetActiveQueue( &Queue );
    State.CallHook( "PostTurn", GetActivePlayer() );
    State.ClearActiveQueue();
    
    PlayerTurn NextTurn = State.SwitchPlayerTurn();
    Queue.Callback = std::bind( &SingleplayerAuthority::PreTurn, this, NextTurn );
    
    GM->RunActionQueue( std::move( Queue ) );
}

void SingleplayerAuthority::FinishTurn()
{
    if( State.pState == PlayerTurn::LocalPlayer )
    {
        if( State.tState == TurnState::Marshal )
            Attack();
        else if( State.tState == TurnState::Attack )
            Block();
    }
    else
    {
        if( State.tState == TurnState::Block )
            Damage();
    }
}

void SingleplayerAuthority::AI_SetAttackers( const std::vector< uint32_t >& In )
{
    if( State.pState != PlayerTurn::Opponent ||
        State.mState != MatchState::Main ||
        State.tState != TurnState::Attack )
    {
        cocos2d::log( "[AI] Attempt to set attackers outside of proper round state!" );
        return;
    }
    
    BattleMatrix.clear();
    auto& Opponent = State.GetOpponent();
    
    for( auto It = In.begin(); It != In.end(); It++ )
    {
        auto Card = Opponent.Field.end();
        for( auto i = Opponent.Field.begin(); i != Opponent.Field.end(); i++ )
        {
            if( i->EntId == *It )
            {
                Card = i;
                break;
            }
        }
        
        if( Card == Opponent.Field.end() || Card->Power <= 0 || Card->Stamina <= 0 )
        {
            cocos2d::log( "[Auth] AI attempted to attack with invalid card!" );
            continue;
        }
        
        BattleMatrix.insert( std::make_pair( *Card, std::vector< CardState& >() ) );
        
        // TODO: Tell GameMode to highlight these cards
    }
    
    Block();
}


void SingleplayerAuthority::SetAttackers( const std::vector< uint32_t >& In )
{
    if( State.pState != PlayerTurn::LocalPlayer ||
       State.mState != MatchState::Main ||
       State.tState != TurnState::Attack )
    {
        cocos2d::log( "[Auth] Attempt to set attackers outisde of player attack phase!" );
        return;
    }
    
    BattleMatrix.clear();
    auto& Player = State.GetPlayer();
    bool bError = false;
    
    auto GM = GetGameMode< GameModeBase >();
    CC_ASSERT( GM );
    
    for( auto It = In.begin(); It != In.end(); It++ )
    {
        CardState* Target = nullptr;
        if( !State.GetCard( *It, Player, Target, true )
           || !Target || Target->Power <= 0 || Target->Stamina <= 0 )
        {
            cocos2d::log( "[Auth] Player attempted to attack with an invalid card!" );
            bError = true;
            break;
        }
        
        BattleMatrix.insert( std::make_pair( *It, std::vector< uint32_t >() ) );
        
        // TODO: Tell GameMode to highlight these cards
    }
    
    if( bError )
    {
        BattleMatrix.clear();
        
        auto Queue = ActionQueue();
        auto Err = Queue.CreateAction< CardErrorAction >()
        Err->Name = "AttackError";
        
        GM->RunActionQueue( std::move( Err ) );
        return;
    }
    
    Block();
}


void SingleplayerAuthority::AI_SetBlockers( const std::map< uint32_t, uint32_t >& Matrix )
{
    if( State.pState != PlayerTurn::LocalPlayer ||
        State.mState != MatchState::Main ||
        State.tState != TurnState::Block )
    {
        cocos2d::log( "[AI] Attempt to set blockers outside of opponent block phase!" );
        Damage();
        return;
    }
    
    auto& AttackingPlayer = State.GetPlayer();
    auto& BlockingPlayer = State.GetOpponent();
    
    for( auto It = Matrix.begin(); It != Matrix.end(); It++ )
    {
        CardState* Blocker      = nullptr;
        CardState* Attacker     = nullptr;
        
        if( !State.GetCard( It->first, BlocingPlayer, Blocker, true ) || !Blocker )
        {
            cocos2d::log( "[Auth] AI Set Blockers: Bad blocker.. wasnt found" );
            continue;
        }
        if( !State.GetCard( It->second, AttackingPlayer, Attacker, true ) || !Attacker )
        {
            cocos2d::log( "[Auth] AI Set Blockers: Bad attacker.. wasnt found" );
            continue;
        }
        
        // Validate Pair
        if( Blocker->Power <= 0 || Blocker->Stamina <= 0 || Attacker->Power <= 0 || Attacker->Stamina <= 0 )
        {
            cocos2d::log( "[Auth] AI Set Blockers: Bad block matchup! Check stamina and power of both cards" );
            continue;
        }
        
        BattleMatrix[ It->second ].push_back( It->first );
    }
    
    Damage();
}


void SingleplayerAuthority::SetBlockers( const std::map< uint32_t, uint32_t >& Matrix )
{
    if( State.pState != PlayerTurn::Opponent ||
        State.mState != MatchState::Main ||
        State.tState != TurnState::Block )
    {
        cocos2d::log( "[Auth] Player attempted to set blockers outside of correct phase" );
        return;
    }
    
    auto& AttackingPlayer = State.GetOpponent();
    auto& BlockingPlayer = State.GetPlayer();
    bool bError = false;
    
    for( auto It = Matrix.begin(); It != Matrix.end(); It++ )
    {
        CardState* Attacker     = nullptr;
        CardState* Blocker      = nullptr;
        
        if( !State.GetCard( It->first, BlockingPlayer, Blocker, true ) || !Blocker )
        {
            cocos2d::log( "[Auth] Player Set Blockers: Blocking card wasnt found" );
            bError = true;
            break;
        }
        
        if( !State.GetCard( It->second, AttackingPlayer, Attacker, true ) || !Attacker )
        {
            cocos2d::log( "[Auth] Player Set Blockers: Attacking card wasnt found" );
            bError = true;
            break;
        }
        
        if( Blocker->Power <= 0 || Blocker->Stamina <= 0 || Attacker->Power <= 0 || Attacker->Stamina <= 0 )
        {
            cocos2d::log( "[Auth] Player Set Blockers: Invalid combat pairing!" );
            bError = true;
            break;
        }
        
        BattleMatrix[ It->second ].push_back( It->first );
    }
    
    if( bError )
    {
        for( auto It = BattleMatrix.begin(); It != BattleMatrix.end(); It++ )
            It->second.clear();
        
        return;
    }
    
    Damage();
}


void SingleplayerAuthority::TriggerAbility( uint32_t Card, uint8_t AbilityId )
{
    // NOTE: For now, this is only for the local player
    CardState* Target   = nullptr;
    auto& Player        = State.GetPlayer();
    auto GM             = GetGameMode< GameModeBase >();
    
    CC_ASSERT( GM );
    
    if( !State.GetCard( Card, Player, Target ) || !Target )
    {
        cocos2d::log( "[Auth] Failed to trigger ability.. couldnt find card!" );
        return;
    }
    
    auto& CM = CardManager::GetInstance();
    CardInfo TargetInfo;
    
    if( !CM.GetInfo( Target->Id, TargetInfo ) )
    {
        cocos2d::log( "[Auth] Failed to trigger ability.. couldnt load card info!" );
        return;
    }
    
    if( TargetInfo.Abilities.count( AbilityId ) <= 0 )
    {
        cocos2d::log( "[Auth] Failed to trigger ability.. ability id was invalid" );
        return;
    }
    
    auto& Ability = TargetInfo.Abilities.at( AbilityId );
    if( Ability.ManaCost > Player.Mana || Ability.StaminaCost > Target->Stamina )
    {
        cocos2d::log( "[Auth] Failed to trigger ability.. not enough mana/stamina" );
        return;
    }
    
    // Run Custom Check Function
    if( Ability.CheckFunc && Ability.CheckFunc->isFunction() )
    {
        try
        {
            if( !( *Ability.CheckFunc )( State, Player, *Target ) )
            {
                cocos2d::log( "[Auth] Failed to trigger ability.. Check func returned false" );
                return;
            }
        }
        catch( std::exception& e )
        {
            cocos2d::log( "[Auth] An exception was thrown while performing ability check func! %s", e.what() );
            return;
        }
    }
    
    // Ensure function is valid
    if( !Ability.MainFunc || !Ability.MainFunc->isFunction() )
    {
        cocos2d::log( "[Auth] Failed to trigger ability.. Main function null" );
        return;
    }
    
    // Build Action Queue
    auto Queue = ActionQueue();
    auto Parallel = Queue.CreateAction< ParallelAction >();
    if( Ability.ManaCost > 0 )
    {
        // Take Mana
        Player.Mana -= Ability.ManaCost;
        
        // Add Action
        auto Mana = Parallel->CreateAction< UpdateManaAction >();
        Mana->TargetPlayer = Player.EntId;
        Mana->Amount = Player.Mana;
    }
    
    if( Ability.StaminaCost > 0 )
    {
        // Take Stamina
        Target->Stamina -= Ability.StaminaCost;
        
        // Add Action
        auto Stamina = Parallel->CreateAction< UpdateStaminaAction >();
        Stamina->Target = Target->EntId;
        Stamina->Inflictor = Target->EntId;
        Stamina->Amount = Target->Stamina;
    }
    
    State.SetActiveQueue( &Queue );
    
    try
    {
        ( *Ability.MainFunc )( State, *Target );
    }
    catch( std::exception& e )
    {
        cocos2d::log( "[Auth] An exception was thrown in an ability function! %s", e.what() );
        
        State.ClearActiveQueue();
        return;
    }
    
    // Call Hook
    State.CallHook( "AbilityTriggered", Player, *Target, Ability );
    State.RunActiveQueue();
    
}


void SingleplayerAuthority::OnGameWon( uint32_t Winner )
{
    // Stop all timers
    cocos2d::Director::getInstance()->getScheduler()->unscheduleAllForTarget( this );
    
    // Create Action
    auto GM     = GetGameMode< GameModeBase >();
    auto Queue  = ActionQueue();
    
    auto Win = Queue.CreateAction< WinAction >();
    if( Winner == State.GetPlayer().EntId )
    {
        cocos2d::log( "[Auth] Local Player Won!!" );
        Win->Player = Winner;
    }
    else if( Winner == State.GetOpponent().EntId )
    {
        cocos2d::log( "[Auth] Opponent Won!!" );
        Win->Player = Winner;
    }
    else
    {
        cocos2d::log( "[Auth] Win Error: Specified winner id was invalid!" );
        Win->Player = 0;
    }
    
    State.mState = MatchState::PostMatch;
    GM->RunActionQueue( std::move( Queue ) );
}


void SingleplayerAuthority::Tick( float Delta )
{
    if( State.mState == MatchState::Main )
    {
        auto& Player = State.GetPlayer();
        auto& Opponent = State.GetOpponent();
        
        // Check for win
        if( Opponent.Health <= 0 || Opponent.Deck.size() <= 0 )
        {
            OnGameWon( Opponent.EntId );
        }
        else if( Player.Health <= 0 || Player.Deck.size() <= 0 )
        {
            OnGameWon( Player.EntId );
        }
    }
    else if( State.mState == MatchState::Blitz )
    {
        // Check for blitz completion
        if( !_bBlitzComplete && PlayerBlitzSelection.size() > 0 && OpponentBlitzSelection.size() > 0 )
        {
            _bBlitzComplete = true;
            FinishBlitz();
        }
    }
}


bool SingleplayerAuthority::DoLoad( PlayerState &Target, const std::string &Name, uint16_t Health, uint16_t Mana, const Regicide::Deck &Deck )
{
    auto& Ent   = IEntityManager::GetInstance();
    auto& CM    = CardManager::GetInstance();
    
    // Create Player Object
    Target.DisplayName  = Name;
    Target.Mana         = Mana;
    Target.Health       = Health;
    Target.EntId        = Ent.AllocateIdentifier();
    Target.King         = Deck.KingId;
    
    Target.Deck.clear();
    Target.Field.clear();
    Target.Hand.clear();
    Target.Graveyard.clear();
    
    // Build Deck
    for( auto It = Deck.Cards.begin(); It != Deck.Cards.end(); It++ )
    {
        CardInfo Info;
        if( CM.GetInfo( It->Id, Info ) )
        {
            for( int i = 0; i < It->Ct; i++ )
            {
                auto Card       = CardState();
                Card.EntId      = Ent.AllocateIdentifier();
                Card.Id         = It->Id;
                Card.FaceUp     = false;
                Card.Owner      = Target.EntId;
                Card.Position   = CardPos::DECK;
                Card.ManaCost   = Info.ManaCost;
                Card.Power      = Info.Power;
                Card.Stamina    = Info.Stamina;
                
                Target.Deck.push_back( Card );
                State.OnCardAdded( std::addressof( Target.Deck.back() ) );
            }
        }
        else
        {
            cocos2d::log( "[Auth] Failed to load card '%d' for player '%s'", (int) It->Id, Target.DisplayName.c_str() );
        }
    }
}

bool SingleplayerAuthority::LoadPlayers( const std::string &LocalPlayerName, uint16_t PlayerHealth, uint16_t PlayerMana, const Regicide::Deck &PlayerDeck, const std::string &OpponentName, uint16_t OpponentHealth, uint16_t OpponentMana, const Regicide::Deck &OpponentDeck )
{
    
    // Load both players
    if( !DoLoad( State.LocalPlayer, LocalPlayerName, PlayerHealth, PlayerMana, PlayerDeck ) )
    {
        cocos2d::log( "[Auth] Failed to load local player!" );
        return false;
    }
    
    if( !DoLoad( State.Opponent, OpponentName, OpponentHealth, OpponentMana, OpponentDeck ) )
    {
        cocos2d::log( "[Auth] Failed to load opponent!" );
        return false;
    }
    
    // Shuffle Decks
    State.ShuffleDeck( State.LocalPlayer );
    State.ShuffleDeck( State.Opponent );
    
    // Setup States
    State.mState = MatchState::PreMatch;
    State.pState = PlayerTurn::None;
    State.tState = TurnState::None;
    
    return true;
}
