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

#define GAME_INITDRAW_COUNT 8

using namespace Game;

ActionQueue* SingleplayerAuthority::Lua_RootQueue;
Action* SingleplayerAuthority::Lua_RootAction( nullptr );
Action* SingleplayerAuthority::Lua_LastSerial( nullptr );
Action* SingleplayerAuthority::Lua_LastParallel( nullptr );
uint32_t SingleplayerAuthority::OpponentDeckIndex( 0 );
uint32_t SingleplayerAuthority::OwnerDeckIndex( 0 );

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
    mState = MatchState::PreMatch;
    
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
    cocos2d::log( "[Auth] Starting Game..." );
    mState = MatchState::CoinFlip;
    
    // Choose player to start first
    int RandValue = cocos2d::random( 0, 1 );
    //if( RandValue <= 0 )
        pState = PlayerTurn::LocalPlayer;
    //else
        //pState = PlayerTurn::Opponent;
    
    // Inform GameMode
    GameModeBase* GM = GetGameMode< GameModeBase >();
    CC_ASSERT( GM );
    
    auto Queue = ActionQueue();
    Queue.Callback = std::bind( &SingleplayerAuthority::CoinFlipFinish, this );
    
    auto coinFlip = Queue.CreateAction< CoinFlipAction >( GM );
    coinFlip->StartingPlayer = pState;
    
    SetLuaActionRoot( &Queue, coinFlip );
    CallHook( "CoinFlip", pState == PlayerTurn::LocalPlayer ? GetPlayer() : GetOpponent() );
    ClearLuaActionRoot();
    
    GM->RunActionQueue( std::move( Queue ) );
}

void SingleplayerAuthority::CoinFlipFinish()
{
    GameModeBase* GM = GetGameMode< GameModeBase >();
    CC_ASSERT( GM );
    
    // Update Match State
    mState = MatchState::Blitz;
    
    auto Queue = ActionQueue();
    auto blitzStart = Queue.CreateAction< EventAction >( GM );
    blitzStart->ActionName = "BlitzStart";
    
    // Create 'DrawCard' actions for both players
    auto Pl = GetPlayer();
    auto Op = GetOpponent();
    
    CC_ASSERT( Pl );
    CC_ASSERT( Op );
    
    auto PlDeck = Pl->GetDeck();
    auto OpDeck = Op->GetDeck();
    
    CC_ASSERT( PlDeck );
    CC_ASSERT( OpDeck );
    
    Action* lastPlayerAction = blitzStart;
    Action* lastOpponentAction = blitzStart;
    
    for( int i = 0; i < GAME_INITDRAW_COUNT; i++ )
    {
        // Create draw actions that will run sequentially, each players draws will
        // be parallel to one another though
        auto plCard = PlDeck->At( i );
        if( plCard )
        {
            auto plDraw = lastPlayerAction->CreateAction< DrawCardAction >( Pl );
            plDraw->TargetCard = plCard->GetEntityId();
            
            lastPlayerAction = plDraw;
            
            SetLuaActionRoot( &Queue, lastPlayerAction );
            SetLuaDeckIndex( Pl, i + 1 );
            CallHook( "DrawCard", plCard );
            ClearLuaActionRoot();
        }
        
        auto opCard = OpDeck->At( i );
        if( opCard )
        {
            auto opDraw = lastOpponentAction->CreateAction< DrawCardAction >( Op );
            opDraw->TargetCard = opCard->GetEntityId();
            
            lastOpponentAction = opDraw;
            
            SetLuaActionRoot( &Queue, lastOpponentAction );
            SetLuaDeckIndex( Op, i + 1 );
            CallHook( "DrawCard", opCard );
            ClearLuaActionRoot();
        }
    }
    
    // Create action to query blitz cards from the player
    auto blitzQuery = lastPlayerAction->CreateAction< TimedQueryAction >( GM );
    blitzQuery->ActionName = "BlitzQuery";
    blitzQuery->Deadline = std::chrono::steady_clock::now() + std::chrono::seconds( 25 );
    
    SetLuaActionRoot( &Queue, blitzQuery );
    CallHook( "BlitzStart" );
    ClearLuaActionRoot();
    
    GM->RunActionQueue( std::move( Queue ) );
}


void SingleplayerAuthority::SetBlitzCards( const std::vector< CardEntity* >& Cards )
{
    cocos2d::log( "[Auth] Received blitz selection" );
    
    // Ensure the game is in the blitz game state
    if( mState != MatchState::Blitz )
    {
        cocos2d::log( "[Authority] Player attempted to set blitz cards outside of the blitz round!" );
        return;
    }
    
    // Validate
    std::map< uint32_t, uint8_t > Errors;
    int TotalManaCost = 0;
    auto* LocalPlayer = GetPlayer();
    
    PlayerBlitzSelection.clear();
    
    for( auto It = Cards.begin(); It != Cards.end(); It++ )
    {
        if( !( *It ) )
        {
            cocos2d::log( "[Authority] Failed to set blitz card! Card was null.." );
            Errors[ 0 ] = PLAY_ERROR_INVALID;
            continue;
        }
        
        // Check if card is in hand and owned by the local player
        if( ( *It )->GetOwningPlayer() != LocalPlayer || !( *It )->InHand() )
        {
            cocos2d::log( "[Authority] Failed to set blitz card! Card was not in local players hand" );
            Errors[ (*It)->GetEntityId() ] = PLAY_ERROR_BADCARD;
            continue;
        }
        
        // Finally, check mana cost
        if( TotalManaCost + ( *It )->ManaCost > LocalPlayer->GetMana() )
        {
            cocos2d::log( "[Authority] Failed to set blits card! Not enough mana!" );
            Errors[ (*It)->GetEntityId() ] = PLAY_ERROR_BADMANA;
            continue;
        }
        
        TotalManaCost += ( *It )->ManaCost;
        PlayerBlitzSelection.push_back( *It );
    }
    
    // If there were errors, inform the player
    if( Errors.size() > 0 )
    {
        auto GM = GetGameMode< SingleplayerGameMode >();
        CC_ASSERT( GM );
        
        auto Queue = ActionQueue();
        auto errEvent = Queue.CreateAction< CardErrorAction >( GM );
        errEvent->ActionName = "BlitzError";
        errEvent->Errors = Errors;
        
        GM->RunActionQueue( std::move( Queue ) );
    }
    else
    {
        FinishBlitz();
    }
}

