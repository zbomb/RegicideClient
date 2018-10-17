#include "RegisterLayer.h"
#include "MainMenuScene.h"
#include "RegCloud.h"
#include "utf8.h"


bool RegisterLayer::init()
{
    // Initialize Parent
    if( !LayerColor::initWithColor( Color4B( 0, 0, 0, 180 ) ) )
    {
        return false;
    }
    
    // Constants
    auto TotalSize = Director::getInstance()->getVisibleSize();
    float thisW = TotalSize.width * 0.6f;
    float thisH = TotalSize.height * 0.9f;
    float thisX = TotalSize.width * 0.2f;
    float thisY = TotalSize.height * 0.05f;
    float FontSize = TotalSize.width / 1920.f * 65;
    
    Draw = DrawNode::create();
    if( Draw )
    {
        Draw->drawSolidRect( Vec2( thisX, thisY ), Vec2( thisX + thisW, thisY + thisH ), Color4F( 0.3f, 0.3f, 0.3f, 1.f ) );
        Draw->drawSolidRect( Vec2( thisX + 4, thisY + 4 ), Vec2( thisX + thisW - 4, thisY + thisH - 4 ), Color4F( 0.15f, 0.15f, 0.15f, 1.f ) );
        addChild( Draw, 15 );
    }
    else
    {
        log( "[UI ERROR] Failed to create draw node for register menu" );
    }
    
    Header = Label::createWithTTF( "Register Account", "fonts/arial.ttf", FontSize, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER );
    if( Header )
    {
        Header->setAnchorPoint( Vec2( 0.5f, 1.f ) );
        Header->setPosition( thisX + thisW / 2.f, thisY + thisH - 12.f );
        addChild( Header, 17 );
    }
    else
    {
        log( "[UI ERROR] Failed to create header label for register menu" );
    }
    
    UserLabel = Label::createWithTTF( "Username", "fonts/arial.ttf", FontSize * 0.75f, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER );
    if( UserLabel )
    {
        UserLabel->setPosition( thisX + thisW / 2.f, thisY + thisH * 0.86f );
        addChild( UserLabel, 17 );
    }
    
    UserBox = ui::EditBox::create( Size( thisW * 0.75f, FontSize * 0.7f ), "editbox_bg.png" );
    if( UserBox )
    {
        UserBox->setAnchorPoint( Vec2( 0.5f, 0.5f ) );
        UserBox->setPosition( Vec2( thisX + thisW * 0.5f, thisY + thisH * 0.81f ) );
        //UserBox->setPlaceHolder( "Username" );
        UserBox->setFontColor( Color4B( 10, 10, 10, 255 ) );
        UserBox->setTextHorizontalAlignment( TextHAlignment::CENTER );
        UserBox->setInputMode( ui::EditBox::InputMode::SINGLE_LINE );
        UserBox->setReturnType( ui::EditBox::KeyboardReturnType::DONE );
        UserBox->onNextFocusedWidget = [ = ]( ui::Widget::FocusDirection dir )
        {
            if( dir == ui::Widget::FocusDirection::DOWN ||
               dir == ui::Widget::FocusDirection::RIGHT )
            {
                return (Widget*) ConfPassBox;
            }
            else
            {
                return (Widget*) DispBox;
            }
        };
        
        addChild( UserBox, 17, "UserBox" );
    }
    
    DispLabel = Label::createWithTTF( "Display Name", "fonts/arial.ttf", FontSize * 0.75f, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER );
    if( DispLabel )
    {
        DispLabel->setPosition( thisX + thisW / 2.f, thisY + thisH * 0.72f );
        addChild( DispLabel, 17 );
    }
    
    DispBox = ui::EditBox::create( Size( thisW * 0.75f, FontSize * 0.75f ), "editbox_bg.png" );
    if( DispBox )
    {
        DispBox->setAnchorPoint( Vec2( 0.5f, 0.7f ) );
        DispBox->setPosition( Vec2( thisX + thisW * 0.5f, thisY + thisH * 0.67f ) );
        //DispBox->setPlaceHolder( "Username" );
        DispBox->setFontColor( Color4B( 10, 10, 10, 255 ) );
        DispBox->setTextHorizontalAlignment( TextHAlignment::CENTER );
        DispBox->setInputMode( ui::EditBox::InputMode::SINGLE_LINE );
        DispBox->setReturnType( ui::EditBox::KeyboardReturnType::DONE );
        DispBox->onNextFocusedWidget = [ = ]( ui::Widget::FocusDirection dir )
        {
            if( dir == ui::Widget::FocusDirection::DOWN ||
               dir == ui::Widget::FocusDirection::RIGHT )
            {
                return (Widget*) UserBox;
            }
            else
            {
                return (Widget*) EmailBox;
            }
        };
        
        addChild( DispBox, 17, "DispBox" );
    }
    
    EmailLabel = Label::createWithTTF( "Email Address", "fonts/arial.ttf", FontSize * 0.75f, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER );
    if( EmailLabel )
    {
        EmailLabel->setPosition( thisX + thisW / 2.f, thisY + thisH * 0.58 );
        addChild( EmailLabel, 17 );
    }
    
    EmailBox = ui::EditBox::create( Size( thisW * 0.75f, FontSize * 0.75f ), "editbox_bg.png" );
    if( EmailBox )
    {
        EmailBox->setAnchorPoint( Vec2( 0.5f, 0.7f ) );
        EmailBox->setPosition( Vec2( thisX + thisW * 0.5f, thisY + thisH * 0.53f ) );
        //EmailBox->setPlaceHolder( "Username" );
        EmailBox->setFontColor( Color4B( 10, 10, 10, 255 ) );
        EmailBox->setTextHorizontalAlignment( TextHAlignment::CENTER );
        EmailBox->setInputMode( ui::EditBox::InputMode::SINGLE_LINE );
        EmailBox->setReturnType( ui::EditBox::KeyboardReturnType::DONE );
        EmailBox->onNextFocusedWidget = [ = ]( ui::Widget::FocusDirection dir )
        {
            if( dir == ui::Widget::FocusDirection::DOWN ||
               dir == ui::Widget::FocusDirection::RIGHT )
            {
                return (Widget*) DispBox;
            }
            else
            {
                return (Widget*) PassBox;
            }
        };
        
        addChild( EmailBox, 17, "EmailBox" );
    }
    
    PassLabel = Label::createWithTTF( "Password", "fonts/arial.ttf", FontSize * 0.75f, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER );
    if( PassLabel )
    {
        PassLabel->setPosition( thisX + thisW / 2.f, thisY + thisH * 0.44f );
        addChild( PassLabel, 17, "PassLabel" );
    }
    
    PassBox = ui::EditBox::create( Size( thisW * 0.75f, FontSize * 0.75f ), "editbox_bg.png" );
    if( PassBox )
    {
        PassBox->setAnchorPoint( Vec2( 0.5f, 0.7f ) );
        PassBox->setPosition( Vec2( thisX + thisW * 0.5f, thisY + thisH * 0.39f ) );
        //PassBox->setPlaceHolder( "Username" );
        PassBox->setFontColor( Color4B( 10, 10, 10, 255 ) );
        PassBox->setTextHorizontalAlignment( TextHAlignment::CENTER );
        PassBox->setInputMode( ui::EditBox::InputMode::SINGLE_LINE );
        PassBox->setInputFlag( ui::EditBox::InputFlag::PASSWORD );
        PassBox->setReturnType( ui::EditBox::KeyboardReturnType::DONE );
        PassBox->onNextFocusedWidget = [ = ]( ui::Widget::FocusDirection dir )
        {
            if( dir == ui::Widget::FocusDirection::DOWN ||
               dir == ui::Widget::FocusDirection::RIGHT )
            {
                return (Widget*) EmailBox;
            }
            else
            {
                return (Widget*) ConfPassBox;
            }
        };
        
        addChild( PassBox, 17, "PassBox" );
    }
    
    ConfPassLabel = Label::createWithTTF( "Password", "fonts/arial.ttf", FontSize * 0.75f, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER );
    if( ConfPassLabel )
    {
        ConfPassLabel->setPosition( thisX + thisW / 2.f, thisY + thisH * 0.3f );
        addChild( ConfPassLabel, 17, "ConfPassLabel" );
    }
    
    ConfPassBox = ui::EditBox::create( Size( thisW * 0.75f, FontSize * 0.75f ), "editbox_bg.png" );
    if( ConfPassBox )
    {
        ConfPassBox->setAnchorPoint( Vec2( 0.5f, 0.7f ) );
        ConfPassBox->setPosition( Vec2( thisX + thisW * 0.5f, thisY + thisH * 0.25f ) );
        //ConfPassBox->setPlaceHolder( "Username" );
        ConfPassBox->setFontColor( Color4B( 10, 10, 10, 255 ) );
        ConfPassBox->setTextHorizontalAlignment( TextHAlignment::CENTER );
        ConfPassBox->setInputMode( ui::EditBox::InputMode::SINGLE_LINE );
        ConfPassBox->setInputFlag( ui::EditBox::InputFlag::PASSWORD );
        ConfPassBox->setReturnType( ui::EditBox::KeyboardReturnType::DONE );
        ConfPassBox->onNextFocusedWidget = [ = ]( ui::Widget::FocusDirection dir )
        {
            if( dir == ui::Widget::FocusDirection::DOWN ||
               dir == ui::Widget::FocusDirection::RIGHT )
            {
                return (Widget*) PassBox;
            }
            else
            {
                return (Widget*) ConfPassBox;
            }
        };
        
        addChild( ConfPassBox, 17, "ConfPassLabel" );
    }
    
    ErrorMessage = Label::createWithTTF( "", "fonts/arial.ttf", FontSize * 0.5f, Size( thisW * 0.92f, thisH * 0.1f ), TextHAlignment::CENTER, TextVAlignment::CENTER );
    if( ErrorMessage )
    {
        ErrorMessage->setAnchorPoint( Vec2( 0.5f, 0.5f ) );
        ErrorMessage->setPosition( thisX + thisW / 2.f, thisY + thisH * 0.16f );
        ErrorMessage->setTextColor( Color4B( 240, 30, 30, 255 ) );
        addChild( ErrorMessage, 17, "ErrorMessage" );
        
    }
    
    cocos2d::Vector< MenuItem* > MenuNodes;
    
    CancelButton = MenuItemImage::create( "dev_button_cancel.png", "dev_button_cancel.png", CC_CALLBACK_1( RegisterLayer::OnCancel, this ) );
    if( CancelButton )
    {
        auto defaultSize = CancelButton->getContentSize();
        float buttonScale = 160.f / defaultSize.width;
        
        CancelButton->setScale( buttonScale );
        CancelButton->setAnchorPoint( Vec2( 0.f, 0.f ) );
        CancelButton->setPosition( Vec2( thisX + 20.f, thisY + 20.f ) );
        MenuNodes.pushBack( CancelButton );
    }
    
    CreateButton = MenuItemImage::create( "dev_button_create_account.png", "dev_button_create_account.png", CC_CALLBACK_1( RegisterLayer::DoRegister, this ) );
    if( CreateButton )
    {
        auto defaultSize = CreateButton->getContentSize();
        float buttonScale = 275.f / defaultSize.width;
        
        CreateButton->setScale( buttonScale );
        CreateButton->setAnchorPoint( Vec2( 1.f, 0.f ) );
        CreateButton->setPosition( Vec2( thisX + thisW - 20.f, thisY + 20.f ) );
        MenuNodes.pushBack( CreateButton );
    }
    
    LoginButton = MenuItemImage::create( "dev_button_login.png", "dev_button_login.png", CC_CALLBACK_1( RegisterLayer::OpenLogin, this ) );
    if( LoginButton )
    {
        auto defaultSize = LoginButton->getContentSize();
        float buttonScale = 160.f / defaultSize.width;
        
        LoginButton->setScale( buttonScale );
        LoginButton->setAnchorPoint( Vec2( 0.5f, 0.f ) );
        LoginButton->setPosition( Vec2( thisX + thisW / 2.f, thisY + 20.f ) );
        MenuNodes.pushBack( LoginButton );
    }
    
    Buttons = Menu::createWithArray( MenuNodes );
    if( Buttons )
    {
        Buttons->setPosition( 0.f, 0.f );
        addChild( Buttons, 17, "Buttons" );
    }
    
    return true;
}

