//
//  CardLayer.cpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/13/18.
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
        auto GM = world->GetGameMode();
        if( GM )
        {
            auto pl = GM->GetLocalPlayer();
            auto op = GM->GetOpponent();
            
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
        GM->TouchMoved( inTouch );
    }
}

void CardLayer::onTouchEnded( cocos2d::Touch *inTouch, cocos2d::Event *inEvent )
{
    Game::CardEntity* TouchedCard = inTouch ? TraceTouch( inTouch->getLocation() ) : nullptr;
    
    auto world = Game::World::GetWorld();
    auto GM = world ? world->GetGameMode() : nullptr;
    
    if( GM )
    {
        GM->TouchEnd( inTouch, TouchedCard );
    }
}

void CardLayer::onTouchCancelled( cocos2d::Touch *inTouch, cocos2d::Event *inEvent )
{
    auto world = Game::World::GetWorld();
    auto GM = world ? world->GetGameMode() : nullptr;
    
    if( GM )
    {
        GM->TouchCancel( inTouch );
    }
}
