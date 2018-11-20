//
//    MainMenuScene.cpp
//    Regicide Mobile
//
//    Created: 10/9/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#include "MainMenuScene.hpp"
#include "SimpleAudioEngine.h"
#include "LoginLayer.hpp"
#include "EventHub.hpp"
#include "Utils.hpp"
#include "API.hpp"
#include "IContentSystem.hpp"
#include "OptionsScene.hpp"
#include "SingleplayerLauncher.hpp"
#include "OnlineLauncher.hpp"


using namespace cocos2d;
using namespace cocos2d::ui;
using namespace Regicide;


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
    
    // Free unused textures
    auto Cache = Director::getInstance()->getTextureCache();
    Cache->removeUnusedTextures();

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
	
    BannerTouch = EventListenerTouchOneByOne::create();
    BannerTouch->onTouchEnded = CC_CALLBACK_2( MainMenu::HandleBannerTouch, this );
    BannerTouch->onTouchBegan = CC_CALLBACK_2( MainMenu::HandleBannerTouchBegin, this );
    _eventDispatcher->addEventListenerWithFixedPriority( BannerTouch, -10 );
    
    // Were going to check if the user is already logged in
    auto act = IContentSystem::GetAccounts();
    if( act->IsLoginStored() )
    {
        SetLoginState( LoginState::LoggedIn );
    }
    else
    {
        SetLoginState( LoginState::LoggedOut );
    }
    
	return true;
}

void MainMenu::onEnterTransitionDidFinish()
{
    // First, check if the content was cleared manually
    auto storage = IContentSystem::GetStorage();
    if( storage->WasContentCleared() && !updPrompt )
    {
        updPrompt = UpdatePrompt::create();
        this->addChild( updPrompt, 100 );
    }
    else
    {
        // Check if the login menu needs to be opened
        auto act = IContentSystem::GetAccounts();
        if( !act->IsLoginStored() )
        {
            OpenLoginMenu();
            SetLoginState( LoginState::LoggedOut );
        }
        else
        {
            SetLoginState( LoginState::LoggedIn );
        }
    }
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



void MainMenu::OnlineCallback( Ref* Caller )
{
    Director::getInstance()->pushScene( TransitionSlideInL::create( 0.5f, OnlineLauncherScene::createScene() ) );
}

void MainMenu::OnStoreCallback( Ref* Caller )
{

}

void MainMenu::OnSingleplayerCallback( Ref* Caller )
{
    Director::getInstance()->pushScene( TransitionSlideInL::create( 0.5f, SingleplayerLauncherScene::createScene() ) );
}

void MainMenu::OnAccountCallback( Ref* Caller )
{

}

void MainMenu::OnOptionsCallback( Ref* Caller )
{
    Director::getInstance()->pushScene( TransitionSlideInL::create( 0.5f, OptionsScene::createScene() ) );
}


bool MainMenu::PerformLogin( std::string Username, std::string Password )
{
    auto Client = APIClient::GetInstance();
    
    if( !Client )
    {
        log( "[Menu] Failed to get API Client! Login failed" );
        return false;
    }
    
    LoginRequest Request;
    Request.Username = Username;
    Request.Password = Password;
    
    return Client->LoginAsync( Request, [ this ]( LoginResponse Response )
       {
           if( this )
           {
               if( Response.Result == LoginResult::BadRequest )
               {
                   if( this->LoginPanel ) this->LoginPanel->ShowError( "Please check input and try again" );
               }
               else if( Response.Result == LoginResult::DatabaseError ||
                       Response.Result == LoginResult::OtherError )
               {
                   if( this->LoginPanel ) this->LoginPanel->ShowError( "An error has occured. Please retry momentarily" );
               }
               else if( Response.Result != LoginResult::Success )
               {
                   if( this->LoginPanel ) this->LoginPanel->ShowError( "Invalid Username/Password. Please retry" );
               }
               else
               {
                   // Update Login Status
                   this->SetLoginState( LoginState::LoggedIn );
                   
                   // Close Login Panel
                   this->CloseLoginMenu();
               }
           }
       } );
}

bool MainMenu::PerformRegister( std::string Username, std::string Password, std::string DisplayName, std::string EmailAddress )
{
    auto Client = APIClient::GetInstance();
    
    if( !Client )
    {
        log( "[Menu] Failed to get API Client! Register failed" );
        return false;
    }
    
    RegisterRequest Request;
    Request.Username = Username;
    Request.Password = Password;
    Request.DispName = DisplayName;
    Request.EmailAdr = EmailAddress;
    
    return Client->RegisterAsync( Request, [ this ]( RegisterResponse Response )
         {
             if( this )
             {
                 if( Response.Result == RegisterResult::BadPassHash ||
                    Response.Result == RegisterResult::Error )
                 {     if( RegisterPanel ) RegisterPanel->ShowError( "Please recheck input and try again!" ); }
                else if( Response.Result == RegisterResult::EmailExists )
                { if( RegisterPanel ) RegisterPanel->ShowError( "Email address already exists" ); }
                else if( Response.Result == RegisterResult::InvalidDispName )
                { if( RegisterPanel ) RegisterPanel->ShowError( "Invalid Display Name" ); }
                else if( Response.Result == RegisterResult::InvalidEmail )
                { if( RegisterPanel ) RegisterPanel->ShowError( "Invalid Email" ); }
                else if( Response.Result == RegisterResult::InvalidUsername )
                { if( RegisterPanel ) RegisterPanel->ShowError( "Invalid Username" ); }
                else if( Response.Result == RegisterResult::SuccessBadResponse )
                {
                        this->CloseRegisterMenu();
                        this->OpenLoginMenu();
                        if( this->LoginPanel )
                            this->LoginPanel->ShowError( "Account was registered, but an error occured. Please manually log in" );
                }
                 else
                 {
                    // Update Login Status
                     this->SetLoginState( LoginState::LoggedIn );
                     
                     // Close Login Panel
                     this->CloseRegisterMenu();
                 }
             }
         });
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
    CloseLoginMenu();
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
        /*
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
         */
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
