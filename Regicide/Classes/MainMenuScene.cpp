/*==========================================================================================
	Regicide Mobile

	MainMenuScene.cpp
	(c) 2018 Zachary Berry
=============================================================================================*/

#include "MainMenuScene.h"
#include "SimpleAudioEngine.h"
#include "RegCloud.h"
#include "LoginLayer.h"
#include "EventHub.h"


using namespace cocos2d;
using namespace cocos2d::ui;


Scene* MainMenu::createScene()
{
	auto ret = MainMenu::create();
	if( ret ) { ret->setName( "MainMenu" ); }
	return ret;
}

MainMenu::~MainMenu()
{
    EVENT_SAFE_UNBIND( ConnectBeginId );
    EVENT_SAFE_UNBIND( ConnectEndId );
    EVENT_SAFE_UNBIND( DisconnectId );
    EVENT_SAFE_UNBIND( LoginId );
    EVENT_SAFE_UNBIND( RegisterId );
}

bool MainMenu::init()
{
	// Initialize Parent
	if( !Scene::init() )
	{
		return false;
	}

	auto sceneSize = Director::getInstance()->getVisibleSize();
	auto sceneOrigin = Director::getInstance()->getVisibleOrigin();
	float Padding = sceneSize.width / 50.f;

	Background = Sprite::create( "menu_background.png" );
	if( !Background )
	{
		log( "[UI ERROR] Failed to load Menu Background Image!" );
	}
	else
	{	
		auto ImageSize = Background->getContentSize();

		float ScaleX = sceneSize.width / ImageSize.width;
		float ScaleY = sceneSize.height / ImageSize.height;

		Background->setStretchEnabled( true );
		Background->setScale( ScaleX > ScaleY ? ScaleX : ScaleY );
		Background->setPosition( Vec2( sceneOrigin.x + sceneSize.width / 2.f,
										sceneOrigin.y + sceneSize.height / 2.f ) );

		this->addChild( Background, 0, "Background" );

	}

	// Determine font size for header
	int HeaderFontSize = ( sceneSize.width / 1920.f ) * 86.f;

	// Header
	Header = Label::createWithTTF( "Regicide", "fonts/Ringbearer Medium.ttf", HeaderFontSize );
	if( !Header )
	{
		log( "[UI ERROR] Failed to load Ringbearer Medium font!" );
 	}
	else
	{
		Header->setTextColor( Color4B( 240, 240, 240, 255 ) );
		Header->setPosition( Vec2(  sceneOrigin.x + sceneSize.width / 2.f,
							 		sceneOrigin.y + sceneSize.height - Header->getContentSize().height - Padding ) );

		this->addChild( Header, 2, "Header" );
	}

	HeaderBox = DrawNode::create();
	if( HeaderBox && Header )
	{
		float HeaderHeight = Header->getContentSize().height;
		float HeaderBoxPadding = 3.f;

		HeaderBox->drawSolidRect( Vec2( sceneOrigin.x, sceneOrigin.y + sceneSize.height - Padding - HeaderHeight * 0.5f + HeaderBoxPadding ),
								Vec2( sceneOrigin.x + sceneSize.width, sceneOrigin.y + sceneSize.height - Padding - ( HeaderHeight * 1.5f ) - HeaderBoxPadding ),
								Color4F( 0.3f, 0.3f, 0.3f, 1.f ) );

		HeaderBox->drawSolidRect( Vec2( sceneOrigin.x, sceneOrigin.y + sceneSize.height - Padding - HeaderHeight * 0.5f ), 
								Vec2( sceneOrigin.x + sceneSize.width, sceneOrigin.y + sceneSize.height - Padding - HeaderHeight * 1.5f ), 
								Color4F( 0.1f, 0.1f, 0.1f, 1.f ) );

		this->addChild( HeaderBox, 1, "HeaderBox" );
	}
	else
	{
		log( "[UI ERROR] Failed to load header box!" );
	}

	/*======================================================
		Menu Container and Items
	======================================================*/
	cocos2d::Vector< MenuItem* > MenuItems;

	// Online Button
	auto OnlineLabel = Label::createWithTTF( "Online Play", "fonts/arial.ttf", HeaderFontSize * 0.6f );
	if( OnlineLabel )
	{
		OnlineLabel->setTextColor( Color4B( 240, 240, 240, 255 ) );

		OnlineButton = MenuItemLabel::create( OnlineLabel, CC_CALLBACK_1( MainMenu::OnlineCallback, this ) );

		if( OnlineButton )
		{
			OnlineButton->setAnchorPoint( Vec2( 0.f, 0.5f ) );
			OnlineButton->setPosition( Vec2( sceneOrigin.x + Padding, sceneOrigin.y + sceneSize.height * 0.7f ) );

			MenuItems.pushBack( OnlineButton );
		}
		else
		{
			log( "[UI ERROR] Failed to create online button from label" );
		}
	}
	else
	{
		log( "[UI ERROR] Failed to create online button!" );
	}

	auto SingleplayerLabel = Label::createWithTTF( "Singleplayer", "fonts/arial.ttf", HeaderFontSize * 0.6f );
	if( SingleplayerLabel )
	{
		SingleplayerLabel->setTextColor( Color4B( 240, 240, 240, 255 ) );

		SingleplayerButton = MenuItemLabel::create( SingleplayerLabel, CC_CALLBACK_1( MainMenu::OnSingleplayerCallback, this ) );

		if( SingleplayerButton )
		{ 
			SingleplayerButton->setAnchorPoint( Vec2( 0.f, 0.5f ) );
			SingleplayerButton->setPosition( Vec2( sceneOrigin.x + Padding, sceneOrigin.y + sceneSize.height * 0.55f ) );

			MenuItems.pushBack( SingleplayerButton );
		}
		else
		{
			log( "[UI ERROR] Failed to create singleplayer menu item from label!" );
		}
	}
	else
	{
		log( "[UI ERROR] Failed to create singleplayer label!" );
	}

	auto StoreLabel = Label::createWithTTF( "Store", "fonts/arial.ttf", HeaderFontSize * 0.6f );
	if( StoreLabel )
	{
		StoreLabel->setTextColor( Color4B( 240, 240, 240, 255 ) );

		StoreButton = MenuItemLabel::create( StoreLabel, CC_CALLBACK_1( MainMenu::OnStoreCallback, this ) );

		if( StoreButton )
		{
			StoreButton->setAnchorPoint( Vec2( 0.f, 0.5f ) );
			StoreButton->setPosition( Vec2( sceneOrigin.x + Padding, sceneOrigin.y + sceneSize.height * 0.4f ) );

			MenuItems.pushBack( StoreButton );
		}
		else
		{
			log( "[UI ERROR] Failed to create store button from label!" );
		}
	}
	else
	{
		log( "[UI ERROR] Failed to create store label!" );
	}

	auto AccountLabel = Label::createWithTTF( "Account & Cards", "fonts/arial.ttf", HeaderFontSize * 0.6f );
	if( AccountLabel )
	{
		AccountLabel->setTextColor( Color4B( 240, 240, 240, 255 ) );
		AccountButton = MenuItemLabel::create( AccountLabel, CC_CALLBACK_1( MainMenu::OnAccountCallback, this ) );

		if( AccountButton )
		{
			AccountButton->setAnchorPoint( Vec2( 0.f, 0.5f ) );
			AccountButton->setPosition( Vec2( sceneOrigin.x + Padding, sceneOrigin.y + sceneSize.height * 0.25f ) );

			MenuItems.pushBack( AccountButton );
		}
		else
		{
			log( "[UI ERROR] Failed to create account button from label!" );
		}
	}
	else
	{
		log( "[UI ERROR] Failed to create label for account button!" );
	}

	auto OptionsLabel = Label::createWithTTF( "Options", "fonts/arial.ttf", HeaderFontSize * 0.6f );
	if( OptionsLabel )
	{
		OptionsLabel->setTextColor( Color4B( 240, 240, 240, 255 ) );
		OptionsButton = MenuItemLabel::create( OptionsLabel, CC_CALLBACK_1( MainMenu::OnOptionsCallback, this ) );

		if( OptionsButton )
		{
			OptionsButton->setAnchorPoint( Vec2( 0.f, 0.5f ) );
			OptionsButton->setPosition( Vec2( sceneOrigin.x + Padding, sceneOrigin.y + sceneSize.height * 0.1f ) );

			MenuItems.pushBack( OptionsButton );
		}
		else
		{
			log( "[UI ERROR] Failed to create options button from label!" );
		}
	}
	else
	{
		log( "[UI ERROR] Failed to create label for options button!" );
	}

#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32 || CC_TARGET_PLATFORM == CC_PLATFORM_MAC

	auto ExitButton = MenuItemImage::create( "CloseNormal.png", "CloseSelected.png", "CloseSelected.png", [ this ]( Ref* Caller )
	{
		Director::getInstance()->end();
	} );

	if( ExitButton && ExitButton->getContentSize().width > 0 )
	{
		float x = sceneOrigin.x + sceneSize.width - ExitButton->getContentSize().width / 2;
		float y = sceneOrigin.y + ExitButton->getContentSize().height / 2;
		ExitButton->setPosition( Vec2( x, y ) );

		MenuItems.pushBack( ExitButton );
	}
	else
	{
		log( "[UI ERROR] Failed to create debug exit button" );
	}

#endif
    
	MenuContainer = Menu::createWithArray( MenuItems );
	if( MenuContainer )
	{
		MenuContainer->setPosition( Vec2::ZERO );
		this->addChild( MenuContainer, 5, "MenuContainer" );
	}
	else
	{
		log( "[UI ERROR] Failed to create menu container" );
		return false;
	}

	RegCloud* RegSys = RegCloud::Get();
	
	// If we cant get a ref to RegSys, run in offline mode
	if( !RegSys )
	{
        NumericEventData Data( (int) ConnectResult::OtherError );
        OnLostConnection( &Data );
		return true;
	}
	
	// Based on the current status of RegSys, we will show different UI elements
	ENodeProcessState ConnectionState = RegSys->GetConnectionState();

	// Bind hooks right after checking state, so if we end up connecting before the menu is constructed, the state will be double
	// checked after construction and the situation will be detected, and the same applies if the connection happens in between this line and the last line of code
    using namespace std::placeholders;
    
    ConnectEndId    = EventHub::Bind( "ConnectResult", std::bind( &MainMenu::OnConnect, this, _1 ), CallbackThread::Game );
    DisconnectId    = EventHub::Bind( "Disconnect", std::bind( &MainMenu::OnLostConnection, this, _1 ), CallbackThread::Game );
    ConnectBeginId  = EventHub::Bind( "ConnectBegin", std::bind( &MainMenu::OnReconnectBegin, this, _1 ), CallbackThread::Game );
    LoginId         = EventHub::Bind( "LoginResult", std::bind( &MainMenu::OnLogin, this, _1 ), CallbackThread::Game );
    RegisterId      = EventHub::Bind( "RegisterResult", std::bind( &MainMenu::OnRegister, this, _1 ), CallbackThread::Game );
    
	if( ConnectionState == ENodeProcessState::InProgress || ConnectionState == ENodeProcessState::NotStarted )
	{
        SetLoginState( RegSys->IsLoggedIn() ? LoginState::OfflineLogin : LoginState::LoggedOut );
		ShowConnectingMenu();
	}
	else if( ConnectionState == ENodeProcessState::Reset )
	{
		// Connection Error!
        OnConnectFailure( (int) ConnectResult::ConnectionError );
	}
	else
	{
		// Already Connected!
        SetLoginState( RegSys->IsLoggedIn() ? LoginState::OfflineLogin : LoginState::LoggedOut );
		StartLoginProcess();
	}

	return true;
}

