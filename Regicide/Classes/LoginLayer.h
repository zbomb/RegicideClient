#pragma once

#include "cocos/cocos2d.h"
#include "cocos/2d/CCLayer.h"
#include "cocos/ui/UIEditBox/UIEditBox.h"
#include "cocos/ui/UIButton.h"
#include "NetHeaders.h"

using namespace cocos2d;
using namespace cocos2d::ui;

class LoginLayer : public cocos2d::LayerColor
{

public:

	virtual bool init();
	CREATE_FUNC( LoginLayer );

private:

	DrawNode* Draw = nullptr;
	Label* Header = nullptr;
	EditBox* Username = nullptr;
	EditBox* Password = nullptr;
	MenuItemImage* LoginButton = nullptr;
	MenuItemImage* CancelButton = nullptr;
	MenuItemImage* RegisterButton = nullptr;
	Menu* ButtonBank = nullptr;
	Label* UsernameLabel = nullptr;
	Label* PasswordLabel = nullptr;

	void OnLoginClick( Ref* Sender );
	void OnCancelClick( Ref* Sender );
	void OnRegisterClick( Ref* Sender );

};