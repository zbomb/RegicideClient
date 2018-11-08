/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos2d-x.org
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/

#include "AppDelegate.h"
#include "Scenes/HelloWorldScene.h"
#include "Scenes/MainMenuScene.h"
#include "Scenes/IntroScene.h"
#include "CMS/IContentSystem.hpp"
#include "Scenes/UpdateScene.hpp"
#include <chrono>
#include "RegicideAPI/API.h"
#include "Utils.h"
#include "EventHub.h"
#include "LuaEngine.hpp"


using namespace Regicide;


#ifdef SDKBOX_ENABLED
#ifndef WIN32
//#include "PluginIAP/PluginIAP.h"
#endif
#endif

// #define USE_AUDIO_ENGINE 1
// #define USE_SIMPLE_AUDIO_ENGINE 1

#if USE_AUDIO_ENGINE && USE_SIMPLE_AUDIO_ENGINE
#error "Don't use AudioEngine and SimpleAudioEngine at the same time. Please just select one in your game!"
#endif

#if USE_AUDIO_ENGINE
#include "audio/include/AudioEngine.h"
using namespace cocos2d::experimental;
#elif USE_SIMPLE_AUDIO_ENGINE
#include "audio/include/SimpleAudioEngine.h"
using namespace CocosDenshion;
#endif

USING_NS_CC;

static cocos2d::Size designResolutionSize = cocos2d::Size(1920, 1080);
static cocos2d::Size smallResolutionSize = cocos2d::Size(480, 320);
static cocos2d::Size mediumResolutionSize = cocos2d::Size(1024, 768);
static cocos2d::Size largeResolutionSize = cocos2d::Size(2048, 1536);

AppDelegate* sharedDelegate = nullptr;
AppDelegate::AppDelegate()
{
    CC_ASSERT( !sharedDelegate ); // Double Init Guard
    sharedDelegate = this;
}

AppDelegate::~AppDelegate() 
{
#if USE_AUDIO_ENGINE
    AudioEngine::end();
#elif USE_SIMPLE_AUDIO_ENGINE
    SimpleAudioEngine::end();
#endif
    
    CC_ASSERT( sharedDelegate == this );
    sharedDelegate = nullptr;
}

AppDelegate* AppDelegate::GetInstance()
{
    CC_ASSERT( sharedDelegate );
    return sharedDelegate;
}

// if you want a different context, modify the value of glContextAttrs
// it will affect all platforms
void AppDelegate::initGLContextAttrs()
{
    // set OpenGL context attributes: red,green,blue,alpha,depth,stencil,multisamplesCount
    GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8, 0};

    GLView::setGLContextAttrs(glContextAttrs);
}

// if you want to use the package manager to install more packages,  
// don't modify or remove this function
static int register_all_packages()
{
    return 0; //flag for packages manager
}

bool AppDelegate::applicationDidFinishLaunching() {
#ifdef SDKBOX_ENABLED
#ifndef WIN32
    //sdkbox::IAP::init();
#endif
#endif
    // initialize director
    auto director = Director::getInstance();
    auto glview = director->getOpenGLView();
    if(!glview) {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32) || (CC_TARGET_PLATFORM == CC_PLATFORM_MAC) || (CC_TARGET_PLATFORM == CC_PLATFORM_LINUX)
        glview = GLViewImpl::createWithRect("Regicide", cocos2d::Rect(0, 0, designResolutionSize.width, designResolutionSize.height));
#else
        glview = GLViewImpl::create("Regicide");
#endif
        director->setOpenGLView(glview);
    }
    
    // Load file paths
    auto file = FileUtils::getInstance();
    auto paths = file->getSearchPaths();
    paths.push_back( "Resource" );
    paths.push_back( "LuaScripts" );
    file->setSearchPaths( paths );
    
    
    // Initialize Content System
    Regicide::IContentSystem::Init();
    
    // Check if theres an account stored locally
    auto ActManager = Regicide::IContentSystem::GetAccounts();
    
    VerifyFuture = VerifyPromise.get_future();
    if( ActManager->IsLoginStored() )
    {
        // TODO: Call API Method 'ValidateToken'
        auto api = Regicide::APIClient::GetInstance();
        if( !api->VerifyTokenAsync( [ this ]( VerifyResponse Response )
                              {
                                  if( Response.StatusCode == API_STATUS_NO_RESPONSE )
                                  {
                                      // Bad connection usually
                                      this->VerifyPromise.set_value( Verified::Offline );
                                  }
                                  else
                                  {
                                      this->VerifyPromise.set_value( Response.Result ? Verified::Success : Verified::Failed );
                                  }
                              } ) )
        {
            this->VerifyPromise.set_value( Verified::Failed );
        }
    }
    else
    {
        VerifyPromise.set_value( Verified::NoAccount );
    }
    
    // Initialize Lua Engine
    LuaEngine::GetInstance()->Init();
    
    // turn on display FPS
    director->setDisplayStats(true);

    // set FPS. the default value is 1.0/60 if you don't call this
    director->setAnimationInterval(1.0f / 60);

    // Set the design resolution
    glview->setDesignResolutionSize(designResolutionSize.width, designResolutionSize.height, ResolutionPolicy::NO_BORDER);
    auto frameSize = glview->getFrameSize();
    // if the frame's height is larger than the height of medium size.
    if (frameSize.height > mediumResolutionSize.height)
    {        
        director->setContentScaleFactor(MIN(largeResolutionSize.height/designResolutionSize.height, largeResolutionSize.width/designResolutionSize.width));
    }
    // if the frame's height is larger than the height of small size.
    else if (frameSize.height > smallResolutionSize.height)
    {        
        director->setContentScaleFactor(MIN(mediumResolutionSize.height/designResolutionSize.height, mediumResolutionSize.width/designResolutionSize.width));
    }
    // if the frame's height is smaller than the height of medium size.
    else
    {        
        director->setContentScaleFactor(MIN(smallResolutionSize.height/designResolutionSize.height, smallResolutionSize.width/designResolutionSize.width));
    }

    register_all_packages();

    if( director->getRunningScene() )
        director->replaceScene( TransitionFade::create( 2, MainMenu::createScene(), Color3B( 0, 0, 0 ) ) );
    else
        director->runWithScene( IntroScene::createScene() );
    
    // Call Init Hook
    EventHub::Execute<>( "Init" );
    
    auto start = std::chrono::steady_clock::now();
    
    // Begin Update
    auto Manager = IContentSystem::GetManager();
    Manager->ListenForUpdate( [ this, start ] ( bool bNeedUpdates, bool bError, std::string ErrMessage )
                             {
                                 // We need to ensure that the intro scene is open for a minimum peroid of time
                                 auto delta = std::chrono::duration_cast< std::chrono::milliseconds >( std::chrono::steady_clock::now() - start ).count();
                                 using namespace std::placeholders;
                                 std::function< void(float) > Callback = std::bind( &AppDelegate::FinishIntro, this, _1, bNeedUpdates, bError, ErrMessage );
                                 
                                 if( delta < 1500 )
                                 {
                                     auto sch = Director::getInstance()->getScheduler();
                                     float RemainingTime = ( 1500 - delta ) / 1000.f;
                                     sch->schedule( Callback, this, RemainingTime, 0, RemainingTime, false, "IntroEnd" );
                                 }
                                 else
                                 {
                                     Callback( 0.f );
                                 }
                             } );
    
    Manager->CheckForUpdates();
    
    return true;
}

