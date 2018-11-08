//
//  OptionsScene.hpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/8/18.
//

#pragma once

#include "cocos2d.h"
#include "ui/CocosGUI.h"


class OptionsScene : public cocos2d::Scene
{
    
public:
    
    // Cocos2d Implementation
    static cocos2d::Scene* createScene();
    virtual bool init();
    CREATE_FUNC( OptionsScene );
    
    void OnBackClicked( cocos2d::Ref* Caller );
    void OnLogoutClicked( cocos2d::Ref* Caller );
    void OnClearContentClicked( cocos2d::Ref* Caller );
    
private:
    
    cocos2d::DrawNode* Draw;
    cocos2d::Label* Header;
    cocos2d::ui::Button* BackButton;
    cocos2d::ui::Button* LogoutText;
    cocos2d::ui::Button* ContentText;
};
