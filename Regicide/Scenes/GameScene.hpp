//
//    GameScene.hpp
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
    void UpdateTurnState( const std::string& In );
    void UpdatePlayerTurn( const std::string& In );
    
    void ShowFinishButton();
    void HideFinishButton();
    
    virtual void ExitGame();
    
private:
    
    cocos2d::ui::Button* ExitButton;
    cocos2d::ui::Button* FinishButton;
    cocos2d::Label* FinishLabel; 
    cocos2d::Label* TurnLabel;
    cocos2d::Label* PlayerLabel;
    
    CardLayer* cardLayer;
    
};