void SingleplayerAuthority::FinishBlitz()
{
    auto GM = GetGameMode< SingleplayerGameMode >();
    auto LocalPlayer = GetPlayer();
    auto Opponent = GetOpponent();
    CC_ASSERT( GM && LocalPlayer && Opponent );
    
    cocos2d::log( "[Auth] %d blitz cards selected", (int) PlayerBlitzSelection.size() );
    
    // AI cards should be selected by this point, were going to play all card simultaneously
    // then advance the round state into main
    auto Queue = ActionQueue();
    Queue.Callback = std::bind( &SingleplayerAuthority::StartMatch, this );
    
    // Advance Match State
    auto mainStart = Queue.CreateAction< EventAction >( GM );
    mainStart->ActionName = "MatchStart";
    
    int UpdatedMana = LocalPlayer->GetMana();
    for( auto It = PlayerBlitzSelection.begin(); It != PlayerBlitzSelection.end(); It++ )
    {
        auto playCard = mainStart->CreateAction< PlayCardAction >( LocalPlayer );
        
        playCard->TargetCard        = (*It)->GetEntityId();
        playCard->bWasSuccessful    = true;
        playCard->bNeedsMove        = true;
        playCard->TargetIndex       = 1000;
        
        UpdatedMana -= (*It)->ManaCost;
    }
    
    // Update Mana
    auto updateMana = Queue.CreateAction< UpdateManaAction >( LocalPlayer );
    updateMana->UpdatedMana = UpdatedMana;
    
    // TODO: AI Blitz Logic
    
    SetLuaActionRoot( &Queue, updateMana );
    CallHook( "PostBlitz" );
    ClearLuaActionRoot();
    
    GM->RunActionQueue( std::move( Queue ) );
}

void SingleplayerAuthority::StartMatch()
{
    cocos2d::log( "[Auth] Starting match!" );
    
    // Jump Into Main Game Loop
    PreTurn( pState );
}

Player* SingleplayerAuthority::CurrentTurnPlayer()
{
    if( pState == PlayerTurn::LocalPlayer )
        return GetPlayer();
    else
        return GetOpponent();
}

void SingleplayerAuthority::PreTurn( PlayerTurn pTurn )
{
    if( mState == MatchState::PostMatch )
        return;
    
    // Update State
    pState = pTurn;
    mState = MatchState::Main;
    tState = TurnState::PreTurn;
    
    // Set Player Turn
    auto Pl = GetPlayer();
    auto Op = GetOpponent();
    if( pTurn == PlayerTurn::LocalPlayer )
    {
        if( Pl )
            Pl->SetTurn( true );
        if( Op )
            Op->SetTurn( false );
    }
    else if( pTurn == PlayerTurn::Opponent )
    {
        if( Pl )
            Pl->SetTurn( false );
        if( Op )
            Op->SetTurn( true );
    }
    else
    {
        if( Pl )
            Pl->SetTurn( false );
        if( Op )
            Op->SetTurn( false );
    }
    
    // Perform Draw
    auto GM = GetGameMode< GameModeBase >();
    auto ActivePlayer = CurrentTurnPlayer();
    CC_ASSERT( ActivePlayer && GM );
    
    auto Queue = ActionQueue();
    auto deck = ActivePlayer->GetDeck();
    
    // Check if card can be drawn
    if( !deck || deck->Count() <= 0 || !deck->At( 0 ) )
    {
        cocos2d::log( "[Auth] Player ran out of cards!" );
        auto Winner = ActivePlayer == GetPlayer() ? GetOpponent() : GetPlayer();
        OnGameWon( Winner );
        
        return;
    }
    
    // Give Mana
    auto UpdateMana = Queue.CreateAction< UpdateManaAction >( ActivePlayer );
    UpdateMana->UpdatedMana = ActivePlayer->GetMana() + 2;
    
    // Advance Turn State Clientside
    auto turnStart = Queue.CreateAction< TurnStartAction >( GM );
    turnStart->pState = pState;
    
    // Draw Card
    auto drawAction = turnStart->CreateAction< DrawCardAction >( ActivePlayer );
    drawAction->TargetCard = deck->At( 0 )->GetEntityId();
    
    // Setup Lua Hook
    SetLuaActionRoot( &Queue, drawAction );
    Lua_PopDeck( ActivePlayer ); // Ensure lua card draws start at proper index
    CallHook( "PreTurn" );
    ClearLuaActionRoot();
    
    // On callback, advance round state
    Queue.Callback = std::bind( &SingleplayerAuthority::Marshal, this );
    GM->RunActionQueue( std::move( Queue ) );

}

