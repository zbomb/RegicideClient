/*==========================================================================================
	Regicide Mobile

	MainMenuScene.cpp
	(c) 2018 Zachary Berry
=============================================================================================*/

#include "MainMenuScene.h"
#include "SimpleAudioEngine.h"
#include "RegCloud.h"
#include "LoginLayer.h"


using namespace cocos2d;
using namespace cocos2d::ui;


Scene* MainMenu::createScene()
{
	auto ret = MainMenu::create();
	if( ret ) { ret->setName( "MainMenu" ); }
	return ret;
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

	auto DebugBox = ui::EditBox::create( Size( 500.f, 50.f ), "editbox_bg.png" );

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
		OnLostConnection( CloudEvent::ConnectResult, (int) ConnectResult::OtherError );
		return true;
	}
	
	// Based on the current status of RegSys, we will show different UI elements
	ENodeProcessState ConnectionState = RegSys->GetConnectionState();

	// Bind hooks right after checking state, so if we end up connecting before the menu is constructed, the state will be double
	// checked after construction and the situation will be detected, and the same applies if the connection happens in between this line and the last line of code
	RegSys->BindEvent( CloudEvent::ConnectResult, CC_CALLBACK_2( MainMenu::OnConnect, this ) );
	RegSys->BindEvent( CloudEvent::Disconnect, CC_CALLBACK_2( MainMenu::OnLostConnection, this ) );
	RegSys->BindEvent( CloudEvent::ConnectBegin, CC_CALLBACK_2( MainMenu::OnReconnectBegin, this ) );

	if( ConnectionState == ENodeProcessState::InProgress || ConnectionState == ENodeProcessState::NotStarted )
	{
		ShowConnectingMenu();
	}
	else if( ConnectionState == ENodeProcessState::Reset )
	{
		// Connection Error!
		OnConnectFailure( CloudEvent::ConnectResult, (int) EConnectResult::ConnectionFailure );
	}
	else
	{
		// Already Connected!
		StartLoginProcess();
	}

	return true;
}

void MainMenu::OnReconnectBegin( CloudEvent inEvent, int Param )
{
	// Were just going to call ShowConnectingMenu()
	if( !_bPopupVisible ||
		( _bPopupVisible && !_bPopupIsError ) )
	{
		ShowConnectingMenu();
	}
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
		return OnConnectFailure( CloudEvent::ConnectResult, (int) ConnectResult::OtherError );
	}

	// Check if the connection occurred already, if so, there is a possibility it was missed
	auto CurrentState = RegSys->GetConnectionState();
	if( CurrentState == ENodeProcessState::Complete )
	{
		OnConnect( CloudEvent::ConnectResult, (int) ConnectResult::Success );
	}
	else if( CurrentState == ENodeProcessState::Reset )
	{
		OnConnectFailure( CloudEvent::ConnectResult, (int) ConnectResult::ConnectionError );
	}
}


void MainMenu::OnConnect( CloudEvent inEvent, int Parameter )
{
	// Check for failure
	if( Parameter != (int) ConnectResult::Success )
	{
		return OnConnectFailure( inEvent, Parameter );
	}

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
		//StartLoginProcess();
	}

}

void MainMenu::OnConnectFailure( CloudEvent inEvent, int Parameter )
{
	auto Result = ConnectResult( Parameter );

	log( "[UI DEBUG] ON CONNECT FAILURE" );
	
	if( Connecting && !_bPopupIsError && _bPopupVisible )
	{
		_bPopupIsError = true;

		auto sceneSize = Director::getInstance()->getVisibleSize();
		int HeaderFontSize = ( sceneSize.width / 1920.f ) * 86.f;
		
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

void MainMenu::OnLostConnection( CloudEvent inEvent, int Paramater )
{
	log( "[UI] Connection to RegSys appears to be lost!" );
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
	log( "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAVVVVVVVVVVVVVVV" );
	auto RegSys = RegCloud::Get();

	if( !RegSys || !RegSys->IsSecure() )
	{
		// CRITICAL ERROR
		return OnLostConnection( CloudEvent::ConnectResult, (int) ConnectResult::ConnectionError );
	}

	bool bIsAccountStored = false;
	if( bIsAccountStored )
	{

	}
	else
	{
		//log( "ASDASDASDASDASDASDASDADSASDASD" );
	
		//if( !_bLoginOpen )
		//{	
			//_bLoginOpen = true;

			// Force user login
			LoginPanel = LoginLayer::create();
			addChild( LoginPanel, 15, "LoginPanel" );
		//}
	}
}