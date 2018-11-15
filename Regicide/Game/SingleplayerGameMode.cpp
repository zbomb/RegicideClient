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
#include "FieldEntity.hpp"
#include "UI/CardViewer.hpp"


using namespace Game;


void SingleplayerGameMode::Initialize()
{
    
}

void SingleplayerGameMode::PostInitialize()
{
    
}

void SingleplayerGameMode::Cleanup()
{
    _DoCloseViewer();
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

void SingleplayerGameMode::OnTurnChanged( PlayerTurn inTurn )
{
    _CurrentTurn = inTurn;
    
    
}

/*=========================================================================================
    Input Layer
 =========================================================================================*/
void SingleplayerGameMode::TouchBegan( cocos2d::Touch *inTouch, CardEntity *inCard )
{
    _touchedCard = inCard;
    if( inCard && inCard->Sprite && inTouch )
    {
        // Set drag offset to the difference of the card center
        // and the click position
        _DragOffset = inCard->Sprite->getPosition() - inTouch->getLocation();
    }
}

void SingleplayerGameMode::TouchEnd( cocos2d::Touch *inTouch, CardEntity *inCard )
{
    // First, check if we were dragging a card
    if( _bDrag )
    {
        if( _touchedCard )
        {
            // Call DragDrop handler to see if this was a valid operation
            // If not, were going to release the card back to the original container
            bool bResult = OnCardDragDrop( _touchedCard, inTouch );
            _touchedCard->SetIsDragging( false );
            auto cont = _touchedCard->GetContainer();
            
            if( !bResult )
            {
                if( cont )
                    cont->InvalidateCards();
            }
        }
        
        _bDrag = false;
    }
    else if( inCard && inCard == _touchedCard )
    {
        OnCardClicked( inCard );
    }
    else
    {
        // Want touching any cards (or dragging) when touch released
        CloseCardViewer();
        CloseGraveyardViewer();
        CloseHandViewer();
    }
    
    _DragOffset = cocos2d::Vec2::ZERO;
    _touchedCard = nullptr;
}

void SingleplayerGameMode::TouchMoved( cocos2d::Touch *inTouch )
{
    // Check if card is being dragged & in player's hand
    if( _touchedCard && !_touchedCard->GetIsDragging() && _touchedCard->InHand() )
    {
        // Ask auth if it would be valid to play this card
        auto Auth = GetAuthority< SingleplayerAuthority >();
        
        if( Auth && Auth->CanPlayCard( GetLocalPlayer(), _touchedCard ) )
        {
            // We need to make sure that its fairly explicit that the user is trying to drag and not click
            auto deltaVec = inTouch->getDelta();
            
            if( deltaVec.length() > 9.f )
            {
                _touchedCard->SetIsDragging( true );
                _bDrag = true;
            }
        }
    }
    
    if( _bDrag && _touchedCard && inTouch &&_touchedCard->Sprite )
    {
        // Move card with the touch
        auto pos = inTouch->getLocation();
        _touchedCard->Sprite->setPosition( pos + _DragOffset );
    }
}

void SingleplayerGameMode::TouchCancel( cocos2d::Touch* inTouch )
{
    // Stop dragging
    if( _bDrag && _touchedCard )
    {
        _touchedCard->SetIsDragging( false );
        
        // Force the container that holds this card to reposition
        // so the sprite will automtaically animate back
        auto cont = _touchedCard->GetContainer();
        if( cont )
            cont->InvalidateCards();
    }
    
    _DragOffset = cocos2d::Vec2::ZERO;
    _bDrag = false;
    _touchedCard = nullptr;
}


bool SingleplayerGameMode::OnCardDragDrop( CardEntity *inCard, cocos2d::Touch *Info )
{
    // Determine if the player attempted to drop the card on the field
    // TODO: Maybe handle spells different, since they arent static, and dont need to stay on
    // the field, maybe just display something on the screen and query any user input without placing the
    // card in the battlefield
    
    if( !inCard || !Info )
        return false;
    
    // In the future, we can alter this method to allow dropping cards onto other entities
    // besides just the field & hand
    
    auto DropPos = Info->getLocation(); // Were not going to factor the offset when dropping, use actual touch pos
    auto pl = GetLocalPlayer();
    auto hand = pl ? pl->GetHand() : nullptr;
    auto field = pl ? pl->GetField() : nullptr;
    
    // First, attempt to drop into different hand position
    if( hand && hand->AttemptDrop( inCard, DropPos ) )
        return true;
    
    // If not, then try the field
    return( field && field->AttemptDrop( inCard, DropPos ) );
}

void SingleplayerGameMode::OnCardClicked( CardEntity* inCard )
{
    auto LocalPlayer = GetLocalPlayer();
    bool bLocalOwner = inCard->GetOwningPlayer() == LocalPlayer;
    
    if( inCard->InGrave() )
    {
        CloseCardViewer();
        CloseHandViewer();
        OpenGraveyardViewer( inCard->GetOwningPlayer()->GetGraveyard() );
        
        return;
    }
    else if( inCard->OnField() )
    {
        CloseGraveyardViewer();
        CloseHandViewer();
        OpenCardViewer( inCard );
        
        return;
    }
    else if( inCard->InHand() && bLocalOwner )
    {
        CloseGraveyardViewer();
        CloseCardViewer();
        OpenHandViewer( inCard );
        
        return;
    }
    
    CloseGraveyardViewer();
    CloseCardViewer();
    CloseHandViewer();
}

void SingleplayerGameMode::CloseCardViewer()
{
    // We need to force the hand and field to invalidate, which will
    // move the card back to where it should be
    _DoCloseViewer();
    _viewCard = nullptr;
}

void SingleplayerGameMode::CloseGraveyardViewer()
{
    
}

void SingleplayerGameMode::CloseHandViewer()
{
    auto pl = GetLocalPlayer();
    auto hand = pl ? pl->GetHand() : nullptr;
    
    if( hand && hand->IsExpanded() )
        hand->SetExpanded( false );
}

void SingleplayerGameMode::OpenCardViewer( CardEntity *inCard )
{
    if( !inCard || inCard == _viewCard )
        return;
    
    _viewCard = inCard;
    
    auto dir = cocos2d::Director::getInstance();
    auto origin = dir->getVisibleOrigin();
    auto scene = dir->getRunningScene();
    
    // For now, were just going to create a new UI menu, that fades in containing
    // the full sized card image, instead of trying to perform a sprite animation
    // TODO: Better looking animation
    
    // Determine if the card is in the hand or not
    auto Auth = GetAuthority< SingleplayerAuthority >();
    bool bCanPlay = Auth && Auth->CanPlayCard( GetLocalPlayer(), inCard );
    
    _DoCloseViewer();
    _Viewer = CardViewer::create( inCard, bCanPlay );
    _Viewer->SetCloseCallback( std::bind( &SingleplayerGameMode::_DoCloseViewer, this ) );
    if( bCanPlay )
        _Viewer->SetPlayCallback( std::bind( &SingleplayerGameMode::_DoPlayCard, this, inCard ) );
    
    _Viewer->setGlobalZOrder( 300 );
    scene->addChild( _Viewer, 200 );
}

void SingleplayerGameMode::_DoPlayCard( CardEntity *inCard )
{
    // Pass call along to PlayCard
    auto Auth = GetAuthority< SingleplayerAuthority >();
    if( Auth )
    {
        if( !Auth->PlayCard( GetLocalPlayer(), inCard, true ) )
        {
            // TODO: Show something to indicate this card cannot be played
        }
    }
}

void SingleplayerGameMode::_DoCloseViewer()
{
    if( _Viewer )
    {
        _Viewer->removeFromParent();
        _Viewer = nullptr;
    }
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
    cocos2d::log( "[DEBUG] OPENING GRAVEYARD VIEWER" );
}
