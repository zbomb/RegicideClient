//
//  OnlineLauncher.hpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/9/18.
//


#pragma once

#include "cocos2d.h"
#include "ui/CocosGUI.h"

class OnlineLauncherScene : public cocos2d::Scene
{
    
public:
    
    // Cocos2d Implementation
    static cocos2d::Scene* createScene();
    virtual bool init();
    CREATE_FUNC( OnlineLauncherScene );
    
    
private:
    
    cocos2d::DrawNode* Draw;
    cocos2d::Label* Header;
    cocos2d::ui::Button* BackButton;
    
    void OnBackClicked( cocos2d::Ref* Caller );
    
};
