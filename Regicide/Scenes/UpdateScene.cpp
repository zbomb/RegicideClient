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
#include "AppDelegate.h"

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
        
        // Let application know that updates failed
        auto app = AppDelegate::GetInstance();
        app->UpdateFinished( false );
        
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
    Progress = LoadingBar::create( "loading_bar.png" );
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
            // Inform CMS so we dont get prompted for updates
            Regicide::IContentSystem::GetStorage()->SetContentCleared( false );
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
    // Inform application that the updates are complete, and full initialization can begin
    Director::getInstance()->getScheduler()->performFunctionInCocosThread( [ bSuccess ] ()
              {
                  auto app = AppDelegate::GetInstance();
                  app->UpdateFinished( bSuccess );
              } );
}
