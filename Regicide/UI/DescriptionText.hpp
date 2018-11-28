//
//	DescriptionText.hpp
//	Regicide Mobile
//
//	Created: 11/27/18
//	Updated: 11/27/18
//
//	© 2018 Zachary Berry, All Rights Reserved
//

#pragma once

#include "cocos2d.h"
#include "ui/CocosGUI.h"

class DescriptionText : public cocos2d::ui::Widget
{
public:
    
    static DescriptionText* Create( const std::string& InDesc, float inWide, bool bDrawSep );
    virtual bool init( const std::string& InDesc, float inWide, bool bDrawSep );
    
    float GetDesiredHeight();
    virtual void onSizeChanged() override;
    
protected:
    
    cocos2d::Label* Text;
    cocos2d::DrawNode* Draw;
    bool bDrawSeperator = false;
};