void SingleplayerAuthority::Marshal()
{
    if( mState == MatchState::PostMatch )
        return;
    
    cocos2d::log( "[Auth] Marshal Started..." );
    
    // Ensure State Is Updated
    tState = TurnState::Marshal;
    mState = MatchState::Main;
    
    auto GM = GetGameMode< GameModeBase >();
    CC_ASSERT( GM );
    
    auto Queue = ActionQueue();
    auto Update = Queue.CreateAction< EventAction >( GM );
    Update->ActionName = "MarshalStart";
    
    SetLuaActionRoot( &Queue, Update );
    CallHook( "Marshal" );
    ClearLuaActionRoot();
    
    GM->RunActionQueue( std::move( Queue ) );
    
    // Allow player to play cards, once the player is unable to perform
    // any actions, then the state will advance automatically
    
    if( pState == PlayerTurn::Opponent )
    {
        cocos2d::Director::getInstance()->getScheduler()->schedule( [=]( float d )
        {
            // Check if the AI can play a card
            auto Op = GetOpponent();
            auto Hand = Op ? Op->GetHand() : nullptr;
            
            if( !Hand )
                this->Attack();
            
            auto HandCount = Hand->Count();
            for( int i = 0; i < HandCount; i++ )
            {
                auto Card = Hand->At( 0 );
                if( !Card )
                {
                    cocos2d::log( "[DEBUG] AI Marshal Test Failed.. Null Card" );
                    this->Attack();
                    return;
                }
                
                // Check if we have enough mana to play this card
                if( Op->GetMana() < Card->ManaCost )
                {
                    cocos2d::log( "[DEBUG] AI Marshal.. Ran out of mana!" );
                    this->Attack();
                    return;
                }
                
                if( Hand->Count() == 1  )
                {
                    this->AI_PlayCard( Card, std::bind( &SingleplayerAuthority::Attack, this ) );
                }
                else
                {
                    this->AI_PlayCard( Card, nullptr );
                }
            
            }
            
        }, this, 1.f, 0, 0.f, false, "DEBUG_AIPlayCard" );
    }
    
}

void SingleplayerAuthority::Attack()
{
    if( mState == MatchState::PostMatch )
        return;
    
    cocos2d::log( "[Auth] Attack Started" );
    
    // Update State
    tState = TurnState::Attack;
    mState = MatchState::Main;
    
    auto GM = GetGameMode< GameModeBase >();
    CC_ASSERT( GM );
    
    auto Queue = ActionQueue();
    auto Update = Queue.CreateAction< EventAction >( GM );
    Update->ActionName = "AttackStart";
    
    SetLuaActionRoot( &Queue, Update );
    CallHook( "AttackStart" );
    ClearLuaActionRoot();
    
    GM->RunActionQueue( std::move( Queue ) );

    
    // Allow player/opponent to select attackers, the player must call FinishTurn
    // to advance the round state
    
    if( pState == PlayerTurn::Opponent )
    {
        // Attack with one card
        cocos2d::Director::getInstance()->getScheduler()->schedule( [=]( float d )
        {
            auto Op = GetOpponent();
            auto Field = Op->GetField();
            
            if( Field )
            {
                std::vector< CardEntity* > Attackers;
                for( auto It = Field->Begin(); It != Field->End(); It++ )
                {
                    Attackers.push_back( *It );
                    break;
                }
                
                AI_SetAttackers( Attackers );
            }
            else
            {
                Block();
            }
        }, this, 1.f, 0, 0.f, false, "DEBUG_AI_Attack" );
    }
}

void SingleplayerAuthority::Block()
{
    if( mState == MatchState::PostMatch )
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
    tState = TurnState::Block;
    mState = MatchState::Main;
    
    auto GM = GetGameMode< GameModeBase >();
    CC_ASSERT( GM );
    
    auto Queue = ActionQueue();
    
    auto Update = Queue.CreateAction< EventAction >( GM );
    Update->ActionName = "BlockStart";
    
    SetLuaActionRoot( &Queue, Update );
    CallHook( "BlockStart" );
    ClearLuaActionRoot();
    
    GM->RunActionQueue( std::move( Queue ) );
    
    // Allow player/opponent to select blockers, the player must call FinishTurn
    // to advance the round state
    
    // DEBUG
    if( pState == PlayerTurn::LocalPlayer )
    {
        cocos2d::Director::getInstance()->getScheduler()->schedule( [=]( float d )
        {
            // Block opponent cards 1:1
            auto Op = GetOpponent();
            auto Field = Op->GetField();
            
            std::deque< CardEntity* > CardList( Field->Begin(), Field->End() );
            std::map< CardEntity*, CardEntity* > Blockers;
            for( auto It = BattleMatrix.begin(); It != BattleMatrix.end(); It++ )
            {
                if( It->first )
                {
                    // Find card to block this one
                    if( CardList.empty() )
                        break;
                    
                    auto Blocker = CardList.front();
                    CardList.pop_front();
                    
                    if( Blocker )
                    {
                        Blockers[ Blocker ] = It->first;
                        GM->SetBlocker( Blocker, It->first );
                        GM->RedrawBlockers();
                    }
                }
            }
            
            cocos2d::Director::getInstance()->getScheduler()->schedule( [=]( float d )
            {
                AI_SetBlockers( Blockers );
                
            }, this, 0.75f, 0, 0.f, false, "DEBUG_AI_Block_Finish" );
            
        }, this, 0.75f, 0, 0.f, false, "DEBUG_AI_Block" );
    }
    
}

