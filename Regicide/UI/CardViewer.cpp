//
//    CardViewer.cpp
//    Regicide Mobile
//
//    Created: 11/13/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#include "CardViewer.hpp"
#include "ui/CocosGUI.h"
#include "DescriptionText.hpp"


CardViewer* CardViewer::create( Game::CardEntity* inCard, bool bAllowPlay )
{
    auto ret = new (std::nothrow) CardViewer();
    if( ret && ret->init() )
    {
        ret->autorelease();
        ret->SetTargetCard( inCard, bAllowPlay );
        return ret;
    }
    else
    {
        delete ret;
        ret = nullptr;
        return nullptr;
    }
}

bool CardViewer::init()
{
    if( !Layer::init() )
        return false;
    
    auto dir = cocos2d::Director::getInstance();
    auto origin = dir->getVisibleOrigin();
    
    Background = cocos2d::DrawNode::create();
    Background->setGlobalZOrder( 399 );
    addChild( Background, 100 );
    
    Listener = cocos2d::EventListenerTouchOneByOne::create();
    CC_ASSERT( Listener );
    
    Listener->onTouchBegan = CC_CALLBACK_2( CardViewer::onTouch, this );
    Listener->onTouchEnded = CC_CALLBACK_2( CardViewer::onTouchEnd, this );
    Listener->setSwallowTouches( true );
    
    _eventDispatcher->addEventListenerWithFixedPriority( Listener, -100 );
    
    return true;
}

bool CardViewer::onTouch( cocos2d::Touch* inTouch, cocos2d::Event* inEvent )
{
    if( !inTouch )
        return false;
    
    if( CardImage )
    {
        auto CardBounds = CardImage->getBoundingBox();
        if( CardBounds.containsPoint( inTouch->getLocation() ) )
        {
            if( ScrollPanel )
            {
                auto ScrollBounds = ScrollPanel->getBoundingBox();
                ScrollBounds.origin = ScrollBounds.origin + CardBounds.origin;
                
                if( ScrollBounds.containsPoint( inTouch->getLocation() ) )
                {
                    return false;
                }
                else
                {
                    return true;
                }
            }
        }
        
    }
    
    
    return false;
}

void CardViewer::onTouchEnd( cocos2d::Touch *inTouch, cocos2d::Event *inEvent )
{

}

CardViewer::CardViewer()
: Listener( nullptr ), CardImage( nullptr )
{
    
}

CardViewer::~CardViewer()
{
    if( Listener )
        _eventDispatcher->removeEventListener( Listener );
    
    cocos2d::log( "[DEBUG] DESTROY CARD VIEWER" );
    CardImage = nullptr;
}