void RegisterLayer::Destroy()
{
    auto FadeOut = Sequence::create( FadeOut::create( 0.5f ), RemoveSelf::create( true ), NULL );
    
    for( Node* Child : getChildren() )
    {
        if( Child )
            Child->runAction( FadeOut );
    }
    
    this->runAction( FadeOut );
}

void RegisterLayer::OnCancel( Ref* inRef )
{
    Destroy();
}

void RegisterLayer::OpenLogin( Ref* inRef )
{
    MainMenu* Menu = static_cast< MainMenu* >( getParent() );
    if( !Menu )
    {
        log( "[UI ERROR] Failed to open login menu from register menu.. invalid MainMenu parent.." );
        return;
    }
    
    Menu->OpenLoginMenu();
}

void RegisterLayer::DoRegister( Ref* inRef)
{
    if( !UserBox || !PassBox || !DispBox || !ConfPassBox || !EmailBox )
    {
        ShowError( "Strange error.. wtf did you do? Re-open the menu and try again." );
        return;
    }
    
    // Deselect All Boxes
    // TODO
    
    std::string Username = UserBox->getText();
    std::string Password = PassBox->getText();
    std::string ConfirmPassword = ConfPassBox->getText();
    std::string DispName = DispBox->getText();
    std::string EmailAddr = EmailBox->getText();
    
    log( "%s", DispName.c_str() );
    // Check if passwords match
    if( Password != ConfirmPassword )
    {
        ShowError( "Passwords do not match! Re-enter your passwords and try again." );
        ConfPassBox->setText( "" );
        return;
    }
    
    // Clear Password Fields
    PassBox->setText( "" );
    ConfPassBox->setText( "" );
    
    // Validate Input
    if( !utf8::is_valid( Username.begin(), Username.end() ) ||
       !utf8::is_valid( Password.begin(), Password.end() ) ||
       !utf8::is_valid( DispName.begin(), DispName.end() ) ||
       !utf8::is_valid( EmailAddr.begin(), EmailAddr.end() ) )
    {
        ShowError( "Invalid input provided! Ensure all fields are filled out with valid text." );
        return;
    }

    auto UserLen = utf8::distance( Username.begin(), Username.end() );
    auto PassLen = utf8::distance( Password.begin(), Password.end() );
    auto DispLen = utf8::distance( DispName.begin(), DispName.end() );
    auto EmailLen = utf8::distance( EmailAddr.begin(), EmailAddr.end() );
    
    if( UserLen < Regicide::REG_USERNAME_MINLEN || UserLen > Regicide::REG_USERNAME_MAXLEN )
    {
        ShowError( "Invalid Username. Must be between " + std::to_string( Regicide::REG_USERNAME_MINLEN ) +
                  " and " + std::to_string( Regicide::REG_USERNAME_MAXLEN ) + " characters." );
        return;
    }
    else if( PassLen < Regicide::REG_PASSWORD_MINLEN )
    {
        ShowError( "Invlaid Password. Must be at least " + std::to_string( Regicide::REG_PASSWORD_MINLEN ) + " characters." );
        return;
    }
    else if( DispLen < Regicide::REG_DISPNAME_MINLEN || DispLen > Regicide::REG_DISPNAME_MAXLEN )
    {
        ShowError( "Invalid Display Name. Must be between " + std::to_string( Regicide::REG_DISPNAME_MINLEN ) +
          " and " + std::to_string( Regicide::REG_DISPNAME_MAXLEN ) + " characters." );
        return;
    }
    else if( EmailLen < Regicide::REG_EMAIL_MINLEN || EmailLen > Regicide::REG_EMAIL_MAXLEN )
    {
        ShowError( "Invalid Email. Ensure a valid email address was specified!" );
        return;
    }
    
    typedef utf8::iterator< std::string::iterator > UTFIter;
    UTFIter UserBegin( Username.begin(), Username.begin(), Username.end() );
    UTFIter UserEnd( Username.end(), Username.begin(), Username.end() );
    UTFIter PassBegin( Password.begin(), Password.begin(), Password.end() );
    UTFIter PassEnd( Password.end(), Password.begin(), Password.end() );
    UTFIter DispBegin( DispName.begin(), DispName.begin(), DispName.end() );
    UTFIter DispEnd( DispName.end(), DispName.begin(), DispName.end() );
    UTFIter EmailBegin( EmailAddr.begin(), EmailAddr.begin(), EmailAddr.end() );
    UTFIter EmailEnd( EmailAddr.end(), EmailAddr.begin(), EmailAddr.end() );
    
    // Now we need to check the contents of the strings
    for( auto Iter = UserBegin; Iter != UserEnd; Iter++ )
    {
        if(( *Iter < 0x0030 ) ||
           ( *Iter > 0x0039 && *Iter < 0x0041 ) ||
           ( *Iter > 0x005A && *Iter < 0x0061 ) ||
           ( *Iter > 0x007A ) )
        {
            ShowError( "Invalid character(s) found in Username! Must only contain letters or numbers!" );
            return;
        }
    }
    
    bool bUppercase = false;
    bool bLowercase = false;
    bool bSymbol = false;
    bool bNumber = false;
    
    for( auto Iter = PassBegin; Iter != PassEnd; Iter++ )
    {
        if( *Iter < 0x0030 ||
           ( *Iter > 0x007E && *Iter < 0x00A1 ) )
        {
            ShowError( "Invalid character(s) found in Password! Can not contain control characters!" );
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
        
        if( bSymbol && bNumber && bLowercase && bUppercase )
            break;
    }
    
    if( !bNumber || !bLowercase || !bUppercase )
    {
        ShowError( "Password must contain at lesat one uppercase, one lowercase and one number, while being at least " +
                  std::to_string( Regicide::REG_PASSWORD_MINLEN ) + " characters." );
        return;
    }
    
    // Email Address
    // Just checking for an @
    bool bAtSymbol = false;
    for( auto Iter = EmailBegin; Iter != EmailEnd; Iter++ )
    {
        if( *Iter == 0x0040 )
        {
            bAtSymbol = true;
            break;
        }
    }
    
    if( !bAtSymbol )
    {
        ShowError( "Invalid Email. Ensure a valid email address was specified and try again!" );
        return;
    }
    
    // Display Name
    // Checking for invalid characters, much more flexible than username
    for( auto Iter = DispBegin; Iter != DispEnd; Iter++ )
    {
        if( *Iter < 0x0020 ||
           ( *Iter > 0x007E && *Iter < 0x00A1 ) )
        {
            ShowError( "Display Name contains invalid characters! Check input and try again." );
            return;
        }
        
        // TODO, Check for other invalid characters
    }
    
    MainMenu* Menu = static_cast< MainMenu* >( getParent() );
    if( !Menu )
    {
        ShowError( "Critical Error. Main Menu is invalid? Restart game and retry." );
        return;
    }
    
    log( "2 %s", DispName.c_str() );
    
    if( !Menu->PerformRegister( Username, Password, DispName, EmailAddr ) )
    {
        ShowError( "Critical Error. Invalid network connection. Restart game and retry." );
    }
}

void RegisterLayer::ShowError( std::string inError )
{
    if( ErrorMessage )
        ErrorMessage->setString( inError );
    
    log( "[RegLogin] Register Error: %s", inError.c_str() );
}

void RegisterLayer::OnRegisterFailure( int Result )
{
    if( Result == (int) ERegisterResult::ConnectionError )
    {
        ShowError( "Please check your internet connection and try again." );
    }
    else if( Result == (int) ERegisterResult::EmailTaken )
    {
        ShowError( "This email address is already in use. Please return to login screen to login or request a new password." );
    }
    else if( Result == (int) ERegisterResult::IllegalUsername )
    {
        ShowError( "The provided username contains illegal characters. Please choose a valid username and try again." );
    }
    else if( Result == (int) ERegisterResult::InvalidEmail )
    {
        ShowError( "The provided email address is not valid. Please choose a valid email address and try again." );
    }
    else if( Result == (int) ERegisterResult::ParamLengths )
    {
        ShowError( "Some of the provided information is incorrect. Check input and try again." ); // TODO: Make this more descriptive
    }
    else
    {
        ShowError( "An unknown error has occured. Please try again momentarily." );
    }
}
