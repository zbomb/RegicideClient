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
    
    // Setup Tick
    cocos2d::Director::getInstance()->getScheduler()->schedule( std::bind( &SingleplayerAuthority::Tick, this, _1 ), this, 0.f, CC_REPEAT_FOREVER, 0.f, false, "SingleplayerAuthTick" );
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
        // TODO: Win Game Action
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
    
    GM->RunActionQueue( std::move( Queue ) );
    
    // Allow player/opponent to select attackers, the player must call FinishTurn
    // to advance the round state
    
    if( pState == PlayerTurn::Opponent )
    {
        // Attack with everything
        /*
        auto Op = GetOpponent();
        auto Field = Op->GetField();
        
        if( Field )
        {
            std::vector< CardEntity* > Attackers;
            for( auto It = Field->Begin(); It != Field->End(); It++ )
            {
                Attackers.push_back( *It );
            }
            
            AI_SetAttackers( Attackers );
        }
         */
        
        Block();
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
    
    GM->RunActionQueue( std::move( Queue ) );
    
    // Allow player/opponent to select blockers, the player must call FinishTurn
    // to advance the round state
    
    // DEBUG
    if( pState == PlayerTurn::LocalPlayer )
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
                }
            }
        }
        
        AI_SetBlockers( Blockers );
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
        
        auto TargetPlayer = pState == PlayerTurn::LocalPlayer ? GetOpponent() : GetPlayer();
        int FinalKingHealth = TargetPlayer->GetHealth();
        
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
                
                FinalKingHealth -= Attacker->Power;
                
                Damage->ActionName = "KingDamage";
                Damage->Amount = Attacker->Power;
                Damage->TargetPower = FinalKingHealth;
                Damage->InflictorPower = Attacker->Power;
                Damage->Inflictor = Attacker;

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
                    uint16_t Remaining = (*It)->Power - DamageDealt;
                    
                    // Update remaining attack
                    TotalAttack -= DamageDealt;
                    
                    // Create Damage Action
                    DamageAction* Damage;
                    if( lastAction )
                        Damage = lastAction->CreateAction< DamageAction >( GM );
                    else
                        Damage = Queue.CreateAction< DamageAction >( GM );
                    
                    Damage->ActionName = "CardDamage";
                    Damage->Target = *It;
                    Damage->Inflictor = Attacker;
                    Damage->Amount = DamageDealt;
                    Damage->TargetPower = Remaining;
                    Damage->InflictorPower = TotalAttack;
                    
                    lastAction = Damage;
                    
                    // Check if were finished dealing damage
                    if( TotalAttack <= 0 )
                        break;
                }
            }
        }
        
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
    }
    
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
        
        // TODO: Other reasons a card couldnt attack?
        
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
    
    FinishTurn();
}

void SingleplayerAuthority::TriggerAbility( CardEntity *Card, uint8_t AbilityId )
{
    
}

bool SingleplayerAuthority::DrawCard( Player *In, uint32_t Count )
{
    if( mState == MatchState::PostMatch )
        return false;
    
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