void CardViewer::SetTargetCard( Game::CardEntity *inCard, bool bAllowPlay )
{
    if( CardImage )
    {
        removeChild( CardImage );
        CardImage = nullptr;
    }
    
    if( Abilities.size() > 0 )
    {
        for( auto It = Abilities.begin(); It != Abilities.end(); It++ )
        {
            if( It->second )
            {
                removeChild( It->second );
            }
        }
        
        Abilities.clear();
    }
    
    if( !inCard )
    {
        cocos2d::log( "[CardViewer] ERROR! Invalid card specified" );
        return;
    }
    
    cocos2d::Texture2D* TargetTexture = inCard->FullSizedTexture ? inCard->FullSizedTexture : inCard->FrontTexture;
    
    if( !TargetTexture )
        return;
    
    CardImage = cocos2d::Sprite::createWithTexture( TargetTexture );
    if( !CardImage )
    {
        cocos2d::log( "[CardViewer] ERROR! Invalid card texture" );
        return;
    }
    
    auto dir = cocos2d::Director::getInstance();
    auto size = dir->getVisibleSize();
    auto origin = dir->getVisibleOrigin();
    
    Background->clear();
    auto CardSize = CardImage->getContentSize();
    cocos2d::Vec2 DrawOrigin = cocos2d::Vec2( origin.x + size.width / 2.f - CardSize.width / 2.f - 10.f, origin.y + size.height / 2.f - CardSize.height / 2.f - 10.f );
    cocos2d::Vec2 DrawDest = DrawOrigin + cocos2d::Vec2( CardSize.width + 20.f + size.width / 15.f, CardSize.height + 20.f );
    Background->drawSolidRect( DrawOrigin, DrawDest, cocos2d::Color4F( 0, 0, 0, 180 ) );
    
    auto CloseIcon = cocos2d::ui::Button::create( "ExitIcon.png" );
    CloseIcon->setAnchorPoint( cocos2d::Vec2( 0.5f, 0.5f ) );
    CloseIcon->setPosition( cocos2d::Vec2( origin.x + size.width * 0.5f + CardSize.width / 2.f + 10.f + size.width / 30.f, origin.y + size.height * 0.5f + CardSize.height * 0.35f ) );
    CloseIcon->addTouchEventListener( [&] ( cocos2d::Ref* Caller, cocos2d::ui::Widget::TouchEventType Type )
                                     {
                                         if( Type == cocos2d::ui::Widget::TouchEventType::ENDED )
                                         {
                                             if( this->CloseCallback )
                                             {
                                                 cocos2d::log( "[DEBUG] CALLING CALLBACK TO CLOSE" );
                                                 this->CloseCallback();
                                             }
                                             else
                                             {
                                                 cocos2d::log( "[DEBUG] HAVE TO CLOSE SELF" );
                                                 this->removeFromParent();
                                             }
                                         }
                                     });
    CloseIcon->setGlobalZOrder( 500 );
    addChild( CloseIcon, 230 );
    
    if( bAllowPlay )
    {
        auto PlayIcon = cocos2d::ui::Button::create( "ArrowUp.png" );
        PlayIcon->setAnchorPoint( cocos2d::Vec2( 0.5f, 0.5f ) );
        PlayIcon->setPosition( cocos2d::Vec2( origin.x + size.width / 2.f + CardSize.width / 2.f + 10.f + size.width / 30.f, origin.y + size.height * 0.5f + CardSize.height * 0.175f ) );
        PlayIcon->addTouchEventListener( [&] ( cocos2d::Ref* Caller, cocos2d::ui::Widget::TouchEventType Type )
        {
            if( Type == cocos2d::ui::Widget::TouchEventType::ENDED )
            {
                if( this->PlayCallback )
                    this->PlayCallback();
                
                if( this->CloseCallback )
                    this->CloseCallback();
                else
                    this->removeFromParent();
            }
        } );
        PlayIcon->setGlobalZOrder( 500 );
        addChild( PlayIcon, 230 );
    }
    
    CardImage->setAnchorPoint( cocos2d::Vec2( 0.5f, 0.5f ) );
    CardImage->setPosition( cocos2d::Vec2( origin.x + size.width * 0.5f, origin.y + size.height * 0.5f ) );
    CardImage->setGlobalZOrder( 400 );
    addChild( CardImage, 220 );
    
    // Create Scroll Panel
    ScrollPanel = cocos2d::ui::ScrollView::create();
    ScrollPanel->setBackGroundColorType( cocos2d::ui::Layout::BackGroundColorType::SOLID );
    ScrollPanel->setBackGroundColor( cocos2d::Color3B( 20, 20, 20 ) );
    ScrollPanel->setBackGroundColorOpacity( 255 );
    ScrollPanel->setAnchorPoint( cocos2d::Vec2( 0.f, 0.f ) );
    ScrollPanel->setPosition( cocos2d::Vec2( 0.f, 32.f ) );
    ScrollPanel->setContentSize( cocos2d::Size( CardSize.width, CardSize.height * 0.4f - 32.f ) );
    ScrollPanel->setDirection( cocos2d::ui::ScrollView::Direction::VERTICAL );
    ScrollPanel->setLayoutType( cocos2d::ui::Layout::Type::VERTICAL );
    ScrollPanel->setGlobalZOrder( 405 );
    CardImage->addChild( ScrollPanel, 5 );
    
    bool bFirst = true;
    bool bActuallyFirst = true;
    float TotalHeight = 0.f;
    
    for( auto It = inCard->Abilities.begin(); It != inCard->Abilities.end(); It++ )
    {
        auto Text = AbilityText::Create( inCard, It->second, CardSize.width * 0.9f, !bFirst );
        Text->setContentSize( cocos2d::Size( CardSize.width * 0.9f, Text->GetDesiredHeight() ) );
        Text->setGlobalZOrder( 405 );
        
        // If this ability is triggerable, we dont want the next text to display a seperator
        bFirst = Text->CanTrigger();
        
        auto Layout = cocos2d::ui::LinearLayoutParameter::create();
        Layout->setGravity( cocos2d::ui::LinearLayoutParameter::LinearGravity::CENTER_HORIZONTAL );
        Layout->setMargin( cocos2d::ui::Margin( 4.f,  bActuallyFirst ? 10.f : 4.f, 4.f, 4.f ) );
        
        Text->setLayoutParameter( Layout );
        ScrollPanel->addChild( Text );
        
        TotalHeight += ( Text->getContentSize().height + ( bActuallyFirst ? 14.f : 8.f ) );
        
        Abilities[ It->first ] = Text;
        bActuallyFirst = false;
    }
    
    if( inCard->Description.size() > 0 )
    {
        auto Description = DescriptionText::Create( inCard->Description, CardSize.width * 0.9f, !bFirst );
        Description->setContentSize( cocos2d::Size( CardSize.width * 0.9f, Description->GetDesiredHeight() ) );
        Description->setGlobalZOrder( 405 );
        
        auto Layout = cocos2d::ui::LinearLayoutParameter::create();
        Layout->setGravity( cocos2d::ui::LinearLayoutParameter::LinearGravity::CENTER_HORIZONTAL );
        Layout->setMargin( cocos2d::ui::Margin( 4.f, 4.f, 4.f, 4.f ) );
        
        Description->setLayoutParameter( Layout );
        ScrollPanel->addChild( Description, 5 );
        
        TotalHeight += Description->getContentSize().height + 8.f;
    }
    
    ScrollPanel->setInnerContainerSize( cocos2d::Size( CardSize.width, TotalHeight + 10.f ) );

}
