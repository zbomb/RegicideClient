//
//    CardLayer.hpp
//    Regicide Mobile
//
//    Created: 11/13/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#pragma once

#include "cocos2d.h"
#include "Game/CardEntity.hpp"

class CardLayer : public cocos2d::Layer
{
    
public:
    
    virtual bool init();
    CREATE_FUNC( CardLayer );
    
    ~CardLayer();
    
    virtual bool onTouchBegan( cocos2d::Touch* inTouch, cocos2d::Event* inEvent );
    virtual void onTouchMoved( cocos2d::Touch* inTouch, cocos2d::Event* inEvent );
    virtual void onTouchEnded( cocos2d::Touch* inTouch, cocos2d::Event* inEvent );
    virtual void onTouchCancelled( cocos2d::Touch* inTouch, cocos2d::Event* inEvent );
    
protected:
    
    Game::CardEntity* TraceTouch( const cocos2d::Vec2& inPos );
    
};
