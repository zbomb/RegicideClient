/*==========================================================================================
	Regicide Mobile

	MainMenuScene.h
	(c) 2018 Zachary Berry
=============================================================================================*/
#pragma once

#include "cocos2d.h"
#include "LoginLayer.h"
#include "EventDataTypes.h"
#include "RegisterLayer.h"
#include "EventHub.h"
#include "Numeric.h"
#include "UI/UpdatePrompt.hpp"

using namespace cocos2d;

class MainMenu : public cocos2d::Scene
{

public:
	
	// Cocos2d Implementation
	static cocos2d::Scene* createScene();
	virtual bool init();
	CREATE_FUNC( MainMenu );
    
    ~MainMenu();

    void CancelLogin();
    
    // Local Login State
    enum LoginState { LoggedOut, OfflineLogin, LoggedIn };
    LoginState CurrentState;
    inline LoginState GetLoginState() const { return CurrentState; }
    void SetLoginState( LoginState inState );
    
    bool PerformLogin( std::string Username, std::string Password );
    bool PerformRegister( std::string Username, std::string Password, std::string DisplayName, std::string EmailAddress );
    
    void CloseLoginMenu();
    void CloseRegisterMenu();
    
    void OpenRegisterMenu();
    void OpenLoginMenu();
    
    virtual void onEnterTransitionDidFinish();
    
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
										
	MenuItemLabel* OnlineButton			= nullptr;
	MenuItemLabel* SingleplayerButton	= nullptr;
	MenuItemLabel* StoreButton			= nullptr;
	MenuItemLabel* AccountButton		= nullptr;
	MenuItemLabel* OptionsButton		= nullptr;
    
    UpdatePrompt* updPrompt = nullptr;

	void OnlineCallback( Ref* Caller );
	void OnSingleplayerCallback( Ref* Caller );
	void OnStoreCallback( Ref* Caller );
	void OnAccountCallback( Ref* Caller );
	void OnOptionsCallback( Ref* Caller );
    
	// Popup Menu Tracking
	bool _bLoginOpen	= false;
    bool _bRegisterOpen = false;

	LoginLayer* LoginPanel = nullptr;
    RegisterLayer* RegisterPanel = nullptr;
    
    void ShowDisconnectedBanner();
    void HideDisconnectedBanner();
    
    DrawNode* BannerDraw = nullptr;
    Label* BannerLabel = nullptr;
    
    bool _bBannerOpen = false;
    bool _bCanClosePopup = true;
    
    EventListenerTouchOneByOne* BannerTouch = nullptr;
    void HandleBannerTouch( Touch* inTouch, Event* inEvent );
    bool HandleBannerTouchBegin( Touch* inTouch, Event* inEvent );
    
    void ShowError( std::string ErrorMessage );
    void ShowPopup( std::string inMessage, Color4B inColor, float inScale, bool bAllowClose );
    void HidePopup();
    
    DrawNode* PopupNode     = nullptr;
    Label* PopupLabel       = nullptr;
    std::shared_ptr< EventListenerTouchOneByOne > TouchHandler;
    

};
