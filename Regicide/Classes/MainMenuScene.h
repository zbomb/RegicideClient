/*==========================================================================================
	Regicide Mobile

	MainMenuScene.h
	(c) 2018 Zachary Berry
=============================================================================================*/
#pragma once

#include "cocos2d.h"
#include "NetHeaders.h"
#include "LoginLayer.h"

using namespace cocos2d;

class MainMenu : public cocos2d::Scene
{

public:
	
	// Cocos2d Implementation
	static cocos2d::Scene* createScene();
	virtual bool init();
	CREATE_FUNC( MainMenu );

	void CloseErrorMenu();

private:

	DrawNode* HeaderBox		= nullptr;
	Label* Header			= nullptr;
	Sprite* Background		= nullptr;
	Menu* MenuContainer		= nullptr;
	DrawNode* Connecting	= nullptr;
	Label* ConnectingLabel	= nullptr;
										
	MenuItemLabel* OnlineButton			= nullptr;
	MenuItemLabel* SingleplayerButton	= nullptr;
	MenuItemLabel* StoreButton			= nullptr;
	MenuItemLabel* AccountButton		= nullptr;
	MenuItemLabel* OptionsButton		= nullptr;

	class ConnectingPopup* Popup	= nullptr;

	void OnlineCallback( Ref* Caller );
	void OnSingleplayerCallback( Ref* Caller );
	void OnStoreCallback( Ref* Caller );
	void OnAccountCallback( Ref* Caller );
	void OnOptionsCallback( Ref* Caller );

	void OnConnect( CloudEvent inEvent, int Paramater );
	void OnConnectFailure( CloudEvent inEvent, int Paramter );
	std::shared_ptr< EventListenerTouchOneByOne > TouchKiller;

	// Popup Menu Tracking
	bool _bPopupVisible = false;
	bool _bPopupIsError = false;
	bool _bLoginOpen	= false;

	void OnLostConnection( CloudEvent inEvent, int Paramater );
	void ShowConnectingMenu();
	void StartLoginProcess();

	LoginLayer* LoginPanel = nullptr;

	void OnReconnectBegin( CloudEvent inEvent, int Param );

};