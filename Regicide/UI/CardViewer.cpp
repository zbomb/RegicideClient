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
    Listener->setSwallowTouches( true );
    
    _eventDispatcher->addEventListenerWithFixedPriority( Listener, -100 );
    
    return true;
}

bool CardViewer::onTouch( cocos2d::Touch* inTouch, cocos2d::Event* inEvent )
{
    if( CardImage )
    {
        if( CardImage->getBoundingBox().containsPoint( inTouch->getLocation() ) )
        {
            return true;
        }
        
    }
    
    return false;
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

}
