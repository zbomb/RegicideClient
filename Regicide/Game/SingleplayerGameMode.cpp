//
//  SingleplayerGameMode.cpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/10/18.
//

#include "SingleplayerGameMode.hpp"
#include "World.hpp"
#include "SingleplayerAuthority.hpp"
#include "CardEntity.hpp"
#include "HandEntity.hpp"
#include "GraveyardEntity.hpp"


using namespace Game;


void SingleplayerGameMode::Initialize()
{
    
}

void SingleplayerGameMode::PostInitialize()
{
    
}

void SingleplayerGameMode::Cleanup()
{

}

Player* SingleplayerGameMode::GetLocalPlayer()
{
    // Get reference to Authority
    auto Auth = GetAuthority< SingleplayerAuthority >();
    if( !Auth )
        return nullptr;
    
    return Auth->GetPlayer();
}

Player* SingleplayerGameMode::GetOpponent()
{
    // Get reference to Authority
    auto Auth = GetAuthority< SingleplayerAuthority >();
    if( !Auth )
        return nullptr;
    
    return Auth->GetOpponent();
}

void SingleplayerGameMode::TouchBegan( cocos2d::Touch *inTouch, CardEntity *inCard )
{
    // TODO: Dragging from hand to play cards
    _touchedCard = inCard;
}

void SingleplayerGameMode::TouchEnd( cocos2d::Touch *inTouch, CardEntity *inCard )
{
    cocos2d::log( "[DEBUG] TOUCH ENDED" ); // RESUME HERE
    // If we pressed and released while on the same card, count as click
    // If we drag onto a card from nothing, or drag off from a card then it isnt a click
    if( inCard && _touchedCard == inCard )
    {
        OnCardClicked( inCard );
    }
    else
    {
        // Close Hand
        auto pl = GetLocalPlayer();
        auto hand = pl ? pl->GetHand() : nullptr;
        
        if( hand )
            hand->SetExpanded( false );
    }
    
    _touchedCard = nullptr;
}

void SingleplayerGameMode::TouchMoved( cocos2d::Touch *inTouch )
{
    
}

void SingleplayerGameMode::TouchCancel( cocos2d::Touch* inTouch )
{
    _touchedCard = nullptr;
}

void SingleplayerGameMode::OnCardClicked( CardEntity* inCard )
{
    cocos2d::log( "[DEBUG] CARD CLICKED" );
    
    auto Container = inCard->GetContainer();
    
    auto LocalPlayer = GetLocalPlayer();
    auto Opponent = GetOpponent();
    
    // Check if player clicked on a graveyard
    if( Container == (ICardContainer*)LocalPlayer->GetGraveyard() ||
       Container == (ICardContainer*)Opponent->GetGraveyard() )
    {
        OpenGraveyardViewer( static_cast< GraveyardEntity* >( Container ) );
    }
    else if( Container == (ICardContainer*)LocalPlayer->GetField() ||
            Container == (ICardContainer*)Opponent->GetField() )
    {
        OpenCardViewer( inCard );
    }
    else if( Container == (ICardContainer*)LocalPlayer->GetHand() )
    {
        OpenHandViewer( inCard );
    }
    else
    {
        // Other situations are ignored, we cant look at the deck, or opponents hand
        // We will close the viewers instead
        auto hand = LocalPlayer->GetHand();
        
        if( hand && hand->IsExpanded() )
            hand->SetExpanded( false );
    }
    
}

void SingleplayerGameMode::OpenCardViewer( CardEntity *inCard )
{
    
}

void SingleplayerGameMode::OpenHandViewer( CardEntity *inCard )
{
    auto Player = GetLocalPlayer();
    if( Player == inCard->GetOwningPlayer() )
    {
        auto hand = Player->GetHand();
        
        // If the hand isnt expanded, then exapnd it
        if( !hand->IsExpanded() )
        {
            hand->SetExpanded( true );
        }
        else
        {
            // Fully open card viewer for this card
            OpenCardViewer( inCard );
        }
    }
}

void SingleplayerGameMode::OpenGraveyardViewer( GraveyardEntity *Grave )
{
    
}