bool MainMenu::OnReconnectBegin( EventData* inData )
{
	// Were just going to call ShowConnectingMenu()
	if( !_bPopupVisible ||
		( _bPopupVisible && !_bPopupIsError ) )
	{
		ShowConnectingMenu();
	}
    
    RegCloud* Cloud = RegCloud::Get();
    SetLoginState( Cloud && Cloud->IsLoggedIn() ? LoginState::OfflineLogin : LoginState::LoggedOut );
    
    return true;
}

void MainMenu::ShowConnectingMenu()
{
	auto sceneOrigin = Director::getInstance()->getVisibleOrigin();
	auto sceneSize = Director::getInstance()->getVisibleSize();
	int HeaderFontSize = ( sceneSize.width / 1920.f ) * 86.f;

	// Check if the popup is valid
	DrawNode* _Connecting = nullptr;
	Label* _ConnectingLabel = nullptr;

	// Construct the nodes if needed
	if( !Connecting )
	{
		_Connecting = DrawNode::create();

		if( _Connecting )
		{
			_Connecting->drawSolidRect( Vec2( sceneOrigin.x, sceneOrigin.y ), Vec2( sceneOrigin.x + sceneSize.width, sceneOrigin.y + sceneSize.height ), Color4F( 0.f, 0.f, 0.f, .8f ) );
			_Connecting->drawSolidRect( Vec2( sceneOrigin.x + sceneSize.width * 0.2f, sceneOrigin.y + sceneSize.height * 0.35f ),
										Vec2( sceneOrigin.x + sceneSize.width * 0.8f, sceneOrigin.y + sceneSize.height * 0.65f ),
										Color4F( 0.35f, 0.35f, 0.35f, 1.f ) );
			_Connecting->drawSolidRect( Vec2( sceneOrigin.x + sceneSize.width * 0.2f + 4, sceneOrigin.y + sceneSize.height * 0.35f + 4 ),
										Vec2( sceneOrigin.x + sceneSize.width * 0.8f - 4, sceneOrigin.y + sceneSize.height * 0.65f - 4 ),
										Color4F( 0.15f, 0.15f, 0.15f, 1.f ) );

			this->addChild( _Connecting, 10, "ConnectingBox" );
		}
	}

	if( !ConnectingLabel )
	{
		_ConnectingLabel = Label::createWithTTF( "Connecting...", "fonts/arial.ttf", HeaderFontSize,
													  Size( sceneSize.width, sceneSize.height * 0.3f ),
													  TextHAlignment::CENTER, TextVAlignment::CENTER );
		if( _ConnectingLabel )
		{
			_ConnectingLabel->setOverflow( Label::Overflow::RESIZE_HEIGHT );
			_ConnectingLabel->setPosition( Vec2( sceneOrigin.x + sceneSize.width / 2.f, sceneOrigin.y + sceneSize.height / 2.f ) );
			this->addChild( _ConnectingLabel, 11, "ConnectingLabel" );
		}
	}

	// Set Touch Event Listener
	// First, remove any linked listeners
	_eventDispatcher->removeEventListenersForTarget( this, false );

	if( !TouchKiller )
	{
		TouchKiller = std::make_shared< EventListenerTouchOneByOne >( *EventListenerTouchOneByOne::create() );
	}

	if( TouchKiller )
	{
		TouchKiller->onTouchBegan = [ = ]( Touch* inTouch, Event* inEvent )
		{
			return true;
		};

		TouchKiller->setSwallowTouches( true );
		_eventDispatcher->addEventListenerWithFixedPriority( &(*TouchKiller), -100 );
	}

	// Set node refs
	if( _ConnectingLabel )
	{
		ConnectingLabel = _ConnectingLabel;
	}
	else
	{
		// If there was already a ConnectingLabel, then we will just update the text
		ConnectingLabel->setString( "Connecting..." );
		ConnectingLabel->setScale( 1.f );
	}

	if( _Connecting )
	{
		Connecting = _Connecting;
	}

	_bPopupIsError = false;
	_bPopupVisible = true;

	RegCloud* RegSys = RegCloud::Get();

	// Check if RegSys in null
	if( !RegSys )
	{
		return OnConnectFailure( (int) ConnectResult::OtherError );
	}

	// Check if the connection occurred already, if so, there is a possibility it was missed
	auto CurrentState = RegSys->GetConnectionState();
	if( CurrentState == ENodeProcessState::Complete )
	{
        NumericEventData Data( (int) ConnectResult::Success );
        OnConnect( &Data );
	}
	else if( CurrentState == ENodeProcessState::Reset )
	{
		OnConnectFailure( (int) ConnectResult::ConnectionError );
	}
}


