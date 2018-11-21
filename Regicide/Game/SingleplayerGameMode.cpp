//
//    SingleplayerGameMode.cpp
//    Regicide Mobile
//
//    Created: 11/10/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#include "SingleplayerGameMode.hpp"
#include "World.hpp"
#include "SingleplayerAuthority.hpp"
#include "CardEntity.hpp"
#include "HandEntity.hpp"
#include "GraveyardEntity.hpp"
#include "FieldEntity.hpp"
#include "UI/CardViewer.hpp"
#include "Scenes/GameScene.hpp"

using namespace Game;


SingleplayerGameMode::SingleplayerGameMode()
: GameModeBase()
{
    _bSelectionEnabled = true;
    
    using namespace std::placeholders;
    SetActionCallback( "Win", std::bind( &SingleplayerGameMode::Action_GameWon, this, _1, _2 ) );
}

void SingleplayerGameMode::Initialize()
{
    _bSelectionEnabled = true;
}

void SingleplayerGameMode::Cleanup()
{
    GameModeBase::Cleanup();
    _DoCloseViewer();
}

void SingleplayerGameMode::DisableSelection()
{
    _bSelectionEnabled = false;
    
    if( _bDrag && _touchedCard )
        _touchedCard->SetIsDragging( false );
}

void SingleplayerGameMode::EnableSelection()
{
    _bSelectionEnabled = true;
}

/*=========================================================================================
    Input Layer
 =========================================================================================*/
void SingleplayerGameMode::TouchBegan( cocos2d::Touch *inTouch, CardEntity *inCard )
{
    if( !_bSelectionEnabled )
    {
        _bDrag = false;
        if( _touchedCard )
            _touchedCard->SetIsDragging( false );
        
        _touchedCard = nullptr;
        return;
    }
    
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
    if( !_bSelectionEnabled )
    {
        _bDrag = false;
        if( _touchedCard )
            _touchedCard->SetIsDragging( false );
        
        _touchedCard = nullptr;
        return;
    }
    
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
    if( !_bSelectionEnabled )
    {
        _bDrag = false;
        if( _touchedCard )
            _touchedCard->SetIsDragging( false );
        
        _touchedCard = nullptr;
        return;
    }
    
    // Check if card is being dragged & in player's hand
    if( _touchedCard && !_touchedCard->GetIsDragging() && _touchedCard->InHand() )
    {
        // Check if the gamemode thinks we can even play this card
        if( CanPlayCard( _touchedCard ) )
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
    if( !_bSelectionEnabled )
    {
        _bDrag = false;
        if( _touchedCard )
            _touchedCard->SetIsDragging( false );
        
        _touchedCard = nullptr;
        return;
    }
    
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
    auto pl = GetPlayer();
    auto hand = pl ? pl->GetHand() : nullptr;
    auto field = pl ? pl->GetField() : nullptr;
    
    // First, attempt to drop into different hand position
    if( hand && hand->AttemptDrop( inCard, DropPos ) )
        return true;
    
    // If not, then try the field
    if( field )
    {
        int BestIndex = field->AttemptDrop( inCard, DropPos );
        if( BestIndex < 0 )
            return false;
        
        // Player attempted to drop card onto field, so we need to pass call along
        // to the authority to be processed
        auto Auth = GetAuthority< SingleplayerAuthority >();
        CC_ASSERT( Auth );
        
        Auth->PlayCard( inCard, BestIndex );
    }
    
    return false;
}

void SingleplayerGameMode::OnCardClicked( CardEntity* inCard )
{
    auto LocalPlayer = GetPlayer();
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
        // Check if were in attack phase
        if( tState == TurnState::Attack )
        {
            if( inCard && inCard->bAttacking )
            {
                inCard->bAttacking = false;
                inCard->ClearOverlay();
            }
            else if( inCard && !inCard->bAttacking )
            {
                // Check if the card is able to attack
                if( CanCardAttack( inCard ) )
                {
                    inCard->bAttacking = true;
                    inCard->SetOverlay( "icon_attack.png", 180 );
                }
            }
        }
        else
        {
            CloseGraveyardViewer();
            CloseHandViewer();
            OpenCardViewer( inCard );
        }
        
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
    auto pl = GetPlayer();
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
    bool bCanPlay = CanPlayCard( inCard );
    
    _DoCloseViewer();
    _Viewer = CardViewer::create( inCard, bCanPlay );
    _Viewer->SetCloseCallback( std::bind( &SingleplayerGameMode::_DoCloseViewer, this ) );
    if( bCanPlay )
        _Viewer->SetPlayCallback( std::bind( &SingleplayerGameMode::PlayCard, this, inCard, -1 ) );
    
    _Viewer->setGlobalZOrder( 300 );
    scene->addChild( _Viewer, 200 );
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
    auto Player = GetPlayer();
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

void SingleplayerGameMode::Action_GameWon( Action* In, std::function< void() > Callback )
{
    // Cast to WinEvent
    auto Win = dynamic_cast< WinAction* >( In );
    if( !Win )
    {
        cocos2d::log( "[GM] Win event occured, but unable to cast to WinAction!" );
        Callback();
        return;
    }
    
    cocos2d::Sprite* Sprite;
    if( Win->bDidWin )
        Sprite = cocos2d::Sprite::create( "win_banner.png" );
    else
        Sprite = cocos2d::Sprite::create( "loose_banner.png" );
    
    auto Dir = cocos2d::Director::getInstance();
    auto Size = Dir->getVisibleSize();
    auto Origin = Dir->getVisibleOrigin();
    
    Sprite->setGlobalZOrder( 99999 );
    Sprite->setAnchorPoint( cocos2d::Vec2( 0.5f, 0.5f ) );
    Sprite->setPosition( Origin + Size * 0.5f );
    
    GetScene()->addChild( Sprite );
    DisableSelection();
    
    Sprite->runAction( cocos2d::FadeIn::create( 0.3f ) );
    Dir->getScheduler()->schedule( [=]( float f ) { if( Callback ) Callback(); }, this, 0.35f, 0, 0.f, false, "WinCallback" );
    
    // Clear Card Overlays/Glows
    auto Cards = IEntityManager::GetInstance().GetAllCards();
    for( auto It = Cards.begin(); It != Cards.end(); It++ )
    {
        if( *It )
        {
            (*It)->ClearOverlay();
            (*It)->ClearHighlight();
        }
    }
    
    Dir->getScheduler()->schedule( [=]( float f )
    {
        auto parentScene = GetScene();
        if( parentScene )
        {
            auto Scene = dynamic_cast< GameScene* >( parentScene );
            if( Scene )
            {
                Scene->ExitGame();
            }
        }
        
    }, this, 5.f, 0, 0.f, false, "ExitTimer" );
    
    auto parentScene = GetScene();
    if( parentScene )
    {
        auto Scene = dynamic_cast< GameScene* >( parentScene );
        if( Scene )
        {
            Scene->HideFinishButton();
        }
    }
}

