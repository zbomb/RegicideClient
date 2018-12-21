//
//    CardSelector.cpp
//    Regicide Mobile
//
//    Created: 11/20/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#include "CardSelector.hpp"


CardSelector* CardSelector::Create( Game::CardIter Begin, Game::CardIter End )
{
    auto ret = new (std::nothrow) CardSelector();
    if( ret && ret->init() )
    {
        ret->autorelease();
        ret->LoadCards( Begin, End );
        return ret;
    }
    else
    {
        delete ret;
        ret = nullptr;
        return nullptr;
    }
}

bool CardSelector::init()
{
    if( !Layer::init() )
        return false;
    
    auto dir = cocos2d::Director::getInstance();
    auto Size = dir->getVisibleSize();
    auto Origin = dir->getVisibleOrigin();
    
    setCascadeOpacityEnabled( true );
    
    // Create Scroll Panel
    ScrollPanel = cocos2d::ui::ScrollView::create();
    ScrollPanel->setBackGroundColorType( cocos2d::ui::Layout::BackGroundColorType::SOLID );
    ScrollPanel->setBackGroundColor( cocos2d::Color3B( 100, 100, 100 ) );
    ScrollPanel->setBackGroundColorOpacity( 190 );
    ScrollPanel->setAnchorPoint( cocos2d::Vec2( 0.5f, 0.5f ) );
    ScrollPanel->setPosition( cocos2d::Vec2( Origin.x + Size.width * 0.5f, Origin.y + Size.height * 0.5f ) );
    ScrollPanel->setContentSize( cocos2d::Size( Size.width, Size.height * 0.6f + 50.f ) );
    ScrollPanel->setDirection( cocos2d::ui::ScrollView::Direction::HORIZONTAL );
    ScrollPanel->setLayoutType( cocos2d::ui::Layout::Type::HORIZONTAL );
    ScrollPanel->setCascadeOpacityEnabled( true );
    
    Confirm = cocos2d::ui::Button::create( "generic_button.png" );
    Confirm->setAnchorPoint( cocos2d::Vec2( 0.5f, 1.f ) );
    Confirm->setPosition( cocos2d::Vec2( Origin.x + Size.width * 0.5f, ScrollPanel->getPositionY() - ScrollPanel->getContentSize().height / 2.f - 30.f ) );
    Confirm->setScale( 1.3f );
    Confirm->addTouchEventListener( [=]( cocos2d::Ref* Caller, cocos2d::ui::Widget::TouchEventType Type ) -> void
    {
        if( Type == cocos2d::ui::Widget::TouchEventType::ENDED && !_bLocked && this->_fConfirm )
            this->_fConfirm();
    } );
    Confirm->setCascadeOpacityEnabled( true );
    
    ButtonLabel = cocos2d::Label::createWithTTF( "Confirm", "fonts/arial.ttf", 35 );
    ButtonLabel->setAnchorPoint( cocos2d::Vec2( 0.5f, 0.5f ) );
    ButtonLabel->setPosition( Confirm->getContentSize() / 2.f );
    
    Confirm->addChild( ButtonLabel, 1 );
    
    // Create Draw Node
    Draw = cocos2d::DrawNode::create();
    
    addChild( Confirm, 2 );
    addChild( ScrollPanel, 1 );
    ScrollPanel->addChild( Draw );
    
    return true;
}

void CardSelector::Lock()
{
    _bLocked = true;
    
    if( Confirm )
        Confirm->setEnabled( false );
    
    if( ScrollPanel )
    {
        auto Children = ScrollPanel->getChildren();
        for( auto It = Children.begin(); It != Children.end(); It++ )
        {
            auto Button = dynamic_cast< CardButton* >( *It );
            if( Button )
                Button->setEnabled( false );
        }

    }
}

void CardSelector::UnLock()
{
    _bLocked = false;
    
    if( Confirm )
        Confirm->setEnabled( true );
    
    if( ScrollPanel )
    {
        auto Children = ScrollPanel->getChildren();
        for( auto It = Children.begin(); It != Children.end(); It++ )
        {
            auto Button = dynamic_cast< CardButton* >( *It );
            if( Button )
                Button->setEnabled( true );
        }
    }
}

void CardSelector::SetConfirmLabel( const std::string& In )
{
    if( ButtonLabel )
        ButtonLabel->setString( In );
}

