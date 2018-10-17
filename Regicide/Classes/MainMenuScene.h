/*==========================================================================================
	Regicide Mobile

	MainMenuScene.h
	(c) 2018 Zachary Berry
=============================================================================================*/
#pragma once

#include "cocos2d.h"
#include "NetHeaders.h"
#include "LoginLayer.h"
#include "EventDataTypes.h"
#include "RegisterLayer.h"
#include "EventHub.h"

using namespace cocos2d;

class MainMenu : public cocos2d::Scene
{

public:
	
	// Cocos2d Implementation
	static cocos2d::Scene* createScene();
	virtual bool init();
	CREATE_FUNC( MainMenu );
    
    ~MainMenu();

	void CloseErrorMenu();
    void CancelLogin();
    
    enum LoginState { LoggedOut, OfflineLogin, LoggedIn };
    LoginState CurrentState;
    inline LoginState GetLoginState() const { return CurrentState; }
    void SetLoginState( LoginState inState );
    bool OnLogin( EventData* inData );
    bool OnRegister( EventData* inData );
    
    bool PerformLogin( std::string Username, std::string Password );
    bool PerformRegister( std::string Username, std::string Password, std::string DisplayName, std::string EmailAddress );
    void CloseLoginMenu();
    void CloseRegisterMenu();
    
    void OpenRegisterMenu();
    void OpenLoginMenu();
    
private:
    
    EventId DisconnectId    = EVENT_INVALID;
    EventId ConnectBeginId  = EVENT_INVALID;
    EventId ConnectEndId    = EVENT_INVALID;
    EventId LoginId         = EVENT_INVALID;
    EventId RegisterId      = EVENT_INVALID;

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
    
	bool OnConnect( EventData* inData );
	void OnConnectFailure( int Paramter );
    bool OnReconnectBegin( EventData* inData );
	std::shared_ptr< EventListenerTouchOneByOne > TouchKiller;
    
	// Popup Menu Tracking
	bool _bPopupVisible = false;
	bool _bPopupIsError = false;
	bool _bLoginOpen	= false;
    bool _bRegisterOpen = false;

	bool OnLostConnection( EventData* inData );
	void ShowConnectingMenu();
	void StartLoginProcess();

	LoginLayer* LoginPanel = nullptr;
    RegisterLayer* RegisterPanel = nullptr;

};