bool MainMenu::OnConnect( EventData* inData )
{
    NumericEventData* numData = (NumericEventData*) inData;
    RegCloud* Cloud = RegCloud::Get();
    
	// Check for failure
	if( numData->Data != (int) ConnectResult::Success )
	{
        SetLoginState( Cloud && Cloud->IsLoggedIn() ? LoginState::OfflineLogin : LoginState::LoggedOut );
        OnConnectFailure( numData->Data );
        return true;
	}
    
    SetLoginState( Cloud && Cloud->IsLoggedIn() ? LoginState::LoggedIn : LoginState::LoggedOut );

	if( Connecting && _bPopupVisible )
	{
		_bPopupIsError = false;
		_bPopupVisible = false;

		Connecting->runAction( Sequence::create( FadeOut::create( 1.f ), NULL ) );

		if( ConnectingLabel )
		{
			ConnectingLabel->runAction( Sequence::create( FadeOut::create( 1.f ), NULL ) );
		}

		if( TouchKiller )
		{
			_eventDispatcher->removeEventListener( &(*TouchKiller) );

			//TouchKiller->release();
			TouchKiller.reset();
		}

		// Begin login logic
		StartLoginProcess();
	}
    
    return true;
}

void MainMenu::OnConnectFailure( int Parameter )
{
	log( "[UI DEBUG] ON CONNECT FAILURE" );
    
	if( Connecting && !_bPopupIsError && _bPopupVisible )
	{
		_bPopupIsError = true;
        
		// Animate into this error message
		if( ConnectingLabel )
		{
			std::string DetailedError;
			if( Parameter == (int) ConnectResult::ConnectionError )
				DetailedError = "check your internet connection";
			else if( Parameter == (int) ConnectResult::KeyExchangeError )
				DetailedError = "a crypto error occurred";
			else
				DetailedError = "an unknown error occurred";

			ConnectingLabel->runAction( FadeOut::create( 0.5f ) );
			std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
			ConnectingLabel->setScale( 0.5f );
			ConnectingLabel->setTextColor( Color4B( 220, 30, 30, 255 ) );
			ConnectingLabel->setString( "Failed to connect to cloud, " + DetailedError + "! The game will run in offline-mode until you choose to reconnect. Click to close." );
			ConnectingLabel->runAction( FadeIn::create( 0.5f ) );
		}

		// Create touch listener if needed
		if( !TouchKiller )
		{
			TouchKiller = std::make_shared< EventListenerTouchOneByOne >( *EventListenerTouchOneByOne::create() );
		}
		
		if( TouchKiller )
		{
			TouchKiller->onTouchBegan = [ this ]( Touch* inTouch, Event* inEvent )
			{
				if( this )
				{
					this->CloseErrorMenu();
				}

				return true;
			};
		}
		else
		{
			// Wait for 2 seconds before removing this popup message, since we cant bind to a touch event
			std::this_thread::sleep_for( std::chrono::milliseconds( 2000 ) );
			CloseErrorMenu();
		}
	}
}