void CardSelector::Invalidate()
{
    std::vector< Game::CardEntity* > Output;
    if( ScrollPanel && Draw )
    {
        Draw->clear();
        
        auto Children = ScrollPanel->getChildren();
        for( auto It = Children.begin(); It != Children.end(); It++ )
        {
            // Cast to CardButton
            auto Button = dynamic_cast< CardButton* >( *It );
            if( Button && Button->bSelected )
            {
                Output.push_back( Button->LinkedCard );
                
                auto Origin = Button->getPosition();
                auto Delta = Button->getContentSize() * Button->getScale() * 0.5f;
                Draw->drawSolidRect( Origin - Delta - cocos2d::Vec2( 4.f, 4.f ), Origin + Delta + cocos2d::Vec2( 4.f, 4.f ), cocos2d::Color4F( 1.f, 0.2f, 0.2f, 1.f ) );
            }
        }
    }
    
    if( _fSelectionChanged )
    {
        _fSelectionChanged( Output );
    }
}

std::vector< Game::CardEntity* > CardSelector::GetSelection()
{
    std::vector< Game::CardEntity* > Output;
    
    if( ScrollPanel )
    {
        auto Children = ScrollPanel->getChildren();
        for( auto It = Children.begin(); It != Children.end(); It++ )
        {
            // Cast to CardButton
            auto Button = dynamic_cast< CardButton* >( *It );
            if( Button && Button->bSelected )
            {
                Output.push_back( Button->LinkedCard );
            }
        }
    }
    
    return Output;
}

void CardSelector::Deselect( Game::CardEntity *In )
{
    if( ScrollPanel )
    {
        auto Children = ScrollPanel->getChildren();
        for( auto It = Children.begin(); It != Children.end(); It++ )
        {
            auto Button = dynamic_cast< CardButton* >( *It );
            if( Button && Button->bSelected && Button->LinkedCard == In )
            {
                Button->bSelected = false;
            }
        }
        
        Invalidate();
    }
}

void CardSelector::DeselectAll()
{
    if( ScrollPanel )
    {
        auto Children = ScrollPanel->getChildren();
        for( auto It = Children.begin(); It != Children.end(); It++ )
        {
            auto Button = dynamic_cast< CardButton* >( *It );
            if( Button && Button->bSelected )
            {
                Button->bSelected = false;
            }
        }
        
        Invalidate();
    }
}

void CardSelector::LoadCards( Game::CardIter Begin, Game::CardIter End )
{
    auto dir = cocos2d::Director::getInstance();
    auto Size = dir->getVisibleSize();
    
    // Create Card Widgets, Add To Scroll Panel
    float TotalWidth = Size.width * 0.01f;
    
    for( auto It = Begin; It != End; It++ )
    {
        if( !(*It) )
        {
            cocos2d::log( "[Selector] Card was null" );
            continue;
        }
        
        auto Card = CardButton::Create( *It );
        auto defSize = Card->getContentSize();
        
        // We want to scale the card until the height is 0.25x the screen height
        Card->setScale( ( Size.height * 0.6f ) / defSize.height );
        TotalWidth += ( Card->getContentSize().width + Size.width * 0.01f );
        
        auto layoutParam = cocos2d::ui::LinearLayoutParameter::create();
        layoutParam->setGravity( cocos2d::ui::LinearLayoutParameter::LinearGravity::CENTER_VERTICAL );
        layoutParam->setMargin( cocos2d::ui::Margin( Size.width * 0.01f, 0.f, Size.width * 0.01f, 0.f ) );
        Card->setLayoutParameter( layoutParam );
        
        Card->addTouchEventListener( [=]( cocos2d::Ref* Caller, cocos2d::ui::Widget::TouchEventType Type )
        {
            if( Type == cocos2d::ui::Widget::TouchEventType::ENDED )
            {
                // If were attempting to select this card, check if were able to
                if( Card->bSelected )
                {
                    Card->bSelected = false;
                    this->Invalidate();
                }
                else if( !_fCanSelect || _fCanSelect( Card->LinkedCard ) )
                {
                    Card->bSelected = true;
                    this->Invalidate();
                }
            }
            
        } );
        
        ScrollPanel->addChild( Card );
    }
    
    ScrollPanel->setInnerContainerSize( cocos2d::Size( TotalWidth, ScrollPanel->getContentSize().height ) );
    
    // Ensure were centered
    auto ScrollSize = ScrollPanel->getContentSize();
    ScrollPanel->setInnerContainerPosition( cocos2d::Vec2( ScrollSize.width / 2.f - TotalWidth / 2.f, 0.f ) );
}
