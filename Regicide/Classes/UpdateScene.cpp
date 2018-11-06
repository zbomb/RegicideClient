//
//  UpdateScene.cpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/5/18.
//

#include "UpdateScene.hpp"
#include "IContentSystem.hpp"
#include "MainMenuScene.h"
#include "Utils.h"

using namespace cocos2d;

Scene* UpdateScene::createScene()
{
    auto ret = UpdateScene::create();
    if( ret ) { ret->setName( "UpdateScene" ); }
    return ret;
}

UpdateScene::~UpdateScene()
{
    
}

bool UpdateScene::init()
{
    if( !Scene::init() )
    {
        cocos2d::log( "[UpdateScene] Parent scene failed to init!" );
        // Were going to transition to main menu, and inform it of an update failure
        auto Menu = MainMenu::createScene();
        // TODO: Pass mainmenu a message
        Director::getInstance()->replaceScene( TransitionFade::create( 1.5f, Menu, Color3B( 0, 0, 0 ) ) );
        return false;
    }
    
    auto SceneSize = Director::getInstance()->getVisibleSize();
    auto SceneOrigin = Director::getInstance()->getVisibleOrigin();
    
    int HeaderFontSize = ( SceneSize.width / 1920.f ) * 86.f;
    
    ////////////// Header //////////////
    Header = Label::createWithTTF( "Updating..", "fonts/Ringbearer Medium.ttf", HeaderFontSize );
    Header->setAlignment( TextHAlignment::CENTER );
    Header->setAnchorPoint( Vec2( 0.5f, 0.5f ) );
    Header->setPosition( Vec2( SceneOrigin.x + SceneSize.width / 2.f, SceneOrigin.y + SceneSize.height * 0.8f ) );
    addChild( Header, 10 );
    
    ////////////// Progress Bar //////////////////
    Progress = LoadingBar::create();
    Progress->setAnchorPoint( Vec2( 0.5f, 0.5f ) );
    Progress->setPosition( Vec2( SceneOrigin.x + SceneSize.width / 2.f, SceneOrigin.y + SceneSize.height / 2.f ) );
    Progress->setDirection( ui::LoadingBar::Direction::RIGHT );
    Progress->setPercent( 0.f );
    addChild( Progress, 14 );
    
    ////////////// CurrentFile Label /////////////////
    CurrentFile = Label::createWithTTF( "Starting Update...", "fonts/arial.ttf", HeaderFontSize / 2.f, Size( SceneSize.width, HeaderFontSize * 0.6f ), TextHAlignment::CENTER, TextVAlignment::CENTER );
    CurrentFile->setAnchorPoint( Vec2( 0.5f, 0.5f ) );
    CurrentFile->setPosition( Vec2( SceneOrigin.x + SceneSize.width / 2.f, SceneOrigin.y + SceneSize.height * 0.35 ) );
    addChild( CurrentFile, 12 );
    
    // Begin updates when the transition completes
    auto cms = Regicide::IContentSystem::GetManager();
    
    using namespace std::placeholders;
    cms->ListenForComplete( std::bind( &UpdateScene::UpdateComplete, this, _1, _2 ) );
    cms->ListenForProgress( std::bind( &UpdateScene::UpdateProgress, this, _1, _2, _3 ) );
    
    this->setonEnterTransitionDidFinishCallback( std::bind( &Regicide::IContentManager::ProcessUpdates, cms ) );
    
    return true;
}

void UpdateScene::UpdateComplete( bool bSuccess, uint64 Bytes )
{
    // Wait one second, so we can show the complete message before transitioning to main menu
    auto dir = Director::getInstance();
    auto sch = dir->getScheduler();
    
    if( Progress )
        Progress->setPercent( 100.f );
    if( CurrentFile )
    {
        if( bSuccess )
        {
            CurrentFile->setString( Regicide::Utils::FormatString( "Update complete! %s", Regicide::Utils::ByteString( Bytes ).c_str() ) );
        }
        else
        {
            CurrentFile->setString( "An error occured while updating!" );
        }
    }
    
    using namespace std::placeholders;
    std::function< void(float) > ExitFunc = std::bind( &UpdateScene::Internal_Exit, this, _1, bSuccess, Bytes );
    sch->schedule( ExitFunc, this, 1.f, 0, 1.f, false, "ExitUpdateCallback" );
}

void UpdateScene::UpdateProgress( uint64 Downloaded, uint64 Total, std::string BlockName )
{
    if( Progress )
    {
        Progress->setPercent( Regicide::Utils::Clamp( ( (float) Downloaded / (float) Total ) * 100.f, 0.f, 100.f ) );
    }
    if( CurrentFile )
    {
        CurrentFile->setString( BlockName );
    }
}

void UpdateScene::Internal_Exit( float Delay, bool bSuccess, uint64 Bytes )
{
    auto dir = Director::getInstance();
    auto menu = MainMenu::createScene();
    
    // TODO: Inform menu of errors?
    
    dir->replaceScene( TransitionFade::create( 1.5f, menu, Color3B( 0, 0, 0 ) ) );
}
