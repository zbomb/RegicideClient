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

#ifndef  _APP_DELEGATE_H_
#define  _APP_DELEGATE_H_

#define SDKBOX_ENABLED
#define USE_AUDIO_ENGINE true

#include "Numeric.h"
#include "cocos2d.h"
#include <future>
#include "RegicideAPI/Account.h"

#define REG_TAG_LAUNCHER 1010


enum class GameState
{
    MainMenu,
    Store,
    Singleplayer,
    Multiplayer
};

enum class AIDifficulty
{
    VeryEasy,
    Easy,
    Normal,
    Hard,
    VeryHard
};

struct QuickMatchArguments
{
    std::string PlayerName;
    std::string OpponentName;
    
    Regicide::Deck PlayerDeck;
    Regicide::Deck OpponentDeck;
    
    AIDifficulty Difficulty;
    
    uint32 LevelBackground;
};

struct StoryArguments
{
    std::string LocalPlayerName;
    uint32 StoryIdentifier;
    
    Regicide::Deck CustomDeck;
};

struct PracticeArguments : public QuickMatchArguments
{
};

enum class SingleplayerType
{
    QuickMatch,
    Story,
    Practice
};


/**
@brief    The cocos2d Application.

Private inheritance here hides part of interface from Director.
*/
class  AppDelegate : private cocos2d::Application
{
public:
    AppDelegate();
    virtual ~AppDelegate();
    static AppDelegate* GetInstance();
    
    virtual void initGLContextAttrs();

    /**
    @brief    Implement Director and Scene init code here.
    @return true    Initialize success, app continue.
    @return false   Initialize failed, app terminate.
    */
    virtual bool applicationDidFinishLaunching();

    /**
    @brief  Called when the application moves to the background
    @param  the pointer of the application
    */
    virtual void applicationDidEnterBackground();

    /**
    @brief  Called when the application reenters the foreground
    @param  the pointer of the application
    */
    virtual void applicationWillEnterForeground();
    
    void UpdateFinished( bool bSuccess );
    inline GameState GetState() const { return State; }
    
    // Launching Singleplayer
    // Were going to access the Launcher through AppDelegate
    void LaunchQuickMatch( const QuickMatchArguments& Args );
    void LaunchStoryMode( const StoryArguments& Args );
    void LaunchPractice( const PracticeArguments& Args );
    
    void ExitToMenu();
    
    void update( float Delta );
    
private:
    
    void SingleplayerLaunchError( std::string ErrMessage );
    void SingleplayerLaunchSuccess();
    void LauncherProgress( float Percent );
    
private:
    
    void DebugBuildAIDeck( Regicide::Deck& outDeck );

    void FinishIntro( float Delay, bool bUpdates, bool bError, std::string Message );
    void OpenMainMenu( float Delay );
    
    bool bOpenUpdateMenu = true;
    
    // Future/Promise to wait for api account validations
    enum Verified
    {
        Offline,
        Failed,
        NoAccount,
        Success
    };
    
    std::future< Verified > VerifyFuture;
    std::promise< Verified > VerifyPromise;
    bool bStartupComplete = false;
    
    bool bUpdateComplete = false;
    bool bVerifyComplete = false;
    
    GameState State;

};

#endif // _APP_DELEGATE_H_

