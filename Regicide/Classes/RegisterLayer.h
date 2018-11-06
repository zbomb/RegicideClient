#pragma once

#include "cocos/cocos2d.h"
#include "cocos/2d/CCLayer.h"
#include "cocos/ui/UIEditBox/UIEditBox.h"
#include "cocos/ui/UIButton.h"


using namespace cocos2d;
using namespace cocos2d::ui;

class RegisterLayer : public cocos2d::LayerColor
{
    
public:
    
    virtual bool init();
    CREATE_FUNC( RegisterLayer );
    
    void Destroy();
    void OnRegisterFailure( int Result );
    
private:
    
    DrawNode* Draw  = nullptr;
    Label* Header   = nullptr;
    Label* UserLabel = nullptr;
    Label* PassLabel = nullptr;
    Label* EmailLabel = nullptr;
    Label* DispLabel = nullptr;
    Label* ConfPassLabel = nullptr;
    Label* ErrorMessage     = nullptr;
    
    EditBox* UserBox        = nullptr;
    EditBox* PassBox        = nullptr;
    EditBox* EmailBox       = nullptr;
    EditBox* DispBox        = nullptr;
    EditBox* ConfPassBox    = nullptr;
    
    Menu* Buttons   = nullptr;
    MenuItemImage* CreateButton     = nullptr;
    MenuItemImage* CancelButton     = nullptr;
    MenuItemImage* LoginButton      = nullptr;
    
    void OnCancel( Ref* inRef );
    void OpenLogin( Ref* inRef );
    void DoRegister( Ref* inRef );
    
    void ShowError( std::string ErrorMessage );
};
