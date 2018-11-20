//
//    OnlineLauncher.hpp
//    Regicide Mobile
//
//    Created: 11/9/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
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
