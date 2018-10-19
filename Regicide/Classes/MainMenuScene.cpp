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
    
    if( TouchHandler )
    {
        _eventDispatcher->removeEventListener( &*TouchHandler );
        TouchHandler.reset();
    }
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
        ShowPopup( "Connecting...", Color4B( 240, 240, 240, 255 ), 1.f, false );
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
    
    BannerTouch = EventListenerTouchOneByOne::create();
    BannerTouch->onTouchEnded = CC_CALLBACK_2( MainMenu::HandleBannerTouch, this );
    BannerTouch->onTouchBegan = CC_CALLBACK_2( MainMenu::HandleBannerTouchBegin, this );
    _eventDispatcher->addEventListenerWithFixedPriority( BannerTouch, -10 );
    
	return true;
}

bool MainMenu::OnReconnectBegin( EventData* inData )
{
    ShowPopup( "Connecting...", Color4B( 240, 240, 240, 255 ), 1.f, false );
    
    RegCloud* Cloud = RegCloud::Get();
    SetLoginState( Cloud && Cloud->IsLoggedIn() ? LoginState::OfflineLogin : LoginState::LoggedOut );
    
    return true;
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
    
    // Hide any popups
    HidePopup();
    HideDisconnectedBanner();
    
    // Check if we need to open the login menu
    if( Cloud && !Cloud->IsLoggedIn() )
    {
        OpenLoginMenu();
    }
    
    return true;
}

void MainMenu::ShowPopup( std::string inMessage, Color4B inColor, float inScale, bool bAllowClose )
{
    auto ScreenPos = Director::getInstance()->getVisibleOrigin();
    auto ScreenSize = Director::getInstance()->getVisibleSize();
    
    float thisW = ScreenSize.width * 0.6f;
    float thisH = ScreenSize.height * 0.3f;
    float thisX = ScreenPos.x + ScreenSize.width * 0.2f;
    float thisY = ScreenPos.y + ScreenSize.height * 0.35f;
    
    float FontSize = thisW / 18.f;
    
    // Create the nodes if need be
    if( !PopupNode )
    {
        PopupNode = DrawNode::create();
        
        if( !PopupNode )
        {
            log( "[UI ERROR] Failed to create popup menu! Draw Node couldnt be created" );
            return;
        }
        
        // Draw background blur
        PopupNode->drawSolidRect( Vec2( ScreenPos.x, ScreenPos.y ), Vec2( ScreenPos.x + ScreenSize.width, ScreenPos.y + ScreenSize.height ), Color4F( 0.f, 0.f, 0.f, 0.5f ) );
        
        // Draw Outline
        PopupNode->drawSolidRect( Vec2( thisX, thisY ), Vec2( thisX + thisW, thisY + thisH ), Color4F( 0.4f, 0.4f, 0.4f, 1.f ) );
        
        // Draw Inner Box
        PopupNode->drawSolidRect( Vec2( thisX + 3.f, thisY + 3.f ), Vec2( thisX + thisW - 3.f, thisY + thisH - 3.f ), Color4F( 0.1f, 0.1f, 0.1f, 1.f ) );
        
        addChild( PopupNode, 20, "PopupNode" );
        PopupNode->runAction( Sequence::create( FadeIn::create( 0.3f ), NULL ) );
    }
    
    bool bNewLabel = false;
    if( !PopupLabel )
    {
        PopupLabel = Label::createWithTTF( inMessage, "fonts/arial.ttf", FontSize, Size( thisW - 10.f, thisH - 10.f ), TextHAlignment::CENTER, TextVAlignment::CENTER );
        
        if( !PopupLabel )
        {
            log( "[UI ERROR] Failed to create popup menu! Label Node couldnt be created" );
            
            // Creation failed! Cleanup popup node and stop
            if( PopupNode )
            {
                PopupNode->removeFromParentAndCleanup( true );
                PopupNode = nullptr;
            }
            
            return;
        }
        
        // Setup layout
        PopupLabel->setAnchorPoint( Vec2( 0.5f, 0.5f ) );
        PopupLabel->setPosition( Vec2( thisX + thisW / 2.f, thisY + thisH / 2.f ) );
        
        addChild( PopupLabel, 21, "PopupLabel" );
        PopupLabel->runAction( Sequence::create( FadeIn::create( 0.3f ), NULL ) );
        
        bNewLabel = true;
    }
    
    if( bNewLabel && PopupLabel )
        PopupLabel->runAction( Sequence::create( FadeOut::create( 0.2f ), NULL ) );
    
    if( PopupLabel )
    {
        // Set Label Properties
        PopupLabel->setString( inMessage );
        PopupLabel->setTextColor( inColor );
        PopupLabel->setScale( inScale );
    
        if( bNewLabel )
            PopupLabel->runAction( Sequence::create( FadeIn::create( 0.2f ), NULL ) );
    }
    
    // We need to create a new touch listener every time, since we cant update it
    if( TouchHandler )
    {
        _eventDispatcher->removeEventListener( &(*TouchHandler ) );
        TouchHandler.reset();
    }
    
    TouchHandler = std::make_shared< EventListenerTouchOneByOne >( *EventListenerTouchOneByOne::create() );
    if( !TouchHandler )
    {
        log( "[UI ERROR] Failed to create touch handler for popup menu! Closing automatically" );
        std::this_thread::sleep_for( std::chrono::milliseconds( 2000 ) );
        HidePopup();
        return;
    }

    // Ensure all touches are swallowed by the popup
    TouchHandler->setSwallowTouches( true );
    TouchHandler->onTouchBegan = [] ( Touch* inTouch, Event* inEvent ) { return true; };
    
    if( bAllowClose )
    {
        TouchHandler->onTouchEnded = [ this ] ( Touch* inTouch, Event* inEvent )
        {
            if( this )
                this->HidePopup();
        };
    }
    else
    {
        TouchHandler->onTouchEnded = [] ( Touch* inTouch, Event* inEvent ) {};
    }
    
    _eventDispatcher->addEventListenerWithFixedPriority( &(*TouchHandler), -100 );
}