void SingleplayerAuthority::Damage()
{
    if( mState == MatchState::PostMatch )
        return;
    
    cocos2d::log( "[Auth] Damage Started" );
    
    // Update State
    tState = TurnState::Damage;
    mState = MatchState::Main;
    
    // Calculate Damage
    if( BattleMatrix.empty() )
    {
        cocos2d::log( "[Auth] No Attackers!" );
            cocos2d::Director::getInstance()->getScheduler()->schedule( [=] ( float d ) { this->PostTurn(); }, this, 0.5f, 0, 0.f, false, "MoveToPost" );
    }
    else
    {
        cocos2d::log( "[Auth] %d Attackers!", (int)BattleMatrix.size() );
        
        auto GM = GetGameMode< GameModeBase >();
        CC_ASSERT( GM );
        
        auto Queue = ActionQueue();
        Action* lastAction = Queue.CreateAction< EventAction >( GM );
        lastAction->ActionName = "DamageStart";
        
        SetLuaActionRoot( &Queue, lastAction );
        CallHook( "DamageStart" );
        ClearLuaActionRoot();
        
        auto TargetPlayer = pState == PlayerTurn::LocalPlayer ? GetOpponent() : GetPlayer();
        
        for( auto It = BattleMatrix.begin(); It != BattleMatrix.end(); It++ )
        {
            if( !It->first )
                continue;
            
            cocos2d::log( "[Auth] %s attacking, being blocked by %d cards", It->first->DisplayName.c_str(), (int)It->second.size() );
            
            auto& Attacker = It->first;
            auto& Blockers = It->second;
            
            uint16_t TotalAttack = Attacker->Power;
            
            // Check if the attack was blocked
            if( Blockers.empty() )
            {
                // Deal damage to enemy king
                
                DamageAction* Damage;
                if( lastAction )
                    Damage = lastAction->CreateAction< DamageAction >( TargetPlayer );
                else
                    Damage = Queue.CreateAction< DamageAction >( TargetPlayer );
                
                Damage->ActionName  = "KingDamage";
                Damage->Damage      = Attacker->Power;
                Damage->Inflictor   = Attacker;
                
                StaminaDrainAction* Stamina;
                if( lastAction )
                    Stamina = lastAction->CreateAction< StaminaDrainAction >( GM );
                else
                    Stamina = Queue.CreateAction< StaminaDrainAction >( GM );
                
                Stamina->Target     = Attacker;
                Stamina->Inflictor  = Attacker;
                Stamina->Amount     = 1;

                lastAction = Damage;
            }
            else
            {
                for( auto It = Blockers.begin(); It != Blockers.end(); It++ )
                {
                    if( ! (*It) )
                        continue;
                    
                    // Deal Damage to this card, maximum of card health, then subtract
                    // this damage from the remaining attack power
                    uint16_t DamageDealt = TotalAttack > (*It)->Power ? (*It)->Power : TotalAttack;
                    
                    // Update remaining attack
                    TotalAttack -= DamageDealt;
                    
                    // Create Damage Action
                    DamageAction* Damage;
                    if( lastAction )
                        Damage = lastAction->CreateAction< DamageAction >( GM );
                    else
                        Damage = Queue.CreateAction< DamageAction >( GM );
                    
                    Damage->ActionName      = "CardDamage";
                    Damage->Target          = *It;
                    Damage->Inflictor       = Attacker;
                    Damage->Damage          = DamageDealt;
                    Damage->StaminaDrain    = 1;
                    
                    DamageAction* Blowback;
                    if( lastAction )
                        Blowback = lastAction->CreateAction< DamageAction >( GM );
                    else
                        Blowback = Queue.CreateAction< DamageAction >( GM );
                    
                    Blowback->ActionName    = "CardDamage";
                    Blowback->Target        = Attacker;
                    Blowback->Inflictor     = *It;
                    Blowback->Damage        = DamageDealt;
                    Blowback->StaminaDrain  = 1;
                    
                    lastAction = Damage;
                    
                    // Check if were finished dealing damage
                    if( TotalAttack <= 0 )
                        break;
                }
            }
        }
        
        SetLuaActionRoot( &Queue, lastAction );
        CallHook( "DamageFinish" );
        auto finalAction = Lua_LastSerial ? Lua_LastSerial : lastAction;
        ClearLuaActionRoot();
        
        // Add cleanup board event
        auto Cleanup = finalAction->CreateAction< EventAction >( GM );
        Cleanup->ActionName = "CleanupBoard";
        
        Queue.Callback = std::bind( &SingleplayerAuthority::PostTurn, this );
        GM->RunActionQueue( std::move( Queue ) );
    }
    
    // Clear Battle Matrix
    BattleMatrix.clear();
    
}



