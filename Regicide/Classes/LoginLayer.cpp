
#include "LoginLayer.h"
#include "CryptoLibrary.h"
#include "RegCloud.h"
#include <vector>


namespace Regicide
{
constexpr auto REG_USERNAME_MINLEN =	5;
constexpr auto REG_USERNAME_MAXLEN =	32;
constexpr auto REG_PASSWORD_MINLEN =	5;
constexpr auto REG_EMAIL_MINLEN	= 		5;
constexpr auto REG_EMAIL_MAXLEN	= 		255;
constexpr auto REG_DISPNAME_MINLEN =	5;
constexpr auto REG_DISPNAME_MAXLEN =	48;
};


bool LoginLayer::init()
{
	// Initialize Parent
	if( !LayerColor::initWithColor( Color4B( 0, 0, 0, 180 ) ) )
	{
		return false;
	}

	// Calculate sizes/positions
	auto layerSize = Director::getInstance()->getVisibleSize();
	int fontSize = layerSize.width / 1920.f * 65;
	float thisX = layerSize.width * 0.2f;
	float thisY = layerSize.height * 0.25f;
	float thisW = layerSize.width * 0.6f;
	float thisH = layerSize.height * 0.5f;

	// Draw Box
	
	Draw = DrawNode::create();
	if( Draw )
	{
		Draw->drawSolidRect( Vec2( thisX, thisY ), Vec2( thisX + thisW, thisY + thisH ), Color4F( 0.3f, 0.3f, 0.3f, 1.f ) );
		Draw->drawSolidRect( Vec2( thisX + 4, thisY + 4 ), Vec2( thisX + thisW - 4, thisY + thisH - 4 ), Color4F( 0.15f, 0.15f, 0.15f, 1.f ) );
		addChild( Draw, 15, "DrawNode" );
	}
	else
	{
		log( "[UI ERROR] Failed to create draw node for login panel!" );
	}
	
	

	// Create Header Text
	Header = Label::createWithTTF( "Account Login", "fonts/arial.ttf", fontSize, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::BOTTOM );
	if( Header )
	{
		Header->setPosition( thisX + thisW / 2.f, thisY + thisH - 6.f - fontSize );
		addChild( Header, 17, "Header" );
	}

	UsernameLabel = Label::createWithTTF( "Username", "fonts/arial.ttf", fontSize * 0.6f, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::BOTTOM );
	if( UsernameLabel )
	{
		UsernameLabel->setPosition( thisX + thisW / 2.f, thisY + thisH * 0.6f + fontSize * 0.7f + 5.f );
		addChild( UsernameLabel, 16 );
	}

	// Create Username Box
	Username = ui::EditBox::create( Size( thisW * 0.7f, fontSize * 0.7f ), "editbox_bg.png", ui::Widget::TextureResType::LOCAL );
	if( Username )
	{
		Username->setAnchorPoint( Vec2( 0.5f, 0.5f ) );
		Username->setPosition( Vec2( thisX + thisW * 0.5f, thisY + thisH * 0.6f ) );
		//Username->setPlaceHolder( "Username" );
		Username->setFontColor( Color4B( 10, 10, 10, 255 ) );
		Username->setTextHorizontalAlignment( TextHAlignment::CENTER );
		Username->setInputMode( ui::EditBox::InputMode::SINGLE_LINE );
		Username->setReturnType( ui::EditBox::KeyboardReturnType::DONE );
		Username->onNextFocusedWidget = [ = ]( ui::Widget::FocusDirection dir )
		{
			if( dir == ui::Widget::FocusDirection::DOWN ||
				dir == ui::Widget::FocusDirection::RIGHT )
			{
				return (Widget*) Password;
			}
			else
			{
				return (Widget*) Username;
			}
		};

		addChild( Username, 17, "Username" );
	}

	PasswordLabel = Label::createWithTTF( "Password", "fonts/arial.ttf", fontSize * 0.6f, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::BOTTOM );
	if( PasswordLabel )
	{
		PasswordLabel->setPosition( thisX + thisW / 2.f, thisY + thisH * 0.35f + fontSize * 0.7f + 5.f );
		addChild( PasswordLabel, 16 );
	}

	// Create Password Box
	Password = ui::EditBox::create( Size( thisW * 0.7f, fontSize * 0.7f ), "editbox_bg.png", ui::Widget::TextureResType::LOCAL );
	if( Password )
	{
		Password->setAnchorPoint( Vec2( 0.5f, 0.5f ) );
		Password->setPosition( Vec2( thisX + thisW * 0.5f, thisY + thisH * 0.35f ) );
		//Password->setPlaceHolder( "Password" );
		Password->setFontColor( Color4B( 10, 10, 10, 255 ) );
		Password->setTextHorizontalAlignment( TextHAlignment::CENTER );
		Password->setInputFlag( ui::EditBox::InputFlag::PASSWORD );
		Password->setInputMode( ui::EditBox::InputMode::SINGLE_LINE );
		Password->setReturnType( ui::EditBox::KeyboardReturnType::DONE );
		Password->onNextFocusedWidget = [ = ]( ui::Widget::FocusDirection dir )
		{
			if( dir == ui::Widget::FocusDirection::UP ||
				dir == ui::Widget::FocusDirection::LEFT )
			{
				return (Widget*) Username;
			}
			else
			{
				return (Widget*) LoginButton;
			}
		};

		addChild( Password, 17, "Password" );
	}

	cocos2d::Vector< Node* > MenuItems;

	// Create Login Button
	LoginButton = MenuItemImage::create( "dev_button_login.png", "dev_button_login.png", "", CC_CALLBACK_1( LoginLayer::OnLoginClick, this ) );
	if( LoginButton )
	{
		auto defaultSize = LoginButton->getContentSize();
		float buttonScale = 140.f / defaultSize.width;

		LoginButton->setScale( buttonScale );
		LoginButton->setAnchorPoint( Vec2( 1.f, 0.f ) );
		LoginButton->setPosition( Vec2( thisX + thisW - 20.f, thisY + 20.f ) );
		MenuItems.pushBack( LoginButton );
	}
	else
	{
		log( "[UI ERROR] Failed to create login button in login menu");
	}

	CancelButton = MenuItemImage::create( "dev_button_cancel.png", "dev_button_cancel.png", "", CC_CALLBACK_1( LoginLayer::OnCancelClick, this ) );
	if( CancelButton )
	{
		auto defaultSize = CancelButton->getContentSize();
		float buttonScale = 140.f / defaultSize.width;
		
		CancelButton->setScale( buttonScale );
		CancelButton->setAnchorPoint( Vec2( 0.f, 0.f ) );
		CancelButton->setPosition( Vec2( thisX + 20.f, thisY + 20.f ) );
		MenuItems.pushBack( CancelButton );
	}
	else
	{
		log( "[UI ERROR] Failed to create cancel button in login menu" );
	}

	RegisterButton = MenuItemImage::create( "dev_button_create_account.png", "dev_button_create_account.png", "", CC_CALLBACK_1( LoginLayer::OnRegisterClick, this ) );
	if( RegisterButton )
	{
		auto defaultSize = RegisterButton->getContentSize();
		float buttonScale = 275.f / defaultSize.width;

		RegisterButton->setScale( buttonScale );
		RegisterButton->setAnchorPoint( Vec2( 0.5f, 0.f ) );
		RegisterButton->setPosition( Vec2( thisX + thisW / 2.f, thisY + 20.f ) );
		MenuItems.pushBack( RegisterButton );
	}
	else
	{
		log( "[UI ERROR] Failed to create 'create account' button in login menu" );
	}

	ButtonBank = Menu::create();
	if( ButtonBank )
	{
		ButtonBank->setPosition( 0.f, 0.f );
		for( auto Btn : MenuItems )
		{
			ButtonBank->addChild( Btn, 17 );
		}

		addChild( ButtonBank, 17, "ButtonBank" );
	}

	// Bind events


	return true;
}


