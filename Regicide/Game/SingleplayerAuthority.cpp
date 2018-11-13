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


using namespace Game;


SingleplayerAuthority::~SingleplayerAuthority()
{
    cocos2d::Director::getInstance()->getScheduler()->unschedule( "CardTest", this );
    
    LocalPlayer     = nullptr;
    Opponent        = nullptr;
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
    
}

void SingleplayerAuthority::SceneInit( cocos2d::Scene *inScene )
{
    EntityBase::SceneInit( inScene );
    
    // Move Test
    cocos2d::Director::getInstance()->getScheduler()->schedule( CC_CALLBACK_1( SingleplayerAuthority::Test, this ), this, 0.6f, CC_REPEAT_FOREVER, 0.f, false, "CardTest"  );

}

void SingleplayerAuthority::Test( float Delta )
{
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
    /*  Random Test
     
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
