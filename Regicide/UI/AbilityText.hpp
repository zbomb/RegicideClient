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
    
    static AbilityText* Create( Game::CardEntity* InCard, Game::Ability& InAbility, float inWidth );
    virtual bool init( Game::CardEntity* InCard, Game::Ability& InAbility, float inWidth );
    virtual void onSizeChanged() override;
    
    float GetDesiredHeight();
    
    void OnTouch( cocos2d::Ref* Caller, cocos2d::ui::Widget::TouchEventType Type );
    
protected:
    
    cocos2d::Label* Text;
    IconCount* ManaCost;
    IconCount* StaminaCost;
    cocos2d::DrawNode* Draw;
    
    Game::Ability Ability;
    Game::CardEntity* Card;
    
    bool bCanTrigger;

};