void LoginLayer::ShowError( std::string ErrorMessage )
{
	log( "[RegSys] Login Error: %s", ErrorMessage );
}


void LoginLayer::OnLoginClick( Ref* Caller )
{
	// Get text and attempt login
	if( !Password || !Username )
	{
		ShowError( "UI Error. Please re-open login menu and try again!" );
		return;
	}

	// DEBUG
	log( "[RegSys] LOGGING IN!" );

	std::string UserStr = Username->getText();
	std::string PassStr = Password->getText();

	// We can do some simple checks to ensure the data is proper before sending it off to the server
	auto UserLen = UserStr.length();
	auto PassLen = PassStr.length();

	// TODO: Handle UTF characters better! (at all really)
	if( UserLen < Regicide::REG_USERNAME_MINLEN || UserLen > 128 )
	{
		ShowError( "Username must be between " + std::to_string( Regicide::REG_USERNAME_MINLEN ) + " and " +
					std::to_string( Regicide::REG_USERNAME_MAXLEN ) + " characters!" );
		return;
	}
	else if( PassLen < Regicide::REG_PASSWORD_MINLEN )
	{
		ShowError( "Password must be at least " + std::to_string( Regicide::REG_PASSWORD_MINLEN ) + " characters!" );
		return;
	}

	// Call login and wait for the callback event
	RegCloud* Cloud = RegCloud::Get();

	if( !Cloud )
	{
		ShowError( "Login error! Please restart the game and try again.." );
		return;
	}

	Cloud->Login( UserStr, PassStr );

}

void LoginLayer::OnRegisterClick( Ref* Caller )
{
	// Open register menu

}

void LoginLayer::OnCancelClick( Ref* Caller )
{
	// If the user doesnt want to login, then we cant let them play the game

}