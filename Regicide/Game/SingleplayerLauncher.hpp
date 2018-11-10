//
//  SingleplayerLauncher.hpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/10/18.
//

#pragma once
#include <functional>               // For std::function
#include "RegicideAPI/Account.h"    // For Regicide::Card, Regicide::Deck
#include <thread>


namespace Game
{
    // Forward Declaration
    class World;
    
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
        Practive
    };
    
    class SingleplayerLauncher
    {
        
    public:
        
        static SingleplayerLauncher& GetInstance();
        
        bool LaunchPractice( const PracticeArguments& Args );
        bool LaunchStory( const StoryArguments& Args );
        bool LaunchQuickMatch( const QuickMatchArguments& Args );
        
        inline void ListenForProgress( const std::function< void( float ) >& Callback ) { OnProgress = Callback; }
        inline void ListenForSuccess( const std::function< void() >& Callback ) { OnSuccess = Callback; }
        inline void ListenForError( const std::function< void( std::string ) >& Callback ) { OnError = Callback; }
        inline void ClearCallbacks() { OnProgress = nullptr; OnSuccess = nullptr; OnError = nullptr; }
        
    private:
        
        // Explicitly disallow copying
        SingleplayerLauncher() {}
        SingleplayerLauncher( const SingleplayerLauncher& Other ) = delete;
        SingleplayerLauncher& operator= ( const SingleplayerLauncher& Other ) = delete;
        
        std::function< void( float ) > OnProgress;
        std::function< void() > OnSuccess;
        std::function< void( std::string ) > OnError;
        std::shared_ptr< std::thread > LauncherThread;
        
        void _Thread_Complete( bool bError, const std::string& ErrMessage );
        
        void PerformLaunch( const std::string& PlayerName, const std::string& OpponentName, const Regicide::Deck& PlayerDeck,
                           const Regicide::Deck& OpponentDeck, SingleplayerType, uint32 LevelId, AIDifficulty Difficulty, uint32 StoryId = 0 );
        void Error( const std::string& Message );
        void Success();
        
        World* CreateWorld();
        
    };
}
