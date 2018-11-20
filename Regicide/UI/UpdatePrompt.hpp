//
//    UpdatePrompt.hpp
//    Regicide Mobile
//
//    Created: 11/8/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#pragma once

#include "cocos/cocos2d.h"
#include "cocos/2d/CCLayer.h"
#include "cocos/ui/UIButton.h"
#include "ui/CocosGUI.h"


class UpdatePrompt : public cocos2d::LayerColor
{
    
public:
    
    virtual bool init() override;
    CREATE_FUNC( UpdatePrompt );
    
    void Destroy();
    
    void OnAccept( cocos2d::Ref* Caller );
    void ListenForUpdate( bool bNeeds, bool bError, std::string ErrMessage );
    
private:
    
    cocos2d::DrawNode* Draw         = nullptr;
    cocos2d::Label* Header         = nullptr;
    cocos2d::ui::Text* Body = nullptr;
    cocos2d::ui::Button* Accept = nullptr;
    
    
};

