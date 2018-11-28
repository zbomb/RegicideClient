//
//	AbilityText.hpp
//	Regicide Mobile
//
//	Created: 11/25/18
//	Updated: 11/25/18
//
//	© 2018 Zachary Berry, All Rights Reserved
//

#pragma once

#include "cocos2d.h"
#include "IconCount.hpp"
#include "Game/CardEntity.hpp"
#include "ui/CocosGUI.h"

class AbilityText : public cocos2d::ui::Widget
{
public:
    
    AbilityText();
    ~AbilityText();
    
    static AbilityText* Create( Game::CardEntity* InCard, Game::Ability& InAbility, float inWidth, bool bDrawSep );
    virtual bool init( Game::CardEntity* InCard, Game::Ability& InAbility, float inWidth, bool bDrawSep );
    virtual void onSizeChanged() override;
    
    float GetDesiredHeight();
    
    inline bool CanTrigger() const { return bCanTrigger; }
    
protected:
    
    cocos2d::Label* Text;
    IconCount* ManaCost;
    IconCount* StaminaCost;
    cocos2d::DrawNode* Draw;
    
    Game::Ability Ability;
    Game::CardEntity* Card;
    
    cocos2d::Vec2 _touchStart;
    
    cocos2d::EventListenerTouchOneByOne* Listener;
    
    bool bCanTrigger;
    bool bDrawSeperator;
    bool bDescription;
    
    bool onTouch( cocos2d::Touch* inTouch, cocos2d::Event* inEvent );
    void onTouchEnd( cocos2d::Touch* inTouch, cocos2d::Event* inEvent );

};
