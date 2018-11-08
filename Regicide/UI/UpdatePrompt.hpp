//
//  UpdatePrompt.hpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/8/18.
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

