//
//  SingleplayerAuthority.cpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/11/18.
//

#include "SingleplayerAuthority.hpp"
#include "Player.hpp"
#include "DeckEntity.hpp"
#include "HandEntity.hpp"
#include "FieldEntity.hpp"
#include "GraveyardEntity.hpp"

#define GAME_INITDRAW_COUNT 10

using namespace Game;


SingleplayerAuthority::~SingleplayerAuthority()
{
    LocalPlayer     = nullptr;
    Opponent        = nullptr;
}

void SingleplayerAuthority::Cleanup()
{
    EntityBase::Cleanup();
    
    cocos2d::Director::getInstance()->getScheduler()->unscheduleUpdate( this );
    cocos2d::Director::getInstance()->getScheduler()->unschedule( "StartTimer", this );
}

void SingleplayerAuthority::PostInit()
{
    // Call base class method
    EntityBase::PostInit();
    
    CC_ASSERT( LocalPlayer && Opponent );
    
    auto dir = cocos2d::Director::getInstance();
    auto Origin = dir->getVisibleOrigin();
    auto Size = dir->getVisibleSize();
    
    // Setup field
    // Set player object location
    LocalPlayer->SetPosition( cocos2d::Vec2( Origin.x + Size.width / 2.f, Origin.y + Size.height * 0.f ) );
    Opponent->SetPosition( cocos2d::Vec2( Origin.x + Size.width / 2.f, Origin.y + Size.height * 1.f ) );
    
    // Set locations of decks
    auto PlayerDeck = LocalPlayer->GetDeck();
    auto OpponentDeck = Opponent->GetDeck();
    
    PlayerDeck->SetPosition( cocos2d::Vec2( Size.width * 0.4f, Size.height * 0.25f ) );
    PlayerDeck->SetRotation( 0.f );
    
    OpponentDeck->SetPosition( cocos2d::Vec2( Size.width * 0.4f, -Size.height * 0.25f ) );
    OpponentDeck->SetRotation( 180.f );
    
    // Invalidate deck Z-Order
    PlayerDeck->InvalidateZOrder();
    OpponentDeck->InvalidateZOrder();
    
    // Set hand positions
    auto PlayerHand = LocalPlayer->GetHand();
    auto OpponentHand = Opponent->GetHand();
    
    PlayerHand->SetPosition( cocos2d::Vec2( 0.f, 0.f) );
    PlayerHand->SetRotation( 0.f );
    OpponentHand->SetPosition( cocos2d::Vec2( 0.f, 0.f ) );
    OpponentHand->SetRotation( 180.f );
    
    auto PlayerField = LocalPlayer->GetField();
    auto OpponentField = Opponent->GetField();
    
    PlayerField->SetPosition( cocos2d::Vec2( 0.f, Size.height * 0.3f ) );
    PlayerField->SetRotation( 0.f );
    OpponentField->SetPosition( cocos2d::Vec2( 0.f, -Size.height * 0.3f ) );
    OpponentField->SetRotation( 180.f );
    
    auto PlayerGrave = LocalPlayer->GetGraveyard();
    auto OpponentGrave = Opponent->GetGraveyard();
    
    PlayerGrave->SetPosition( cocos2d::Vec2( -Size.width * 0.4f, Size.height * 0.25f ) );
    PlayerGrave->SetRotation( 0.f );
    OpponentGrave->SetPosition( cocos2d::Vec2( -Size.width * 0.35f, -Size.height * 0.25f ) );
    OpponentGrave->SetRotation( 180.f );
    
    // Setup starting state on everything
    LocalPlayer->SetMana( 10 );
    Opponent->SetMana( 10 );
    
    // TODO: King Card
    
    LocalPlayer->SetHealth( 25 );
    Opponent->SetHealth( 25 );
    
}

void SingleplayerAuthority::SceneInit( cocos2d::Scene *inScene )
{
    EntityBase::SceneInit( inScene );
    
    // Setup turn manager callbacks
    CC_ASSERT( turnManager );
    
    using namespace std::placeholders;
    turnManager->SetGameStateCallback( std::bind( &SingleplayerAuthority::OnGameStateChanged, this, _1 ) );
    turnManager->SetTurnStateCallback( std::bind( &SingleplayerAuthority::OnTurnStateChanged, this, _1, _2 ) );
    
    // Wait 2 seconds and then start the match
    cocos2d::Director::getInstance()->getScheduler()->schedule( CC_CALLBACK_0( SingleplayerAuthority::StartGame, this ), this, 2.f, 0, 0.f, false, "StartTimer" );
}