void SingleplayerAuthority::AI_PlayCard( CardEntity* In, std::function< void() > Callback )
{
    if( mState == MatchState::PostMatch )
        return;
    
    auto GM = GetGameMode< SingleplayerGameMode >();
    auto Op = GetOpponent();
    
    CC_ASSERT( GM && Op );
    
    auto Queue = ActionQueue();
    if( Callback )
        Queue.Callback = Callback;
    
    auto playCard = Queue.CreateAction< PlayCardAction >( Op );
    
    playCard->bNeedsMove = true;
    playCard->bWasSuccessful = false;
    playCard->TargetCard = In->GetEntityId();
    playCard->TargetIndex = 10000;
    
    if( !In )
    {
        cocos2d::log( "[Authority] AI attempted to play null card!" );
    }
    else if( tState != TurnState::Marshal || mState != MatchState::Main || pState != PlayerTurn::Opponent )
    {
        cocos2d::log( "[Authority] AI attempted to play card outside of proper turn state!" );
    }
    else if( !In->InHand() || In->GetOwningPlayer() != Op )
    {
        cocos2d::log( "[Authority] AI attempted to play a card not in the AI's hand!" );
    }
    else if( In->ManaCost > Op->GetMana() )
    {
        cocos2d::log( "[Authority] AI attempted to play a card but doesnt have enough mana!" );
    }
    else
    {
        // Request appears to be valid
        auto manaAction = Queue.CreateAction< UpdateManaAction >( Op );
        manaAction->UpdatedMana = Op->GetMana() - In->ManaCost;
        
        playCard->bWasSuccessful = true;
    }
    
    SetLuaActionRoot( &Queue, playCard );
    CallHook( "PlayCard", GetOpponent(), In );
    ClearLuaActionRoot();
    
    GM->RunActionQueue( std::move( Queue ) );
}

void SingleplayerAuthority::PlayCard( CardEntity* card, int Index )
{
    if( mState == MatchState::PostMatch )
        return;
    
    auto GM = GetGameMode< SingleplayerGameMode >();
    Player* LocalPlayer = GetPlayer();
    
    CC_ASSERT( GM );
    CC_ASSERT( LocalPlayer );
    
    auto Queue = ActionQueue();
    auto* playCard = Queue.CreateAction< PlayCardAction >( LocalPlayer );
    
    playCard->bNeedsMove = true;
    playCard->bWasSuccessful = false;
    playCard->TargetCard = card->GetEntityId();
    playCard->TargetIndex = Index;
    
    if( !card )
    {
        cocos2d::log( "[Authority] Failed to play card! Called with null!" );
        
    } // Check if the card is in the players hand
    else if( card->GetOwningPlayer() != LocalPlayer || !card->InHand() )
    {
        cocos2d::log( "[Authority] Attempt to play card thats not in the players hand!" );

    } // Check if the player has enough mana
    else if( card->ManaCost > LocalPlayer->GetMana() )
    {
        cocos2d::log( "[Authority] Failed to play card.. not enough mana" );
        
    }
    else if( tState != TurnState::Marshal || mState != MatchState::Main || pState != PlayerTurn::LocalPlayer )
    {
        cocos2d::log( "[Authority] Player attempted to play a card outside of proper turn state!" );
    }
    else
    {
        // Request is valid, Build action queue
        // This might seem overcomplicated, but were attempting to make multiplayer and singleplayer
        // as close as possible, so the method to update state and run animations is the same
        
        auto* manaAction = Queue.CreateAction< UpdateManaAction >( LocalPlayer );
        manaAction->UpdatedMana = LocalPlayer->GetMana() - card->ManaCost;
        
        playCard->bWasSuccessful = true;
    }
    
    SetLuaActionRoot( &Queue, playCard );
    CallHook( "PlayCard", GetPlayer(), card );
    ClearLuaActionRoot();
    
    GM->RunActionQueue( std::move( Queue ) );
}

void SingleplayerAuthority::PostTurn()
{
    if( mState == MatchState::PostMatch )
        return;
    
    cocos2d::log( "[Auth] Post Turn Started..." );
    
    mState = MatchState::Main;
    tState = TurnState::PostTurn;
  
    auto GM = GetGameMode< GameModeBase >();
    CC_ASSERT( GM );
    
    auto Queue = ActionQueue();
    auto Update = Queue.CreateAction< EventAction >( GM );
    Update->ActionName = "PostTurnStart";
    
    // TODO: Any Cleanup/Hooks?
    
    // On action queue complete, next turn starts
    PlayerTurn nextTurn = pState == PlayerTurn::LocalPlayer ? PlayerTurn::Opponent : PlayerTurn::LocalPlayer;
    Queue.Callback = std::bind( &SingleplayerAuthority::PreTurn, this, nextTurn );
    
    SetLuaActionRoot( &Queue, Update );
    CallHook( "PostTurn" );
    ClearLuaActionRoot();
    
    GM->RunActionQueue( std::move( Queue ) );
    
}


void SingleplayerAuthority::FinishTurn()
{
    // Ensure were in the main match state
    if( mState != MatchState::Main )
    {
        cocos2d::log( "[Auth] 'FinishTurn' called outside of main match state!" );
        return;
    }
    
    if( pState == PlayerTurn::LocalPlayer )
    {
        if( tState == TurnState::Marshal )
        {
            // Advance state to 'Attack'
            Attack();
        }
        else if( tState == TurnState::Attack )
        {
            // Advance state to 'block'
            Block();
        }
        else
        {
            cocos2d::log( "[Auth] 'FinishTurn' called outside of Marshal/Attack!" );
        }
    }
    else
    {
        if( tState == TurnState::Block )
        {
            // Advance state to 'damage'
            Damage();
        }
        else
        {
            cocos2d::log( "[Auth] 'FinishTurn' called outside of player's turn!" );
        }
    }
}

