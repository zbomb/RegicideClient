//
//    CardLayer.cpp
//    Regicide Mobile
//
//    Created: 11/13/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#include "CardLayer.hpp"
#include "Game/World.hpp"
#include "Game/GameModeBase.hpp"
#include "Game/Player.hpp"
#include "Game/CardEntity.hpp"

CardLayer::~CardLayer()
{
    _eventDispatcher->removeEventListenersForTarget( this );
}

bool CardLayer::init()
{
    if( !Layer::init() )
        return false;
    
    auto Listener = cocos2d::EventListenerTouchOneByOne::create();
    CC_ASSERT( Listener );
    
    Listener->setEnabled( true );
    Listener->setSwallowTouches( false );
    
    Listener->onTouchBegan      = CC_CALLBACK_2( CardLayer::onTouchBegan, this );
    Listener->onTouchMoved      = CC_CALLBACK_2( CardLayer::onTouchMoved, this );
    Listener->onTouchEnded      = CC_CALLBACK_2( CardLayer::onTouchEnded, this );
    Listener->onTouchCancelled  = CC_CALLBACK_2( CardLayer::onTouchCancelled, this );
    
    _eventDispatcher->addEventListenerWithSceneGraphPriority( Listener, this );
    
    FXNode = cocos2d::DrawNode::create();
    FXNode->setGlobalZOrder( 10000 );
    addChild( FXNode );
    
    BlockDraw = cocos2d::DrawNode::create();
    BlockDraw->setGlobalZOrder( 10000 );
    addChild( BlockDraw );
    
    return true;
}

/*=======================================================================================
    Touch Events
    * Passed along to the GameMode to be processed, we dont want to deal with
      this here. Only thing were going to do is determine if a sprite was touched
=======================================================================================*/
Game::CardEntity* CardLayer::TraceTouch( const cocos2d::Vec2 &inPos )
{
    // We need to trace through all cards in the game to check if
    // the touch hit any of them. TODO: Find quicker way to trace touches?
    auto world = Game::World::GetWorld();
    
    if( world )
    {
        auto pl = world->GetLocalPlayer();
        auto op = world->GetOpponent();
        
        if( pl )
        {
            auto ret = pl->PerformTouchTrace( inPos );
            if( ret )
                return ret;
        }
        
        if( op )
        {
            auto ret = op->PerformTouchTrace( inPos );
            if( ret )
                return ret;
        }
    }
    
    return nullptr;
}


bool CardLayer::onTouchBegan( cocos2d::Touch *inTouch, cocos2d::Event *inEvent )
{
    // Check if we clicked a card
    Game::CardEntity* TouchedCard = inTouch ? TraceTouch( inTouch->getLocation() ) : nullptr;
    
    auto world = Game::World::GetWorld();
    auto GM = world ? world->GetGameMode() : nullptr;
    
    if( GM )
    {
        GM->TouchBegan( inTouch, TouchedCard );
    }
    
    return true;
}

void CardLayer::onTouchMoved( cocos2d::Touch *inTouch, cocos2d::Event *inEvent )
{
    auto world = Game::World::GetWorld();
    auto GM = world ? world->GetGameMode() : nullptr;
    
    if( GM )
    {
        GM->TouchMoved( inTouch, FXNode );
    }
}

void CardLayer::onTouchEnded( cocos2d::Touch *inTouch, cocos2d::Event *inEvent )
{
    Game::CardEntity* TouchedCard = inTouch ? TraceTouch( inTouch->getLocation() ) : nullptr;
    auto world = Game::World::GetWorld();
    auto GM = world ? world->GetGameMode() : nullptr;
    
    if( GM )
    {
        GM->TouchEnd( inTouch, TouchedCard, FXNode );
    }
}

void CardLayer::onTouchCancelled( cocos2d::Touch *inTouch, cocos2d::Event *inEvent )
{
    auto world = Game::World::GetWorld();
    auto GM = world ? world->GetGameMode() : nullptr;
    
    if( GM )
    {
        GM->TouchCancel( inTouch, FXNode );
    }
}

void CardLayer::RedrawBlockers()
{
    auto World = Game::World::GetWorld();
    auto GM = World ? World->GetGameMode() : nullptr;
    
    if( GM && BlockDraw )
    {
        BlockDraw->clear();
        
        // Draw a line connecting each blocker to the corresponding attacker
        auto Blockers = GM->GetBlockMatrix();
        for( auto It = Blockers.begin(); It != Blockers.end(); It++ )
        {
            if( It->first && It->second )
            {
                BlockDraw->drawSegment( It->first->GetAbsolutePosition(), It->second->GetAbsolutePosition(), 16.f, cocos2d::Color4F( 0.2f, 0.2f, 0.95f, 0.6f ) );
            }
        }
    }
}
