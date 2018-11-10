//
//  SingleplayerLauncher.cpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/10/18.
//

#include "SingleplayerLauncher.hpp"
#include "AppDelegate.h"
#include "World.hpp"
#include "cocos2d.h"
#include "SingleplayerGameMode.hpp"

using namespace Game;



SingleplayerLauncher& SingleplayerLauncher::GetInstance()
{
    static SingleplayerLauncher Launcher;
    return Launcher;
}


bool SingleplayerLauncher::LaunchStory( const StoryArguments& Args )
{
    if( AppDelegate::GetInstance()->GetState() != GameState::MainMenu )
    {
        cocos2d::log( "[Launcher] Attempt to launch story-mode outside of the main menu!" );
        if( OnError )
            OnError( "Attempt to launch story-mode outside of main menu!" );
        
        return false;
    }
    
    
    return false;
}

bool SingleplayerLauncher::LaunchPractice( const PracticeArguments& Args )
{
    if( AppDelegate::GetInstance()->GetState() != GameState::MainMenu )
    {
        cocos2d::log( "[Launcher] Attempt to launch practice-mode outside of the main menu!" );
        if( OnError )
            OnError( "Attempt to launch practice-mode outside of main menu!" );
        
        return false;
    }
    
    
    return false;
}

bool SingleplayerLauncher::LaunchQuickMatch( const QuickMatchArguments& Args )
{
    if( LauncherThread )
    {
        cocos2d::log( "[Launcher] Attempt to launch quick-match but the launcher thread is already running!" );
        if( OnError )
            OnError( "Attempt to launch with the launcher thread already running" );
        
        return false;
    }
    
    // Launch game on background thread, progress and results reported through callbacks
    LauncherThread = std::make_shared< std::thread >( [ &, Args ] ()
                             {
                                 this->PerformLaunch( Args.PlayerName, Args.OpponentName, Args.PlayerDeck, Args.OpponentDeck, SingleplayerType::QuickMatch, Args.LevelBackground, Args.Difficulty );
                             } );
    return true;
}

void SingleplayerLauncher::_Thread_Complete( bool bError, const std::string& ErrMessage )
{
    auto sch = cocos2d::Director::getInstance()->getScheduler();
    sch->performFunctionInCocosThread( [ = ] ()
                                      {
                                          // Kill the thread used for launcher
                                          if( LauncherThread )
                                          {
                                              LauncherThread->join();
                                              cocos2d::log( "[Launcher] (DEBUG) Killing thread" );
                                          }
                                          
                                          LauncherThread.reset();
                                          
                                          // Call the callback
                                          if( bError && OnError )
                                          {
                                              OnError( ErrMessage );
                                          }
                                          if( !bError && OnSuccess )
                                          {
                                              OnSuccess();
                                          }
                                      });
}

void SingleplayerLauncher::Error( const std::string& Error )
{
    cocos2d::log( "%s", Error.c_str() );
    _Thread_Complete( true, Error );
}

void SingleplayerLauncher::Success()
{
    cocos2d::log( "[Launcher] Success!" );
    _Thread_Complete( false, std::string() );
}

int CountCards( const Regicide::Deck& inDeck )
{
    int Output = 0;
    for( auto& C : inDeck.Cards )
        Output += C.Ct;
    
    return Output;
}

void SingleplayerLauncher::PerformLaunch( const std::string &PlayerName, const std::string &OpponentName, const Regicide::Deck &PlayerDeck, const Regicide::Deck &OpponentDeck, SingleplayerType Type, uint32 LevelId, AIDifficulty Difficulty, uint32 StoryId /* = 0 */ )
{
    // Perform validation
    if( PlayerName.empty() || OpponentName.empty()  )
    {
        Error( "Invalid arguments (Names)!" );
        return;
    }

    static const int MinCards = 1;
    static const int MaxCards = 70;
    
    auto PlayerCount = CountCards( PlayerDeck );
    auto OpponentCount = CountCards( OpponentDeck );
    
    if( PlayerCount < MinCards || PlayerCount > MaxCards || OpponentCount < MinCards || OpponentCount > MaxCards )
    {
        Error( "Invalid arguments (CardCount)!" );
        return;
    }
    
    if( Type == Game::SingleplayerType::Story && StoryId < 1 )
    {
        Error( "Invalid Arguments (StoryID)" );
        return;
    }
    
    Game::World* NewWorld = CreateWorld();
    if( NewWorld == nullptr )
    {
        Error( "Failed to create world!" );
        return;
    }
    
    // Create GameMode and GameAuthority
    auto* GM = NewWorld->CreateGameMode< SingleplayerGameMode >();
    if( !GM )
    {
        Error( "Failed to create Game Mode!" );
        return;
    }
    
    
    
    cocos2d::log( "[DEBUG] Performing Launch! %s", PlayerName.c_str() );
    Success();
}


Game::World* SingleplayerLauncher::CreateWorld()
{
    if( World::GetWorld() )
    {
        cocos2d::log( "[Launcher] Failed to launch! There already is an active world!" );
        return nullptr;;
    }
    
    auto world = Game::IEntityManager::GetInstance().CreateEntity< Game::World >();
    Game::World::CurrentInstance = world; // So we can call World::GetWorld()
    
    return world;
}