void SingleplayerAuthority::AI_SetAttackers( const std::vector< CardEntity* >& In )
{
    if( pState != PlayerTurn::Opponent || mState != MatchState::Main || tState != TurnState::Attack )
    {
        cocos2d::log( "[AI] Attempt to set attackers outside of proper round state!" );
        Block();
        return;
    }
    
    BattleMatrix.clear();
    
    for( auto It = In.begin(); It != In.end(); It++ )
    {
        if( !(*It) )
        {
            cocos2d::log( "[AI] Null card in attacker list!" );
            continue;
        }
        
        if( (*It)->GetOwningPlayer() != GetOpponent() || !(*It)->OnField() )
        {
            cocos2d::log( "[AI] Attacker not on field!" );
            continue;
        }
        
        if( BattleMatrix.count( *It ) > 0 )
        {
            cocos2d::log( "[AI] Duplicate attackers found!" );
            continue;
        }
        
        BattleMatrix.insert( std::make_pair( *It, std::vector< CardEntity* >() ) );
        
        (*It)->SetHighlight( cocos2d::Color3B( 240, 30, 30 ), 180 );
        (*It)->SetOverlay( "icon_attack.png", 180 );
        (*It)->bAttacking = true;
    }
    
    auto Engine = Regicide::LuaEngine::GetInstance();
    auto Table = luabridge::newTable( Engine->State() );
    
    int Index = 1;
    for( auto It = BattleMatrix.begin(); It != BattleMatrix.end(); It++ )
    {
        if( It->first )
            Table[ Index++ ] = It->first;
    }
    
    SetLuaActionRoot( nullptr );
    CallHook( "OnAttackers", Table );
    ClearLuaActionRoot();
    
    Block();
}

void SingleplayerAuthority::SetAttackers( const std::vector<CardEntity *> &Cards )
{
    if( pState != PlayerTurn::LocalPlayer || mState != MatchState::Main || tState != TurnState::Attack )
    {
        cocos2d::log( "[Auth] Attempt to set attackers outside of player attack phase!" );
        return;
    }
    
    BattleMatrix.clear();
    std::vector< CardEntity* > Errors;
    
    // We need to validate the cards, if theres any errors, we need to alert
    // the gamemode of the issue.
    for( auto It = Cards.begin(); It != Cards.end(); It++ )
    {
        // Check for null entries
        if( !(*It ) )
        {
            Errors.push_back( nullptr );
            continue;
        }
        
        // Ensure cards are owned by player, and in field
        if( (*It)->GetOwningPlayer() != GetPlayer() || !(*It)->OnField() )
        {
            Errors.push_back( *It );
            continue;
        }
        
        if( (*It)->Stamina <= 0 )
        {
            Errors.push_back( *It );
            continue;
        }
        
        // Check for duplicates
        if( BattleMatrix.count( *It ) > 0 )
        {
            continue;
        }
    }
    
    if( Errors.size() > 0 )
    {
        cocos2d::log( "[Auth] Warning: Invalid Attackers Selected!" );
        auto GM = GetGameMode< GameModeBase >();
        
        auto Queue = ActionQueue();
        auto errEvent = Queue.CreateAction< CardErrorAction >( GM );
        errEvent->ActionName = "AttackError";
        
        GM->RunActionQueue( std::move( Queue ) );
        return;
    }
    
    cocos2d::log( "[Auth] Setting up BattleMatrix" );
    
    // Setup battle matrix with player selection
    for( auto It = Cards.begin(); It != Cards.end(); It++ )
        BattleMatrix.insert( std::make_pair( *It, std::vector< CardEntity* >() ) );
    
    // TODO: Inform Players of selection
    
    auto Engine = Regicide::LuaEngine::GetInstance();
    auto Table = luabridge::newTable( Engine->State() );
    
    int Index = 1;
    for( auto It = BattleMatrix.begin(); It != BattleMatrix.end(); It++ )
    {
        if( It->first )
            Table[ Index++ ] = It->first;
    }
    
    SetLuaActionRoot( nullptr );
    CallHook( "OnAttackers", Table );
    ClearLuaActionRoot();
    
    // Advance Round State
    FinishTurn();
}