void MainMenu::CloseErrorMenu()
{
	_bPopupVisible = false;
	
	// Fade out error menu
	if( ConnectingLabel )
	{
		ConnectingLabel->runAction( Sequence::create( FadeOut::create( 1.f ), NULL ) );
	}

	if( Connecting )
	{
		Connecting->runAction( Sequence::create( FadeOut::create( 1.f ), NULL ) );
	}

	// Stop consuming touch events
	if( TouchKiller )
	{
		_eventDispatcher->removeEventListener( &(*TouchKiller ) );
		//TouchKiller->release();
		//TouchKiller.reset();
	}

}

bool MainMenu::OnLostConnection( EventData* inData )
{
	log( "[UI] Connection to RegSys appears to be lost!" );
    
    // Check if were logged in still
    RegCloud* Cloud = RegCloud::Get();

    SetLoginState( Cloud && Cloud->IsLoggedIn() ? LoginState::OfflineLogin : LoginState::LoggedOut );
    return true;
}

void MainMenu::OnlineCallback( Ref* Caller )
{

}

void MainMenu::OnStoreCallback( Ref* Caller )
{

}

void MainMenu::OnSingleplayerCallback( Ref* Caller )
{

}

void MainMenu::OnAccountCallback( Ref* Caller )
{

}

