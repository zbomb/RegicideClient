//
//    SingleplayerLauncher.cpp
//    Regicide Mobile
//
//    Created: 11/10/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#include "SingleplayerLauncher.hpp"
#include "AppDelegate.hpp"
#include "World.hpp"
#include "cocos2d.h"
#include "SingleplayerGameMode.hpp"
#include "DeckEntity.hpp"
#include "LuaEngine.hpp"
#include "HandEntity.hpp"
#include "FieldEntity.hpp"
#include "GraveyardEntity.hpp"
#include "SingleplayerAuthority.hpp"
#include "KingEntity.hpp"
#include "AIController.hpp"
#include "ClientState.hpp"


using namespace Game;



SingleplayerLauncher& SingleplayerLauncher::GetInstance()
{
    static SingleplayerLauncher Launcher;
    return Launcher;
}


void SingleplayerLauncher::Launch( const std::string& PlayerName, const std::string& OpponentName, const Regicide::Deck& PlayerDeck,
                                  const Regicide::Deck& OpponentDeck, SingleplayerType, uint32 LevelId, AIDifficulty Difficulty, uint32 StoryId )
{
    if( LauncherThread )
    {
        cocos2d::log( "[Launcher] Attempt to launch singleplayer but the launcher thread is already running!" );
        if( OnError )
            OnError( "Attempt to launch with the launcher thread already running" );
            
        return;
    }
    
    // Free unused textures before loading
    auto Cache = cocos2d::Director::getInstance()->getTextureCache();
    Cache->removeUnusedTextures();
    
    TextureCount = 0;
    
    // Launch game on background thread, progress and results reported through callbacks
    cocos2d::log( "[Launcher] Launch started! Loading entities..." );
    LauncherThread = std::make_shared< std::thread >( [ = ] ()
                                                     {
                                                         this->PerformLaunch( PlayerName, OpponentName, PlayerDeck, OpponentDeck, SingleplayerType::QuickMatch, LevelId, Difficulty );
                                                     } );
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
                                          }
                                          
                                          LauncherThread.reset();
                                          
                                          // Call Init Hooks
                                          if( !bError )
                                          {
                                              auto* world = Game::World::GetWorld();
                                              if( world )
                                                  world->Initialize();
                                          }
                                          
                                          // If everything was successful, then begin loading textures
                                          if( bError && OnError )
                                          {
                                              OnError( ErrMessage );
                                          }
                                          if( !bError && OnSuccess )
                                          {
                                              BeginLoadingTextures();
                                          }
                                      });
}


void SingleplayerLauncher::BeginLoadingTextures()
{
    // Were going to begin loading textures asyncronously, to avoid blocking
    // the main thread, so the loading screen still animates
    // We will wait for all textures to load (or fail) before finishing launch
    TextureCount = 0;
    LoadedTextures = 0;
    bTexturesChecked = false;
    
    auto& EntManager = IEntityManager::GetInstance();
    
    for( auto It = EntManager.Begin(); It != EntManager.End(); It++ )
    {
        if( It->second )
        {
            int resCount = It->second->LoadResources( [ & ]()
            {
                LoadedTextures++;
                
                if( LoadedTextures >= TextureCount && bTexturesChecked )
                {
                    // All done loading!
                    cocos2d::log( "[Launcher] Resources loaded! Creating scene.." );
                    this->OnSuccess();
                }
            } );
            
            TextureCount += resCount;
        }
    }
    
    bTexturesChecked = true;
    
    // Check if textures loaded already, shouldnt ever happen unless all textures are missing
    if( LoadedTextures >= TextureCount )
    {
        this->OnSuccess(); // TODO: Failure when too many textures fail or any?
    }
    
}

void SingleplayerLauncher::Error( const std::string& Error )
{
    _Thread_Complete( true, Error );
}

void SingleplayerLauncher::Success()
{
    cocos2d::log( "[Launcher] Entities loaded! Loading resources..." );
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
    
    if( Type == SingleplayerType::Story && StoryId < 1 )
    {
        Error( "Invalid Arguments (StoryID)" );
        return;
    }
    
    Game::World* NewWorld = CreateWorld();
    if( !NewWorld )
    {
        Error( "Failed to create world!" );
        return;
    }
    
    auto& EntityManager = Game::IEntityManager::GetInstance();
    
    // Create Authority
    auto Authority = EntityManager.CreateEntity< Game::SingleplayerAuthority >();
    
    if( !Authority )
    {
        EntityManager.DestroyEntity( NewWorld );
        Error( "Failed to create authority!" );
        return;
    }
    
    NewWorld->AddChild( Authority );
    NewWorld->Auth = Authority;
    
    // Create Gamemode
    auto* GM = EntityManager.CreateEntity< Game::SingleplayerGameMode >();
    if( !GM )
    {
        EntityManager.DestroyEntity( NewWorld );
        Error( "Failed to create game mode!" );
        return;
    }
    
    // Add to World
    NewWorld->AddChild( GM ); 
    NewWorld->GM = GM;
    
    // Create GameState
    auto* State = EntityManager.CreateEntity< Game::ClientState >();
    if( !State )
    {
        EntityManager.DestroyEntity( NewWorld );
        Error( "Failed to create game state" );
        return;
    }
    
    NewWorld->AddChild( State );
    NewWorld->State = State;
    
    // Begin Loading Authority
    if( !Authority->LoadPlayers( PlayerName, 20, 8, PlayerDeck, OpponentName, 20, 8, OpponentDeck ) )
    {
        EntityManager.DestroyEntity( NewWorld );
        Error( "Failed to load authority" );
        return;
    }
    
    // Now we need to stream AuthState into the client-side of the game
    // During this process, all the entities will be created
    if( !State->StreamFrom( & Authority->GetState() ) )
    {
        EntityManager.DestroyEntity( NewWorld );
        Error( "Failed to stream state into client" );
        return;
    }
    
    // Create AI
    auto NewAI = EntityManager.CreateEntity< Game::AIController >();
    if( !NewAI )
    {
        EntityManager.DestroyEntity( NewWorld );
        Error( "Failed to create AI" );
        return;
    }
    
    // Setup AI
    NewAI->Difficulty   = Difficulty;
     
    Authority->AddChild( NewAI );
    Authority->AI = NewAI;
    
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
    
    return world;
}