void SingleplayerAuthority::AI_SetBlockers( const std::map<CardEntity *, CardEntity *> &Cards )
{
    if( pState != PlayerTurn::LocalPlayer || mState != MatchState::Main || tState != TurnState::Block )
    {
        cocos2d::log( "[AI] Attempt to set blockers outside of opponent block phase!" );
        Damage();
        return;
    }
    
    for( auto It = Cards.begin(); It != Cards.end(); It++ )
    {
        // Check for null cards
        if( !( It->first ) )
        {
            cocos2d::log( "[AI] Bad Blocker.. Null Blocker!" );
            continue;
        }
        else if( !( It->second ) )
        {
            cocos2d::log( "[AI] Bad Blocker.. Null Attacker!" );
            continue;
        }
        
        //  Ensure blocking card is valid
        if( It->first->GetOwningPlayer() != GetOpponent() || !It->first->OnField() )
        {
            cocos2d::log( "[AI] Bad Blocker.. In bad position!" );
            continue;
        }
        
        // Ensure attacking card is valid
        if( BattleMatrix.count( It->second ) <= 0 )
        {
            cocos2d::log( "[AI] Bad Blocker.. Invalid Attacker!" );
            continue;
        }
        
        // Add to battle matrix
        BattleMatrix[ It->second ].push_back( It->first );
    }
    
    auto Engine = Regicide::LuaEngine::GetInstance();
    auto Table = luabridge::newTable( Engine->State() );
    
    for( auto It = BattleMatrix.begin(); It != BattleMatrix.end(); It++ )
    {
        if( It->first )
        {
            auto blockTable = luabridge::newTable( Engine->State() );
            
            if( It->second.size() > 0 )
            {
                int secondIndex = 1;
                for( auto bIt = It->second.begin(); bIt != It->second.end(); bIt++ )
                {
                    if( *bIt )
                        blockTable[ secondIndex ] = *bIt;
                }
            }
            
            Table[ It->first ] = blockTable;
        }
    }
    
    SetLuaActionRoot( nullptr );
    CallHook( "OnBlockers", Table );
    ClearLuaActionRoot();
    
    Damage();
}

void SingleplayerAuthority::SetBlockers( const std::map<CardEntity *, CardEntity *> &Cards )
{
    if( pState != PlayerTurn::Opponent || mState != MatchState::Main || tState != TurnState::Block )
    {
        cocos2d::log( "[Auth] Attempt to set blockers outside of opponent block phase!" );
        return;
    }
    
    // Validate Blockers
    std::vector< CardEntity* > Errors;
    for( auto It = Cards.begin(); It != Cards.end(); It++ )
    {
        // Check for null cards
        if( !( It->first ) )
        {
            Errors.push_back( nullptr );
            continue;
        }
        else if( !( It->second ) )
        {
            Errors.push_back( It->first );
            continue;
        }
        
        //  Ensure blocking card is valid
        if( It->first->GetOwningPlayer() != GetPlayer() || !It->first->OnField() )
        {
            Errors.push_back( It->first );
            continue;
        }
        
        if( It->first->Stamina <= 0 )
        {
            Errors.push_back( It->first );
            continue;
        }
        
        // Ensure attacking card is valid
        if( BattleMatrix.count( It->second ) <= 0 )
        {
            Errors.push_back( It->first );
            continue;
        }
        
        // Add to battle matrix
        BattleMatrix[ It->second ].push_back( It->first );
    }
    
    if( Errors.size() > 0 )
    {
        cocos2d::log( "[Auth] Failed to set blockers!" );
        
        // TODO: Build Error Action Queue
        
        return;
    }
    
    auto Engine = Regicide::LuaEngine::GetInstance();
    auto Table = luabridge::newTable( Engine->State() );
    
    for( auto It = BattleMatrix.begin(); It != BattleMatrix.end(); It++ )
    {
        if( It->first )
        {
            auto blockTable = luabridge::newTable( Engine->State() );
            
            if( It->second.size() > 0 )
            {
                int secondIndex = 1;
                for( auto bIt = It->second.begin(); bIt != It->second.end(); bIt++ )
                {
                    if( *bIt )
                        blockTable[ secondIndex ] = *bIt;
                }
            }
            
            Table[ It->first ] = blockTable;
        }
    }
    
    SetLuaActionRoot( nullptr );
    CallHook( "OnBlockers", Table );
    ClearLuaActionRoot();
    
    FinishTurn();
}

void SingleplayerAuthority::TriggerAbility( CardEntity *Card, uint8_t AbilityId )
{
    auto Player = GetPlayer();
    auto GM = GetGameMode< GameModeBase >();
    
    if( !Card || !Player || !GM )
    {
        // TODO: Respond with error
        cocos2d::log( "[Auth] Failed to trigger ability. No Card/Player/GM" );
        return;
    }
    
    // Check if ability exists
    if( Card->Abilities.count( AbilityId ) <= 0 )
    {
        // TODO: Respond with error
        cocos2d::log( "[Auth] Failed to trigger ability. Invalid Ability" );
        return;
    }
    
    auto& Ability = Card->Abilities.at( AbilityId );
    
    if( Ability.ManaCost > Player->GetMana() || Ability.StaminaCost > Card->Stamina )
    {
        // TODO: Respond with error
        cocos2d::log( "[Auth] Failed to trigger ability. Not enough mana/stamina" );
        return;
    }
    
    if( Ability.CheckFunc && Ability.CheckFunc->isFunction() )
    {
        if( !( *Ability.CheckFunc )( Card ) )
        {
            // TODO: Respond with error
            cocos2d::log( "[Auth] Failed to trigger ability. CheckFunc failed" );
            return;
        }
    }
    
    // Build Action Queue
    auto Queue = ActionQueue();
    auto ManaAction = Queue.CreateAction< UpdateManaAction >( Player );
    ManaAction->UpdatedMana = Player->GetMana() - Ability.ManaCost;
    auto StaminaAction = Queue.CreateAction< StaminaDrainAction >( GM );
    StaminaAction->Target = Card;
    StaminaAction->Inflictor = Card;
    StaminaAction->Amount = Ability.StaminaCost;
    
    SetLuaActionRoot( &Queue );
    
    try
    {
    ( *Ability.MainFunc )( Card );
    }
    catch( std::exception& e )
    {
        cocos2d::log( "[Lua] Error! %s", e.what() );
    }
    
    // TODO: Call Hook?
    ClearLuaActionRoot();
    
    GM->RunActionQueue( std::move( Queue ) );
    cocos2d::log( "[Auth] Triggered Ability. " );
}


