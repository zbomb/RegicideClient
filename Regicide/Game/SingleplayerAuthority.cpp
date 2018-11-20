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

#define GAME_INITDRAW_COUNT 10

using namespace Game;


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
}

void SingleplayerAuthority::SceneInit( cocos2d::Scene *inScene )
{
    EntityBase::SceneInit( inScene );
    
    cocos2d::Director::getInstance()->getScheduler()->schedule( CC_CALLBACK_1( SingleplayerAuthority::Test, this ), this, 2.f, 0, 0.f, false, "test" );
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
    int RandValue = cocos2d::random( 0, 2 );
    if( RandValue <= 0 )
        pState = PlayerTurn::LocalPlayer;
    else
        pState = PlayerTurn::Opponent;
    
    // Inform GameMode
    GameModeBase* GM = GetGameMode< GameModeBase >();
    CC_ASSERT( GM );
    
    auto Queue = ActionQueue();
    Queue.Callback = std::bind( &SingleplayerAuthority::CoinFlipFinish, this );
    
    auto coinFlip = Queue.CreateAction< CoinFlipAction >( GM );
    coinFlip->StartingPlayer = pState;
    
    GM->RunActionQueue( std::move( Queue ) );
}

void SingleplayerAuthority::CoinFlipFinish()
{
    cocos2d::log( "[DEBUG] FINISH COIN FLIP!" );
    
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
        }
        
        auto opCard = OpDeck->At( i );
        if( opCard )
        {
            auto opDraw = lastOpponentAction->CreateAction< DrawCardAction >( Op );
            opDraw->TargetCard = opCard->GetEntityId();
            
            lastOpponentAction = opDraw;
        }
    }
    
    // Create action to query blitz cards from the player
    auto blitzQuery = lastPlayerAction->CreateAction< TimedQueryAction >( GM );
    blitzQuery->ActionName = "BlitzQuery";
    blitzQuery->Deadline = std::chrono::steady_clock::now() + std::chrono::seconds( 25 );
    
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
        cocos2d::log( "[Auth] CALLING BLITZ ERROR!" );
        auto GM = GetGameMode< SingleplayerGameMode >();
        CC_ASSERT( GM );
        
        auto Queue = ActionQueue();
        auto errEvent = Queue.CreateAction< BlitzErrorAction >( GM );
        errEvent->Errors = Errors;
        
        GM->RunActionQueue( std::move( Queue ) );
    }
    else
    {
        cocos2d::log( "[Auth] CALLING FINISH BLITZ!!!" );
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
    // Update State
    pState = pTurn;
    mState = MatchState::Main;
    tState = TurnState::PreTurn;
    
    // Perform Draw
    auto GM = GetGameMode< GameModeBase >();
    auto ActivePlayer = CurrentTurnPlayer();
    CC_ASSERT( ActivePlayer && GM );
    
    auto Queue = ActionQueue();
    auto deck = ActivePlayer->GetDeck();
    
    if( !deck || deck->Count() <= 0 || !deck->At( 0 ) )
    {
        cocos2d::log( "[Auth] Player ran out of cards!" );
        // TODO: Win Game Action
        return;
    }
    
    auto turnStart = Queue.CreateAction< TurnStartAction >( GM );
    turnStart->pState = pState;
    
    auto drawAction = turnStart->CreateAction< DrawCardAction >( ActivePlayer );
    drawAction->TargetCard = deck->At( 0 )->GetEntityId();
    
    // On callback, advance round state
    Queue.Callback = std::bind( &SingleplayerAuthority::Marshal, this );
    GM->RunActionQueue( std::move( Queue ) );
}

void SingleplayerAuthority::Marshal()
{
    cocos2d::log( "[Auth] Marshal Started..." );
    
    // Ensure State Is Updated
    tState = TurnState::Marshal;
    mState = MatchState::Main;
    
    auto GM = GetGameMode< GameModeBase >();
    CC_ASSERT( GM );
    
    auto Queue = ActionQueue();
    auto Update = Queue.CreateAction< EventAction >( GM );
    Update->ActionName = "MarshalStart";
    
    GM->RunActionQueue( std::move( Queue ) );
    
    // Allow player to play cards, once the player is unable to perform
    // any actions, then the state will advance automatically
    
    // TODO: Check if player is able to play cards, or trigger abilities
    
}

void SingleplayerAuthority::Attack()
{
    cocos2d::log( "[Auth] Attack Started" );
    
    // Update State
    tState = TurnState::Attack;
    mState = MatchState::Main;
    
    auto GM = GetGameMode< GameModeBase >();
    CC_ASSERT( GM );
    
    auto Queue = ActionQueue();
    auto Update = Queue.CreateAction< EventAction >( GM );
    Update->ActionName = "AttackStart";
    
    GM->RunActionQueue( std::move( Queue ) );
    
    // Allow player/opponent to select attackers, the player must call FinishTurn
    // to advance the round state
}

