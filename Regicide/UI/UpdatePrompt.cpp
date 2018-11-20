//
//    UpdatePrompt.cpp
//    Regicide Mobile
//
//    Created: 11/8/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#include "UpdatePrompt.hpp"
#include "ui/CocosGUI.h"
#include "CMS/IContentSystem.hpp"
#include "Scenes/UpdateScene.hpp"

using namespace cocos2d;
using namespace cocos2d::ui;


bool UpdatePrompt::init()
{
    // Initialize Parent
    if( !LayerColor::initWithColor( Color4B( 0, 0, 0, 180 ) ) )
    {
        return false;
    }
    
    // Calculate sizes/positions
    auto layerSize = Director::getInstance()->getVisibleSize();
    auto layerPos = Director::getInstance()->getVisibleOrigin();
    int fontSize = layerSize.width / 1920.f * 65;
    float thisX = layerPos.x + layerSize.width * 0.2f;
    float thisY = layerPos.y + layerSize.height * 0.25f;
    float thisW = layerSize.width * 0.6f;
    float thisH = layerSize.height * 0.5f;
    
    Draw = DrawNode::create();
    if( Draw )
    {
        Draw->drawSolidRect( Vec2( thisX, thisY ), Vec2( thisX + thisW, thisY + thisH ), Color4F( 0.3f, 0.3f, 0.3f, 1.f ) );
        Draw->drawSolidRect( Vec2( thisX + 4, thisY + 4 ), Vec2( thisX + thisW - 4, thisY + thisH - 4 ), Color4F( 0.15f, 0.15f, 0.15f, 1.f ) );
        this->addChild( Draw, 15, "DrawNode" );
    }
    else
    {
        log( "[UI ERROR] Failed to create draw node for login panel!" );
    }
    
    // Create Header Text
    Header = Label::createWithTTF( "Update Needed", "fonts/arial.ttf", fontSize, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::BOTTOM );
    if( Header )
    {
        Header->setPosition( thisX + thisW / 2.f, thisY + thisH - 6.f - fontSize );
        this->addChild( Header, 17, "Header" );
    }
    
    Body = Text::create( "It looks like you dont have local content needed to play, would you like to update now?", "fonts/arial.ttf", thisW /25.f );
    Body->setTextAreaSize( Size( thisW * 0.9f, thisH * 0.5f ) );
    Body->setTextHorizontalAlignment( TextHAlignment::CENTER );
    Body->setTextVerticalAlignment( TextVAlignment::TOP );
    Body->setAnchorPoint( Vec2( 0.5f, 1.f ) );
    Body->setPosition( Vec2( thisX + thisW / 2.f, thisY + thisH * 0.75f ) );
    this->addChild( Body, 20 );
    
    Accept = Button::create( "accept.png", "accept.png", "accept.png" );
    Accept->setAnchorPoint( Vec2( 0.5f, 0.f ) );
    Accept->setPosition( Vec2( thisX + thisW * 0.5f, thisY + 10.f ) );
    Accept->addTouchEventListener( [&] ( Ref* Caller, Widget::TouchEventType Type )
                                  {
                                      if( Type == Widget::TouchEventType::ENDED )
                                      {
                                          this->OnAccept( Caller );
                                      }
                                  });
    this->addChild( Accept, 15 );
    
    return true;
}


void UpdatePrompt::OnAccept( cocos2d::Ref *Caller )
{
    if( Accept )
        Accept->setEnabled( false );
    
    // Download manifest, and begin update process
    auto Manager = Regicide::IContentSystem::GetManager();
    using namespace std::placeholders;
    Manager->ListenForUpdate( std::bind( &UpdatePrompt::ListenForUpdate, this, _1, _2, _3 ) );
    Manager->ListenForProgress( nullptr );
    Manager->ListenForComplete( nullptr );
    Manager->CheckForUpdates();
    
}

void UpdatePrompt::ListenForUpdate( bool bNeeds, bool bError, std::string ErrMessage )
{
    if( bError )
    {
        if( Body )
            Body->setString( "Error while checking for updates. " + ErrMessage );
        
        cocos2d::log( "[Update Error] %s", ErrMessage.c_str() );
        if( Accept )
            Accept->setEnabled( true );
    }
    else
    {
        if( !bNeeds )
        {
            cocos2d::log( "[Update] Game is already up-to-date" );
            this->removeFromParentAndCleanup( true );
        }
        else
        {
            // We need updates!
            // We just need to goto the update scene and it will take care of the rest
            cocos2d::log( "[Update] Updates found.. starting update process..." );
            auto dir = Director::getInstance();
            
            auto manager = Regicide::IContentSystem::GetManager();
            manager->ListenForProgress( nullptr );
            manager->ListenForComplete( nullptr );
            manager->ListenForComplete( nullptr );
            
            dir->replaceScene( TransitionFade::create( 1.f, UpdateScene::createScene() ) );
        }
    }
}
