//
//  GameScene.hpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/11/18.
//

#pragma once

#include "cocos2d.h"
#include "Numeric.h"
#include "ui/CocosGUI.h"
#include "UI/CardLayer.hpp"

#define TAG_GAME 4321


class GameScene : public cocos2d::Scene
{
public:
    
    // Cocos2d Implementation
    static cocos2d::Scene* createScene();
    virtual bool init();
    CREATE_FUNC( GameScene );
    
    ~GameScene();
    
    inline CardLayer* GetCardLayer() { return cardLayer; }
    
private:
    
    cocos2d::ui::Button* ExitButton;
    CardLayer* cardLayer;
    
    virtual void ExitGame();
};
