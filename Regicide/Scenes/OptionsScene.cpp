//
//    OptionsScene.cpp
//    Regicide Mobile
//
//    Created: 11/8/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#include "OptionsScene.hpp"
#include "ui/CocosGUI.h"
#include "PopTransition.hpp"
#include "CMS/IContentSystem.hpp"

using namespace cocos2d;
using namespace cocos2d::ui;

Scene* OptionsScene::createScene()
{
    return OptionsScene::create();
}


bool OptionsScene::init()
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
    
    Header = Label::createWithTTF( "Options", "fonts/arial.ttf", FontScale, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER );
    Header->setPosition( Vec2( sceneOrigin.x + sceneSize.width / 2, sceneOrigin.y + sceneSize.height * 0.85f ) );
    this->addChild( Header, 10 );
    
    BackButton = cocos2d::ui::Button::create( "backbutton_arrow_right.png", "backbutton_arrow_right.png", "backbutton_arrow_right.png" );
    BackButton->setAnchorPoint( Vec2( 1.f, 0.f ) );
    BackButton->setPosition( Vec2( sceneOrigin.x + sceneSize.width - Padding, sceneOrigin.y + Padding ) );
    BackButton->addTouchEventListener([&]( Ref* sender, Widget::TouchEventType type ){
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
    
    LogoutText = cocos2d::ui::Button::create( "logout.png", "logout.png", "logout.png" );
    LogoutText->setAnchorPoint( Vec2( 0.f, 0.5f ) );
    LogoutText->setPosition( Vec2( sceneOrigin.x + Padding, sceneOrigin.y + sceneSize.height * 0.7f ) );
    LogoutText->addTouchEventListener( [&] ( Ref* Sender, Widget::TouchEventType Type )
                                      {
                                          if( Type == Widget::TouchEventType::ENDED )
                                          {
                                              this->OnLogoutClicked( Sender );
                                          }
                                      } );
    this->addChild( LogoutText, 20 );
    
    ContentText = cocos2d::ui::Button::create( "clear_content.png", "clear_content.png", "clear_content.png" );
    ContentText->setAnchorPoint( Vec2( 0.f, 0.5f ) );
    ContentText->setPosition( Vec2( sceneOrigin.x + Padding, sceneOrigin.y + sceneSize.height * 0.55f ) );
    ContentText->addTouchEventListener( [&] ( Ref* Sender, Widget::TouchEventType Type )
                                      {
                                          if( Type == Widget::TouchEventType::ENDED )
                                          {
                                              this->OnClearContentClicked( Sender );
                                          }
                                      } );
    this->addChild( ContentText, 20 );
    
    
    return true;
}

void OptionsScene::OnBackClicked( cocos2d::Ref* Caller )
{
    auto dir = Director::getInstance();
    dir->pushScene( pop_scene_with< cocos2d::TransitionSlideInR >::create( 0.5f ) );
}

void OptionsScene::OnLogoutClicked( cocos2d::Ref *Caller )
{
    auto cs = Regicide::IContentSystem::GetAccounts();
    
    if( cs->IsLoginStored() )
    {
        cs->GetLocalAccount().reset();
        cs->WriteAccount();
        
        cocos2d::log( "[Options] Logged out local player!" );
    }
    else
    {
        cocos2d::log( "[Options] Local player isnt logged in!" );
    }
}

void OptionsScene::OnClearContentClicked( cocos2d::Ref* Caller )
{
    // Were going to delete everything (except the local account info)
    auto storage = Regicide::IContentSystem::GetStorage();
    auto deleteCount = storage->ClearLocalContent();
    
    cocos2d::log( "[Options] Local content cleared! %d files deleted", deleteCount );
}
