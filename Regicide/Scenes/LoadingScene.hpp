//
//    LoadingScene.hpp
//    Regicide Mobile
//
//    Created: 11/11/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#pragma once

#include "cocos2d.h"
#include "Numeric.hpp"
#include "ui/CocosGUI.h"

#define TAG_LOADING 1234


class LoadingScene : public cocos2d::Scene
{
    
public:
    
    // Cocos2d Implementation
    static cocos2d::Scene* createScene();
    virtual bool init();
    CREATE_FUNC( LoadingScene );
    
    ~LoadingScene();
    
    void UpdateProgress( float inProgress, const std::string& Description = std::string() );
    
private:
    
    cocos2d::DrawNode* Background;
    cocos2d::Label* Header;
    cocos2d::Label* Description;
    cocos2d::ui::LoadingBar* Progress;
};
