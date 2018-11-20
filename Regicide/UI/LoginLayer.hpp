//
//    LoginLayer.hpp
//    Regicide Mobile
//
//    Created: 10/9/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//
#pragma once

#include "cocos/cocos2d.h"
#include "cocos/2d/CCLayer.h"
#include "cocos/ui/UIEditBox/UIEditBox.h"
#include "cocos/ui/UIButton.h"

using namespace cocos2d;
using namespace cocos2d::ui;

class LoginLayer : public cocos2d::LayerColor, public cocos2d::ui::EditBoxDelegate
{

public:
	
	virtual bool init() override;
	CREATE_FUNC( LoginLayer );
    
    void Destroy();

	// EditBoxDelegate Implementation
	virtual void editBoxEditingDidBegin( EditBox* Box ) override;
	virtual void editBoxEditingDidEnd( EditBox* Box ) override;
	virtual void editBoxTextChanged( EditBox* Box, const std::string& Text ) override;
	virtual void editBoxReturn( EditBox* Box ) override;
	
	void ShowError( std::string ErrorMessage );

private:

	DrawNode* Draw 		= nullptr;
	Label* Header 		= nullptr;
	EditBox* Username 	= nullptr;
	EditBox* Password 	= nullptr;
	
	MenuItemImage* LoginButton 		= nullptr;
	MenuItemImage* CancelButton 	= nullptr;
	MenuItemImage* RegisterButton 	= nullptr;
	
	Menu* ButtonBank 		= nullptr;
	Label* UsernameLabel 	= nullptr;
	Label* PasswordLabel 	= nullptr;
	Label* ErrorMessage 	= nullptr;

	void OnLoginClick( Ref* Sender );
	void OnCancelClick( Ref* Sender );
	void OnRegisterClick( Ref* Sender );

};