void SingleplayerAuthority::StartGame()
{
    cocos2d::log( "[Auth] Starting Game!" );
    
    // Start ticks
    cocos2d::Director::getInstance()->getScheduler()->scheduleUpdate( this, 0, false );
    
    // Start turn manager
    turnManager->StartGame();
}

/*=================================================================================
    Game Flow Logic
 =================================================================================*/
void SingleplayerAuthority::OnGameStateChanged( GameState inState )
{
    if( inState == GameState::CoinFlip )
    {
        cocos2d::log( "[Auth] Game state update: Coin Flip" );
        // Choose starting player, play coin flip animation, and announce winner,
        // wait a couple seconds, and call TurnManager::SetupMatch( <CoinFlipWinner> );
        
        // TODO
        turnManager->SetupMatch( PlayerTurn::LocalPlayer );
    }
    else if( inState == GameState::InitialDraw )
    {
        cocos2d::log( "[Auth] Game state update: Initial Draw" );
        
        // Perform initial draw
        for( int i = 0; i < 10; i++ )
        {
            DrawCard( LocalPlayer );
            DrawCard( Opponent );
        }
        
        // Wait for user to finish playing cards
    }
    else if( inState == GameState::Main )
    {
        cocos2d::log( "[Auth] Game state update: Main" );
        // TODO
    }
    else if( inState == GameState::PostMatch )
    {
        // TODO
    }
}

void SingleplayerAuthority::OnTurnStateChanged( PlayerTurn inPlayer, TurnState inState )
{
    if( inState == TurnState::PreTurn )
    {
        cocos2d::log( "[Auth] Game state update: Pre Turn" );
        // TODO: Reset State?
        if( inPlayer == PlayerTurn::LocalPlayer )
            DrawCard( LocalPlayer );
        else
            DrawCard( Opponent );
        
        turnManager->Advance();
    }
    else if( inState == TurnState::BuildForces )
    {
        cocos2d::log( "[Auth] Game state update: Build Forces" );
        // TODO: Update User/UI
        // Wait for user to finish playing cards
    }
    else if( inState == TurnState::Planning )
    {
        cocos2d::log( "[Auth] Game state update: Planning" );
        // TODO: Update User/UI
        // Wait for user to finish planning
    }
    else if( inState == TurnState::Engage )
    {
        cocos2d::log( "[Auth] Game state update: Engage" );
        // TODO: Have AI perform block
        // Do game stuff
        
        turnManager->Advance();
    }
    else if( inState == TurnState::PostBattle )
    {
        cocos2d::log( "[Auth] Game state update: Post Battle" );
        turnManager->Advance();
    }
}


void SingleplayerAuthority::update( float Delta )
{
    // 'Tick' System Mock Up
     auto delta = std::chrono::duration_cast< std::chrono::seconds >( std::chrono::steady_clock::now() - StateBegin );
    
    // Check round state advane
    if( _gstate == GameState::PreGame )
    {
        // Check if enough time advanced to start coin flip
        if( delta.count() > 2 )
        {
            OnPreGameComplete();
        }
    }
    
    
    // TODO: Play Animation
    
    
}

void SingleplayerAuthority::OnPreGameComplete()
{
    // Update game state
    _gstate = GameState::CoinFlip;
    StateBegin = std::chrono::steady_clock::now();
    
    // Choose random player to start
    _plturn = PlayerTurn::LocalPlayer;
    
    // TODO: Play some type of animation for coin flip
    
}


void SingleplayerAuthority::DrawCard( Player* Target )
{
    if( !Target )
        return;
    
    // Draw a card if the player is able to
    auto deck = Target->GetDeck();
    auto hand = Target->GetHand();
    
    if( deck && hand )
    {
        if( deck->Count() > 0 )
        {
            auto DrawnCard = deck->At( 0 );
            if( DrawnCard )
            {
                // Move card
                deck->Remove( DrawnCard );
                hand->AddToBottom( DrawnCard );
            }
        }
    }
}

void SingleplayerAuthority::FinishTurn( Player *inPlayer )
{
    auto game = GetGameState();
    auto pl = GetPlayersTurn();
    auto state = GetTurnState();
    
    bool bIsLocal = inPlayer == LocalPlayer;
    bool bIsPlTurn = ( bIsLocal && pl == PlayerTurn::LocalPlayer ) || ( !bIsLocal && pl == PlayerTurn::Opponent );
    
    if( game == GameState::InitialDraw && bIsLocal )
    {
        turnManager->Advance();
    }
    else if( game == GameState::Main )
    {
        if( bIsPlTurn && ( state == TurnState::BuildForces || state == TurnState::Planning ) )
        {
            turnManager->Advance();
        }
        else if( !bIsPlTurn && state == TurnState::Engage )
        {
            turnManager->Advance();
        }
    }
}