void MainMenu::HidePopup()
{
    if( PopupNode )
    {
        PopupNode->removeFromParentAndCleanup( true );
        PopupNode = nullptr;
    }
    
    if( PopupLabel )
    {
        PopupLabel->removeFromParentAndCleanup( true );
        PopupLabel = nullptr;
    }
    
    if( TouchHandler )
    {
        _eventDispatcher->removeEventListener( &(*TouchHandler ) );
        TouchHandler.reset();
    }
}

void MainMenu::ShowError( std::string ErrorMessage )
{
    ShowPopup( ErrorMessage, Color4B( 240, 30, 30, 255 ), 0.65f, true );
}

void MainMenu::OnConnectFailure( int Parameter )
{
    std::string DetailedError;
    if( Parameter == (int) ConnectResult::ConnectionError )
        DetailedError = "check your internet connection";
    else if( Parameter == (int) ConnectResult::KeyExchangeError )
        DetailedError = "a crypto error occurred";
    else
        DetailedError = "an unknown error occurred";
    
    // Create popup error message
    ShowError( "Failed to connect to cloud, " + DetailedError + "! The game will run in offline-mode until you reconnect. Tap anywhere to close" );
    
    // Show the disconnected banner
    ShowDisconnectedBanner();
}

bool MainMenu::OnLostConnection( EventData* inData )
{
	log( "[UI] Connection to RegSys appears to be lost!" );
    
    // Check if were logged in still
    RegCloud* Cloud = RegCloud::Get();
    SetLoginState( Cloud && Cloud->IsLoggedIn() ? LoginState::OfflineLogin : LoginState::LoggedOut );
    
    // Close any open login or register menus
    CloseLoginMenu();
    CloseRegisterMenu();
    HidePopup();
    
    // Create the disconnected banner
    ShowDisconnectedBanner();
    
    // Show error message
    ShowError( "Lost connection to the Regicide Network!" );
    
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
    log( "ASDASDADSASDASDASDASDASDASDASDASDASDASD" );
    
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

void MainMenu::ShowDisconnectedBanner()
{
    if( _bBannerOpen )
        return;
    
    if( BannerDraw )
    {
        BannerDraw->removeFromParentAndCleanup( true );
        BannerDraw = nullptr;
    }
    
    if( BannerLabel )
    {
        BannerLabel->removeFromParentAndCleanup( true );
        BannerLabel = nullptr;
    }
    
    auto screenPos = Director::getInstance()->getVisibleOrigin();
    auto screenSiz = Director::getInstance()->getVisibleSize();
    
    float thisX = screenPos.x;
    float thisY = screenPos.y;
    float thisW = screenSiz.width;
    float thisH = screenSiz.height;
    
    float fontSize = thisH / 36.f;
    
    BannerDraw = DrawNode::create();
    if( BannerDraw )
    {
        BannerDraw->drawSolidRect(Vec2( thisX, thisY ), Vec2( thisX + thisW, thisY + 52 ), Color4F( 0.3f, 0.3f, 0.3f, 1.f ) );
        BannerDraw->drawSolidRect( Vec2( thisX, thisY ), Vec2( thisX + thisW, thisY + 50 ), Color4F( 0.1f, 0.1f, 0.1f, 1.f ) );
        addChild( BannerDraw, 15 );
    }
    
    BannerLabel = Label::createWithTTF( "Lost connection to the Regicide Network. Online features disabled. Click here to reconnect.", "fonts/arial.ttf", fontSize, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER );
    if( BannerLabel )
    {
        BannerLabel->setAnchorPoint( Vec2( 0.5f, 0.5f ) );
        BannerLabel->setPosition( Vec2( thisX + thisW / 2.f, thisY + 25 ) );
        BannerLabel->setTextColor( Color4B( 235, 235, 235, 255 ) );
        addChild( BannerLabel, 16 );
    }
    
    _bBannerOpen = true;
 }

void MainMenu::HideDisconnectedBanner()
{
    if( !_bBannerOpen )
        return;
    
    if( BannerDraw )
    {
        BannerDraw->removeFromParentAndCleanup( true );
        BannerDraw = nullptr;
    }
    
    if( BannerLabel )
    {
        BannerLabel->removeFromParentAndCleanup( true );
        BannerLabel = nullptr;
    }
    
    _bBannerOpen = false;
}

void MainMenu::HandleBannerTouch( Touch* inTouch, Event *inEvent )
{
    if( !_bBannerOpen )
        return;
    
    auto screenPos = Director::getInstance()->getVisibleOrigin();
    float thisY = screenPos.y;
    
    auto touchLocation = inTouch->getLocation();
    
    if( touchLocation.y <= thisY + 52 )
    {
        // Clicked on the banner!
        RegCloud* Cloud = RegCloud::Get();
        if( !Cloud )
        {
            log( "[RegCloud] Failed to get RegCloud singleton! Can not reconnect to the network." );
            return;
        }
        
        if( !Cloud->Reconnect() )
        {
            // Show error message
            log( "[RegSys] Failed to attempt reconnect with the Regicide Network. Restart game and try again" );
            ShowError( "Failed to start reconnect. Restart game and try again." );
        }
    }
}

bool MainMenu::HandleBannerTouchBegin( Touch *inTouch, Event *inEvent )
{
    if( !_bBannerOpen )
        return false;
    
    auto screenPos = Director::getInstance()->getVisibleOrigin();
    float thisY = screenPos.y;
    
    auto touchLocation = inTouch->getLocation();
    
    if( touchLocation.y <= thisY + 52 )
    {
        return true;
    }
    
    return false;
}