void SingleplayerAuthority::OnGameWon( Player* Winner )
{
    // Stop all timers
    cocos2d::Director::getInstance()->getScheduler()->unscheduleAllForTarget( this );
    auto GM = GetGameMode< SingleplayerGameMode >();
    
    auto Queue = ActionQueue();
    auto Win = Queue.CreateAction< WinAction >( GM );
    
    if( Winner == GetPlayer() )
    {
        cocos2d::log( "[Auth] Local Player Won!" );
        Win->bDidWin = true;
    }
    else
    {
        cocos2d::log( "[Auth] Opponent Won!" );
        Win->bDidWin = false;
    }
    
    mState = MatchState::PostMatch;
    GM->RunActionQueue( std::move( Queue ) );
}

void SingleplayerAuthority::Tick( float Delta )
{
    if( mState == MatchState::Main )
    {
        // Check if either player died, stop all timers/callbacks and send the win signal
        auto Pl = GetPlayer();
        auto Op = GetOpponent();
        
        if( !Pl || Pl->GetHealth() <= 0 )
        {
            OnGameWon( Op );
        }
        else if( !Op || Op->GetHealth() <= 0 )
        {
            OnGameWon( Pl );
        }
    }
}

uint32_t SingleplayerAuthority::Lua_PopDeck( Player *In )
{
    if( !In )
        return 0;
    
    if( In->IsOpponent() )
        return OpponentDeckIndex++;
    else
        return OwnerDeckIndex++;
}

// Set the index for lua draw actions. If we draw a card in c++, and call a lua hook, we need to
// be able to track which index to draw the next card from, so we dont draw the same card twice
void SingleplayerAuthority::SetLuaDeckIndex( Player *Target, int Index )
{
    if( Target )
    {
        if( Target->IsOpponent() )
            OpponentDeckIndex = Index;
        else
            OwnerDeckIndex = Index;
    }
}

void SingleplayerAuthority::SetLuaActionRoot( ActionQueue* RootQueue, Action* RootAction /* = nullptr */ )
{
    Lua_RootQueue       = RootQueue;
    Lua_RootAction      = RootAction;
    Lua_LastSerial      = nullptr;
    Lua_LastParallel    = nullptr;
    OwnerDeckIndex      = 0;
    OpponentDeckIndex   = 0;
}

void SingleplayerAuthority::ClearLuaActionRoot()
{
    Lua_RootQueue       = nullptr;
    Lua_RootAction      = nullptr;
    Lua_LastSerial      = nullptr;
    Lua_LastParallel    = nullptr;
    OwnerDeckIndex      = 0;
    OpponentDeckIndex   = 0;
}

void SingleplayerAuthority::CallHook( const std::string& In )
{
    auto& Ent = IEntityManager::GetInstance();
    auto lua = Regicide::LuaEngine::GetInstance();
    
    if( !lua )
        return;
    
    // Check if theres an action queue set for lua to use
    // If not, then we will just make our own for local use
    auto Queue = ActionQueue();
    bool bUseLocal = false;
    
    if( !Lua_RootQueue )
    {
        SetLuaActionRoot( &Queue );
        bUseLocal = true;
    }
    
    auto Player = GetPlayer();
    auto Opponent = GetOpponent();
    
    if( Player )
    {
        auto King = Player->GetKing();
        luabridge::LuaRef Func( lua->State() );
        if( King && King->GetHook( In, Func ) )
        {
            try
            {
                Func( King );
            }
            catch( std::exception& e )
            {
                cocos2d::log( "[Lua] Error! %s", e.what() );
            }
        }
    }
    
    if( Opponent )
    {
        auto King = Opponent->GetKing();
        luabridge::LuaRef Func( lua->State() );
        if( King && King->GetHook( In, Func ) )
        {
            try
            {
                Func( King );
            }
            catch( std::exception& e )
            {
                cocos2d::log( "[Lua] Error! %s", e.what() );
            }
        }
    }
    
    for( auto It = Ent.Begin(); It != Ent.End(); It++ )
    {
        if( It->second && It->second->IsCard() )
        {
            auto Card = dynamic_cast< CardEntity* >( It->second.get() );
            if( Card && Card->ShouldCallHook( In ) )
            {
                luabridge::LuaRef Func( lua->State() );
                if( Card->GetHook( In, Func ) )
                {
                    try
                    {
                        Func( Card );
                    }
                    catch( std::exception& e )
                    {
                        cocos2d::log( "[Lua] Error! %s", e.what() );
                    }
                }
            }
        }
    }
    
    // If we created our own queue, then run it
    if( bUseLocal && Queue.ActionTree.size() > 0 )
    {
        auto GM = Game::World::GetWorld()->GetGameMode< GameModeBase >();
        CC_ASSERT( GM );
        
        ClearLuaActionRoot();
        GM->RunActionQueue( std::move( Queue ) );
    }
}