bool SingleplayerAuthority::CanPlayCard( Player* inPlayer, CardEntity* inCard )
{
    if( !inPlayer || !inCard )
        return false;
    
    // Must be owned by the player thats trying to play the card
    if( inCard->GetOwningPlayer() == inPlayer )
    {
        // Check turn and game state
        auto game = GetGameState();
        auto pl = GetPlayersTurn();
        auto turn = GetTurnState();
        
        if( game != GameState::InitialDraw )
        {
            // Ensure its main state
            if( game != GameState::Main )
                return false;
            
            // Check if its this players turn
            if( pl == PlayerTurn::LocalPlayer &&
               inPlayer == Opponent )
                return false;
            
            if( pl == PlayerTurn::Opponent &&
               inPlayer == LocalPlayer )
                return false;
            
            // Check if its the right game state
            if( turn != TurnState::BuildForces )
                return false;
        }
        
        // Ensure card is actually in hand
        if( !inCard->InHand() )
            return false;
        
        // Now, its either initial draw, or main, and this players build forces turn
        // Check if they have enough mana, and we should be all set
        return inPlayer->GetMana() >= inCard->ManaCost;
    }
    
    return false;
}

bool SingleplayerAuthority::PlayCard( Player* inPlayer, CardEntity *inCard, bool bMoveCard )
{
    // Validate Move
    if( !CanPlayCard( inPlayer, inCard ) )
        return false;
    
    
    if( bMoveCard )
    {
        // Move card to field
        auto cont = inCard->GetContainer();
        auto field = inPlayer->GetField();
        
        if( !field )
            return false;
        
        if( cont )
            cont->Remove( inCard );
        
        field->AddToBottom( inCard );
    }
    
    // Subtract mana
    inPlayer->SetMana( inPlayer->GetMana() - inCard->ManaCost );
    
    return true;
}

void SingleplayerAuthority::Test( float Delta )
{
    /*
    ICardContainer* Deck = Opponent->GetDeck();
    ICardContainer* Field = Opponent->GetField();
    ICardContainer* Hand = Opponent->GetHand();
    ICardContainer* Grave = Opponent->GetGraveyard();
    ICardContainer* PDeck = LocalPlayer->GetDeck();
    ICardContainer* PField = LocalPlayer->GetField();
    ICardContainer* PHand = LocalPlayer->GetHand();
    ICardContainer* PGrave = LocalPlayer->GetGraveyard();
    
    static int i = 0;
    i++;
    
    if( Deck->Count() > 0 )
    {
        auto card = Deck->At( 0 );
        Deck->RemoveTop();
        
        if( i % 3 != 0 )
        {
            Field->AddToBottom( card );
        }
        else
        {
            Hand->AddToBottom( card );
        }
    }
    
    if( PDeck->Count() > 0 )
    {
        auto card = PDeck->At( 0 );
        PDeck->RemoveTop();
        
        if( i % 3 != 0 )
        {
            PField->AddToBottom( card );
        }
        else
        {
            PHand->AddToBottom( card );
        }
    }
     Random Test
     
    int res = cocos2d::random( 0, 3 );
    int other = cocos2d::random( 0, 3 );
    
    ICardContainer* RandContainer;
    ICardContainer* OtherContainer;
    if( res == 0 )
        RandContainer = LocalPlayer->GetDeck();
    else if( res == 1 )
        RandContainer = LocalPlayer->GetHand();
    else if( res == 2 )
        RandContainer = LocalPlayer->GetGraveyard();
    else
        RandContainer = LocalPlayer->GetField();
    
    if( other == 0 )
        OtherContainer = LocalPlayer->GetHand();
    else if( other == 1 )
        OtherContainer = LocalPlayer->GetDeck();
    else if ( other == 2 )
        OtherContainer = LocalPlayer->GetGraveyard();
    else
        OtherContainer = LocalPlayer->GetField();
    
    cocos2d::log( "A" );
    int ct = 0;
    for( int i = 0; i < RandContainer->Count(); i++ )
    {
        cocos2d::log( "B" );
        if( RandContainer->IndexValid( i ) )
        {
            cocos2d::log( "C" );
            auto* Card = RandContainer->At( i );
            if( Card )
            {
                cocos2d::log( "D" );
                RandContainer->RemoveAtIndex( i );
                OtherContainer->AddToTop( Card );
            }
        }
        
        ct++;
        if( ct > 1 )
            break;
    }
     */
}