void MainMenu::OnOptionsCallback( Ref* Caller )
{

}

void MainMenu::StartLoginProcess()
{
	auto RegSys = RegCloud::Get();

	if( !RegSys || !RegSys->IsSecure() )
	{
		// CRITICAL ERROR
        NumericEventData Data( (int) ConnectResult::ConnectionError );
        OnLostConnection( &Data );
        return;
	}

	bool bIsAccountStored = false;
	if( bIsAccountStored )
	{

	}
	else
	{
        OpenLoginMenu();
	}
}

bool MainMenu::PerformLogin( std::string Username, std::string Password )
{
    RegCloud* Cloud = RegCloud::Get();
    
    if( !Cloud )
    {
        log( "[RegLogin] Critical Error! RegCloud singleton is null!" );
        return false;
    }
    
    Cloud->Login( Username, Password );
    return true;
}

bool MainMenu::PerformRegister( std::string Username, std::string Password, std::string DisplayName, std::string EmailAddress )
{
    RegCloud* Cloud = RegCloud::Get();
    
    if( !Cloud )
    {
        log( "[RegLogin] Critical Error! RegCloud singleton is null!" );
        return false;
    }
    
    Cloud->Register( Username, Password, DisplayName, EmailAddress );
    return true;
}

void MainMenu::CloseLoginMenu()
{
    if( _bLoginOpen && LoginPanel )
    {
        LoginPanel->Destroy();
        
        if( LoginPanel )
        {
            LoginPanel->removeFromParentAndCleanup( true );
            LoginPanel = nullptr;
        }
        _bLoginOpen = false;
    }
}

