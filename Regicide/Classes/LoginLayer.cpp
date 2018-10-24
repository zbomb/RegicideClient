
#include "LoginLayer.h"
#include "CryptoLibrary.h"
#include "RegCloud.h"
#include <vector>
#include "MainMenuScene.h"
#include "EventHub.h"
#include "utf8.h"



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
		UsernameLabel->setPosition( thisX + thisW / 2.f, thisY + thisH * 0.65f + fontSize * 0.7f + 5.f );
		addChild( UsernameLabel, 16 );
	}

	// Create Username Box
	Username = ui::EditBox::create( Size( thisW * 0.7f, fontSize * 0.7f ), "editbox_bg.png", ui::Widget::TextureResType::LOCAL );
	if( Username )
	{
		Username->setAnchorPoint( Vec2( 0.5f, 0.5f ) );
		Username->setPosition( Vec2( thisX + thisW * 0.5f, thisY + thisH * 0.65f ) );
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
		PasswordLabel->setPosition( thisX + thisW / 2.f, thisY + thisH * 0.4f + fontSize * 0.7f + 5.f );
		addChild( PasswordLabel, 16 );
	}

	// Create Password Box
	Password = ui::EditBox::create( Size( thisW * 0.7f, fontSize * 0.7f ), "editbox_bg.png", ui::Widget::TextureResType::LOCAL );
	if( Password )
	{
		Password->setAnchorPoint( Vec2( 0.5f, 0.5f ) );
		Password->setPosition( Vec2( thisX + thisW * 0.5f, thisY + thisH * 0.4f ) );
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
    else
    {
        log( "[UI ERROR] Failed to create password box!" );
    }
    
    ErrorMessage = Label::createWithTTF( "", "fonts/arial.ttf", fontSize * 0.5f, Size( thisW * 0.92f, thisH * 0.12f ), TextHAlignment::CENTER, TextVAlignment::CENTER );
    if( ErrorMessage )
    {
        ErrorMessage->setPosition( thisX + thisW / 2.f, thisY + thisH * 0.275f );
        ErrorMessage->setTextColor( Color4B( 240, 30, 30, 255 ) );
        addChild( ErrorMessage, 20 );
    }
    else
    {
        log( "[UI ERROR] Failed to create error message label!" );
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
    
	return true;
}


void LoginLayer::ShowError( std::string Message )
{
	log( "[RegSys] Login Error: %s", Message.c_str() );
    
    if( ErrorMessage )
        ErrorMessage->setString( Message );
    
    // Clear password edit box
    if( Password )
    {
        Password->setText( "" );
        Password->setEnabled( true );
    }
    
    if( Username )
        Username->setEnabled( true );
    if( CancelButton )
        CancelButton->setEnabled( true );
    if( LoginButton )
        LoginButton->setEnabled( true );
    if( RegisterButton )
        RegisterButton->setEnabled( true );
}


void LoginLayer::OnLoginClick( Ref* Caller )
{
    // Lock Menus
    if( Password )
        Password->setEnabled( false );
    if( Username )
        Username->setEnabled( false );
    if( CancelButton )
        CancelButton->setEnabled( false );
    if( LoginButton )
        LoginButton->setEnabled( false );
    if( RegisterButton )
        RegisterButton->setEnabled( false );
    
	// Get text and attempt login
	if( !Password || !Username )
	{
		ShowError( "UI Error. Please re-open login menu and try again!" );
		return;
	}
    
    // Deselect Edit Boxes
    // TODO

	std::string UserStr = Username->getText();
	std::string PassStr = Password->getText();
    
    // Lock EditBoxes
    Username->setEnabled( false );
    Password->setEnabled( false );
    
    // Validate Input Format
    if( !utf8::is_valid( UserStr.begin(), UserStr.end() ) ||
        !utf8::is_valid( PassStr.begin(), PassStr.end() ) )
    {
        ShowError( "Invalid Username/Password. Please check input and try again." );
        return;
    }
    
    auto UserLen = utf8::distance( UserStr.begin(), UserStr.end() );
    auto PassLen = utf8::distance( PassStr.begin(), PassStr.end() );
    
    // Check input lengths
    if( UserLen < Regicide::REG_USERNAME_MINLEN || UserLen > Regicide::REG_USERNAME_MAXLEN )
    {
        ShowError( "Invalid Username/Password. Please check input and try again." );
        return;
    }
    else if( PassLen < Regicide::REG_PASSWORD_MINLEN )
    {
        ShowError( "Invalid Username/Password. Please check input and try again." );
        return;
    }
    
    // Check for invalid characters
    // First, check username for invalid characters
    typedef utf8::iterator< std::string::iterator > UTFIter;
    UTFIter UserIt( UserStr.begin(), UserStr.begin(), UserStr.end() );
    UTFIter UserItEnd( UserStr.end(), UserStr.begin(), UserStr.end() );
    UTFIter PassIt( PassStr.begin(), PassStr.begin(), PassStr.end() );
    UTFIter PassItEnd( PassStr.end(), PassStr.begin(), PassStr.end() );
    
    for( auto Iter = UserIt; Iter != UserItEnd; Iter++ )
    {
        if(( *Iter < 0x0030 ) ||
           ( *Iter > 0x0039 && *Iter < 0x0041 ) ||
           ( *Iter > 0x005A && *Iter < 0x0061 ) ||
           ( *Iter > 0x007A ) )
        {
            ShowError( "Invalid Username/Password. Please check input and try again." );
            return;
        }
    }
    
    bool bUppercase = false;
    bool bLowercase = false;
    bool bSymbol = false;
    bool bNumber = false;
    
    for( auto Iter = PassIt; Iter != PassItEnd; Iter++ )
    {
        if( *Iter < 0x0030 ||
           ( *Iter > 0x007E && *Iter < 0x00A1 ) )
        {
            ShowError( "Invalid Username/Password. Please check input and try again." );
            return;
        }
        
        if( !bSymbol &&
           ( *Iter < 0x0030 ||
           ( *Iter > 0x0039 && *Iter < 0x0041 ) ||
           ( *Iter > 0x005A && *Iter < 0x0061 ) ||
           ( *Iter > 0x007A && *Iter < 0x007F ) ) )
        {
            bSymbol = true;
        }
        else if( !bNumber &&
           ( *Iter > 0x002F && *Iter < 0x003A ) )
        {
            bNumber = true;
        }
        else if( !bLowercase &&
                ( *Iter > 0x0040 && *Iter < 0x005B ) )
        {
            bLowercase = true;
        }
        else if( !bUppercase &&
                ( *Iter > 0x0060 && *Iter < 0x007B ) )
        {
            bUppercase = true;
        }
        
        if( bNumber && bLowercase && bUppercase )
            break;
    }
    
    if( !bNumber || !bLowercase || !bUppercase )
    {
        ShowError( "Invalid Username/Password. Please check input and try again." );
        return;
    }
    
	// Call login and wait for the callback event
    MainMenu* Menu = static_cast< MainMenu* >( getParent() );
    if( !Menu )
    {
        ShowError( "Critical Error! Please restart the game and retry!" );
        return;
    }
    
    if( !Menu->PerformLogin( UserStr, PassStr ) )
    {
        ShowError( "Error contacting the Regicide Network! Please retry" );
    }
}

void LoginLayer::OnRegisterClick( Ref* Caller )
{
    MainMenu* Menu = static_cast< MainMenu* >( getParent() );
    
    if( !Menu )
    {
        log( "[UI ERROR] Failed to open register menu.. Login menu parent was null.." );
        return;
    }
    
    Menu->OpenRegisterMenu();
}

void LoginLayer::OnCancelClick( Ref* Caller )
{
	// If the user doesnt want to login, then we cant let them play the game
    auto dir = Director::getInstance();
    auto menu = dir ? dir->getRunningScene() : nullptr;
    MainMenu* MenuScene = static_cast< MainMenu* >( menu );
    
    if( !MenuScene )
    {
        removeFromParentAndCleanup( true );
        return;
    }
    
    MenuScene->CancelLogin();
}

void LoginLayer::Destroy()
{
    auto FadeOut = Sequence::create( FadeOut::create( 0.5f ), RemoveSelf::create( true ), NULL );
    
    for( Node* Child : getChildren() )
    {
        if( Child )
            Child->runAction( FadeOut );
    }
    
    this->runAction( FadeOut );
}

void LoginLayer::OnLoginFailure( LoginResult ErrorCode )
{
    if( ErrorCode == LoginResult::AlreadyLoggedIn )
    {
        ShowError( "Critical Error! Please try to manually log out, and then retry login." );
    }
    else if( ErrorCode == LoginResult::ConnectionError )
    {
        ShowError( "Failed to reach the Regicide Cloud! Please retry momentarily" );
    }
    else if( ErrorCode == LoginResult::InvalidCredentials || ErrorCode == LoginResult::InvalidInput )
    {
        ShowError( "Invalid username/password! Please use the link below to reset your password!" );
    }
    else if( ErrorCode == LoginResult::Timeout )
    {
        ShowError( "The login attempt timed-out. Please retry momentarily" );
    }
}

// EventBoxDelegate Implementation
void LoginLayer::editBoxReturn( EditBox* Box )
{

}

void LoginLayer::editBoxTextChanged( EditBox *Box, const std::string &Text )
{

}

void LoginLayer::editBoxEditingDidEnd( EditBox* Box )
{
    //EditBoxDelegate::editBoxEditingDidEnd( Box );
}

void LoginLayer::editBoxEditingDidBegin( EditBox *Box )
{
    EditBoxDelegate::editBoxEditingDidBegin( Box );
}
