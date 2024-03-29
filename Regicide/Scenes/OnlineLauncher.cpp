//
//    OnlineLauncher.cpp
//    Regicide Mobile
//
//    Created: 11/9/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#include "OnlineLauncher.hpp"
#include "PopTransition.hpp"
#include "AppDelegate.hpp"

USING_NS_CC;

Scene* OnlineLauncherScene::createScene()
{
    auto ret = OnlineLauncherScene::create();
    ret->setTag( REG_TAG_LAUNCHER );
    ret->setName( "OnlineLauncher" );
    return ret;
}


bool OnlineLauncherScene::init()
{
    if( !Scene::init() )
        return false;
    
    auto sceneSize = Director::getInstance()->getVisibleSize();
    auto sceneOrigin = Director::getInstance()->getVisibleOrigin();
    int FontScale = sceneSize.width / 20;
    float Padding = sceneSize.width / 50.f;
    
    Draw = DrawNode::create();
    Draw->drawSolidRect( sceneOrigin, sceneOrigin + sceneSize, Color4F( 0.15f, 0.15f, 0.15f, 1.f ) );
    this->addChild( Draw, 1 );
    
    Header = Label::createWithTTF( "Online", "fonts/arial.ttf", FontScale, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER );
    Header->setPosition( Vec2( sceneOrigin.x + sceneSize.width / 2, sceneOrigin.y + sceneSize.height * 0.85f ) );
    this->addChild( Header, 10 );
    
    BackButton = cocos2d::ui::Button::create( "backbutton_arrow_right.png", "backbutton_arrow_right.png", "backbutton_arrow_right.png" );
    BackButton->setAnchorPoint( Vec2( 1.f, 0.f ) );
    BackButton->setPosition( Vec2( sceneOrigin.x + sceneSize.width - Padding, sceneOrigin.y + Padding ) );
    BackButton->addTouchEventListener([&]( Ref* sender, ui::Widget::TouchEventType type ){
        switch (type)
        {
            case ui::Widget::TouchEventType::BEGAN:
                break;
            case ui::Widget::TouchEventType::ENDED:
                this->OnBackClicked( sender );
                break;
            default:
                break;
        }
    });
    
    this->addChild( BackButton, 20 );
    
    
    
    return true;
}

void OnlineLauncherScene::OnBackClicked( Ref* Caller )
{
    auto dir = Director::getInstance();
    dir->pushScene( pop_scene_with< cocos2d::TransitionSlideInR >::create( 0.5f ) );
}
