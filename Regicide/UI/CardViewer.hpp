//
//    CardViewer.hpp
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


class CardViewer : public cocos2d::Layer
{
    
public:
 
    static CardViewer* create( Game::CardEntity* inCard, bool bAllowPlay );
    virtual bool init();
    
    CardViewer();
    ~CardViewer();
    
    void SetTargetCard( Game::CardEntity* inCard, bool bAllowPlay );
    inline void SetCloseCallback( std::function< void() > inCallback ) { CloseCallback = inCallback; }
    inline void SetPlayCallback( std::function< void() > inCallback ) { PlayCallback = inCallback; }
    inline void SetAbilityCallback( std::function< void( int ) > inCallback ) { AbilityCallback = inCallback; }
    
protected:
    
    Game::CardEntity* TargetCard;
    cocos2d::Sprite* CardImage;
    cocos2d::DrawNode* Background;
    
    bool onTouch( cocos2d::Touch* inTouch, cocos2d::Event* inEvent );
    cocos2d::EventListenerTouchOneByOne* Listener;
    std::function< void() > CloseCallback;
    std::function< void() > PlayCallback;
    std::function< void( int ) > AbilityCallback;
};