void SingleplayerAuthority::Block()
{
    cocos2d::log( "[Auth] Block Started" );
    
    // Update State
    tState = TurnState::Block;
    mState = MatchState::Main;
    
    auto GM = GetGameMode< GameModeBase >();
    CC_ASSERT( GM );
    
    auto Queue = ActionQueue();
    auto Update = Queue.CreateAction< EventAction >( GM );
    Update->ActionName = "BlockStart";
    
    GM->RunActionQueue( std::move( Queue ) );
    
    // Allow player/opponent to select blockers, the player must call FinishTurn
    // to advance the round state
    
    // DEBUG
    if( pState == PlayerTurn::LocalPlayer )
    {
        cocos2d::Director::getInstance()->getScheduler()->schedule( [=] ( float d ){ this->Damage(); }, this, 2.f, 0, 0.f, false, "DebugMoveToDamage" );
    }
}

void SingleplayerAuthority::Damage()
{
    cocos2d::log( "[Auth] Damage Started" );
    
    // Update State
    tState = TurnState::Damage;
    mState = MatchState::Main;
    
    // DEBUG
    cocos2d::Director::getInstance()->getScheduler()->schedule( [=] ( float d ) { this->PostTurn(); }, this, 2.f, 0, 0.f, false, "DebugMoveToPost" );
}

void SingleplayerAuthority::PlayCard( CardEntity* card, int Index )
{
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
    else
    {
        // Request is valid, Build action queue
        // This might seem overcomplicated, but were attempting to make multiplayer and singleplayer
        // as close as possible, so the method to update state and run animations is the same
        
        auto* manaAction = Queue.CreateAction< UpdateManaAction >( LocalPlayer );
        manaAction->UpdatedMana = LocalPlayer->GetMana() - card->ManaCost;
        
        playCard->bWasSuccessful = true;
    }
    
    GM->RunActionQueue( std::move( Queue ) );
}

void SingleplayerAuthority::PostTurn()
{
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

void SingleplayerAuthority::SetAttackers( const std::vector<CardEntity *> &Cards )
{
    if( pState != PlayerTurn::LocalPlayer || mState != MatchState::Main || tState != TurnState::Attack )
    {
        cocos2d::log( "[Auth] Attempt to set attackers outside of player attack phase!" );
        // TODO: Send Error
        return;
    }
    
    BattleMatrix.clear();
    
    // We need to validate the cards, if theres any errors, we need to alert
    // the gamemode of the issue.
    std::vector< CardEntity* > Errors;
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
        
        // TODO: Check if card is able to attack
    }
    
    if( Errors.size() > 0 )
    {
        // Respond to player with errors
        return;
    }
    
    // Setup battle matrix with player selection
    for( auto It = Cards.begin(); It != Cards.end(); It++ )
        BattleMatrix.insert( std::make_pair( *It, std::vector< CardEntity* >() ) );
    
    // Advance Round State
    FinishTurn();
}

void SingleplayerAuthority::SetBlockers( const std::map<CardEntity *, CardEntity *> &Cards )
{
    if( pState != PlayerTurn::Opponent || mState != MatchState::Main || tState != TurnState::Block )
    {
        cocos2d::log( "[Auth] Attempt to set blockers outside of opponent block phase!" );
        
        // TODO: Send error to gamemdoe
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
        
        // TODO: Send error
        return;
    }
    
    FinishTurn();
}

void SingleplayerAuthority::TriggerAbility( CardEntity *Card, uint8_t AbilityId )
{
    
}

bool SingleplayerAuthority::DrawCard( Player *In, uint32_t Count )
{
    // Return false if invalid player, or no cards left to draw
    if( !In )
        return false;
    
    auto Deck = In->GetDeck();
    if( !Deck || Deck->Count() < Count )
        return false;

    
    // Build action queue to draw this card
    auto Queue = ActionQueue();
    Game::Action* lastAction = nullptr;
    
    for( int i = 0; i < Count; i++ )
    {
        auto tarCard = Deck->At( i );
        if( !tarCard )
            return false;
        
        DrawCardAction* drawAction;
        if( lastAction )
        {
            drawAction = lastAction->CreateAction< DrawCardAction >( In );
        }
        else
        {
            drawAction = Queue.CreateAction< DrawCardAction >( In );
        }
        
        lastAction = drawAction;
        drawAction->TargetCard = tarCard->GetEntityId();
    }
    
    auto GM = GetGameMode< GameModeBase >();
    CC_ASSERT( GM );
    
    GM->RunActionQueue( std::move( Queue ) );
    return true;
    
}

void SingleplayerAuthority::Test( float Delta )
{

}