void AppDelegate::FinishIntro( float Delay, bool bNeedUpdates, bool bErrors, std::string ErrorMessage )
{
    auto dir = Director::getInstance();
    EventHub::Execute<>( "FinishIntro" );
    
    if( bNeedUpdates )
    {
        // Open the update scene, which will process the update automatically and call back
        // once the update is complete
        
        auto upd = UpdateScene::createScene();
        dir->replaceScene( TransitionFade::create( 1.2f, upd, Color3B( 0, 0, 0 ) ) );
    }
    else
    {
        UpdateFinished( true );
    }
}

void AppDelegate::OpenMainMenu( float Delay )
{
    auto dir = Director::getInstance();
    
    // Wait for API VerifyToken to complete, timeout of 6 seconds,
    if( !bStartupComplete )
    {
        auto VerifyState = VerifyFuture.get();
        
        if( VerifyState == Verified::Failed )
        {
            // Clear locally stored account, this will cause the login menu to
            // appear once the main menu is created
            cocos2d::log( "[Regicide] You have been logged out!" );
            auto act = IContentSystem::GetAccounts();
            act->GetLocalAccount().reset();
            act->WriteAccount();
        }
        else if( VerifyState == Verified::Offline )
        {
            cocos2d::log( "[Regicide] Failed to contact Regicide Cloud to verify login, please check connection" );
        }
        else if( VerifyState == Verified::Success )
        {
            cocos2d::log( "[Regicide] Login confirmed. Welcome back!" );
            EventHub::Execute( "OnLogin", StringEventData( IContentSystem::GetAccounts()->GetLocalAccount()->Info.Username ) );
        }
    }
    
    bStartupComplete = true;
    
    if( dir->getRunningScene() )
        dir->replaceScene( TransitionFade::create( 2, MainMenu::createScene(), Color3B( 0, 0, 0 ) ) );
    else
        dir->runWithScene( MainMenu::createScene() );
    
    EventHub::Execute<>( "MainMenuOpen" );
}

// This function will be called when the app is inactive. Note, when receiving a phone call it is invoked.
void AppDelegate::applicationDidEnterBackground() {
    Director::getInstance()->stopAnimation();

#if USE_AUDIO_ENGINE
    AudioEngine::pauseAll();
#elif USE_SIMPLE_AUDIO_ENGINE
    SimpleAudioEngine::getInstance()->pauseBackgroundMusic();
    SimpleAudioEngine::getInstance()->pauseAllEffects();
#endif
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground() {
    Director::getInstance()->startAnimation();

#if USE_AUDIO_ENGINE
    AudioEngine::resumeAll();
#elif USE_SIMPLE_AUDIO_ENGINE
    SimpleAudioEngine::getInstance()->resumeBackgroundMusic();
    SimpleAudioEngine::getInstance()->resumeAllEffects();
#endif
}


void AppDelegate::UpdateFinished( bool bSuccess )
{
    // Open main menu
    OpenMainMenu( 0.f );
}
