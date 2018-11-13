//
//  SingleplayerLauncher.hpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/10/18.
//

#pragma once
#include <functional>               // For std::function
#include "RegicideAPI/Account.h"    // For Regicide::Card, Regicide::Deck
#include "AppDelegate.h"
#include <thread>


namespace Game
{
    // Forward Declaration
    class World;
    class Player;

    
    class SingleplayerLauncher
    {
        
    public:
        
        static SingleplayerLauncher& GetInstance();
        
        void Launch( const std::string& PlayerName, const std::string& OpponentName, const Regicide::Deck& PlayerDeck,
                    const Regicide::Deck& OpponentDeck, SingleplayerType, uint32 LevelId, AIDifficulty Difficulty, uint32 StoryId = 0 );
        
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
        Player* CreatePlayer( const std::string& DisplayName, const Regicide::Deck& inDeck, cocos2d::TextureCache* inCache );
        
        void BeginLoadingTextures();
        int TextureCount = 0;
        int LoadedTextures = 0;
        bool bTexturesChecked = false;
        
    };
}