void MainMenu::CancelLogin()
{
    RegCloud* RegSys = RegCloud::Get();
    SetLoginState( RegSys->IsLoggedIn() ? LoginState::OfflineLogin : LoginState::LoggedOut );
    CloseLoginMenu();
}

bool MainMenu::OnLogin( EventData* inData )
{
    LoginEventData* Result = static_cast< LoginEventData* >( inData );
    
    if( !Result || Result->Result != LoginResult::Success )
    {
        SetLoginState( LoginState::LoggedOut );
        if( _bLoginOpen && LoginPanel )
            LoginPanel->OnLoginFailure( Result->Result );
    }
    else
    {
        SetLoginState( LoginState::LoggedIn );
        CloseLoginMenu();
    }

    return true;
}

bool MainMenu::OnRegister( EventData *inData )
{
    NumericEventData* Result = static_cast< NumericEventData* >( inData );
    
    if( !Result || Result->Data != (int) ERegisterResult::Success )
    {        if( _bRegisterOpen && RegisterPanel )
            RegisterPanel->OnRegisterFailure( Result->Data );
    }
    else if( Result->Data == (int) ERegisterResult::SuccessWithInvalidPacket )
    {
        // This means that the account was created, but we didnt download it properly,
        // so we will force the user to relogin
        OpenLoginMenu();
        if( LoginPanel )
            LoginPanel->ShowError( "Account was created.. but the new account couldnt be downloaded. Please login to your new account to continue" );
    }
    else
    {
        SetLoginState( LoginState::LoggedIn );
        CloseRegisterMenu();
    }
    
    return true;
}

void MainMenu::SetLoginState( LoginState inState )
{
    CurrentState = inState;
    
    if( inState == LoginState::OfflineLogin )
    {
        OnlineButton->setEnabled( false );
        StoreButton->setEnabled( false );
        SingleplayerButton->setEnabled( true );
        AccountButton->setEnabled( true );
        OptionsButton->setEnabled( true );
    }
    else if( inState == LoginState::LoggedOut )
    {
        OnlineButton->setEnabled( false );
        StoreButton->setEnabled( false );
        SingleplayerButton->setEnabled( false );
        AccountButton->setEnabled( false );
        OptionsButton->setEnabled( false );
    }
    else
    {
        OnlineButton->setEnabled( true );
        StoreButton->setEnabled( true );
        SingleplayerButton->setEnabled( true );
        AccountButton->setEnabled( true );
        OptionsButton->setEnabled( true );
    }
}

void MainMenu::OpenRegisterMenu()
{
    // Close Menus
    CloseLoginMenu();
    
    if( !_bRegisterOpen )
    {
        if( RegisterPanel )
        {
            RegisterPanel->removeFromParentAndCleanup( true );
            RegisterPanel = nullptr;
        }
        
        RegisterPanel = RegisterLayer::create();
        addChild( RegisterPanel, 15, "RegisterPanel" );
        _bRegisterOpen = true;
    }
}

void MainMenu::OpenLoginMenu()
{
    // Close Menus
    CloseRegisterMenu();
    
    if( !_bLoginOpen )
    {
        if( LoginPanel )
        {
            LoginPanel->removeFromParentAndCleanup( true );
            LoginPanel = nullptr;
        }
        
        LoginPanel = LoginLayer::create();
        addChild( LoginPanel, 15, "LoginPanel" );
        _bLoginOpen = true;
    }
}

void MainMenu::CloseRegisterMenu()
{
    if( _bRegisterOpen && RegisterPanel )
    {
        RegisterPanel->Destroy();
        
        if( RegisterPanel )
        {
            RegisterPanel->removeFromParentAndCleanup( true );
            RegisterPanel = nullptr;
        }
        _bRegisterOpen = false;
    }
}
