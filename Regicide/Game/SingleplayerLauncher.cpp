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
    
    // Get cache ref, so when we load cards we can also preload textures for them
    auto cache = cocos2d::Director::getInstance()->getTextureCache();
    
    auto& EntityManager = Game::IEntityManager::GetInstance();
    
    // Create Authority
    auto Authority = EntityManager.CreateEntity< Game::SingleplayerAuthority >();
    
    if( !Authority )
    {
        EntityManager.DestroyEntity( Authority );
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
    
    auto dir = cocos2d::Director::getInstance();
    auto Origin = dir->getVisibleOrigin();
    
    // Create players
    auto NewLocalPlayer = CreatePlayer( PlayerName, PlayerDeck, cache, false );
    
    if( !NewLocalPlayer )
    {
        // Rollback
        EntityManager.DestroyEntity( NewWorld );
        Error( "Failed to create Local Player!" );
        return;
    }
    
    // Ensure local player hand flips cards face up
    NewLocalPlayer->Hand->bVisibleLocally = true;
    
    NewWorld->AddChild( NewLocalPlayer );
    NewWorld->LocalPlayer = NewLocalPlayer;
    
    auto NewOpponent = CreatePlayer( OpponentName, OpponentDeck, cache, true );
    
    if( !NewOpponent )
    {
        EntityManager.DestroyEntity( NewWorld );
        Error( "Failed to create opponent" );
        return;
    }
    
    NewWorld->AddChild( NewOpponent );
    NewWorld->Opponent = NewOpponent;
    
    Success();
}


Game::Player* SingleplayerLauncher::CreatePlayer( const std::string &DisplayName, const Regicide::Deck &inDeck, cocos2d::TextureCache* cache, bool bOpponent /* = false */ )
{
    // Create new player entity
    auto& EntityManager = Game::IEntityManager::GetInstance();
    
    // Attempt to setup what we need in lua now, so if it fails, we dont have to perform rollback
    auto Lua = Regicide::LuaEngine::GetInstance();
    auto* L = Lua->State();
    CC_ASSERT( Lua );
    
    // Get reference to deep-copy function in Lua
    auto DeepCopy = luabridge::getGlobal( L, "DeepCopy" );
    if( !DeepCopy.isFunction() )
    {
        cocos2d::log( "[Launcher] Fatal Error! Failed to find deep-copy function in Lua!" );
        return nullptr;
    }
    
    auto* NewPlayer = EntityManager.CreateEntity< Game::Player >();
    if( !NewPlayer )
        return nullptr;
    
    // Set Player Traits
    NewPlayer->DisplayName = DisplayName;
    NewPlayer->bOpponent = bOpponent;
    NewPlayer->Health   = 30;
    
    // Load King
    auto KingTable = luabridge::newTable( L );
    luabridge::setGlobal( L, KingTable, "KING" );
    
    if( !Lua->RunScript( "kings/" + std::to_string( inDeck.KingId ) + ".lua" ) )
    {
        cocos2d::log( "[Launcher] Failed to load king with id: %d", inDeck.KingId );
        
        // Reset KING to nil
        luabridge::setGlobal( L, luabridge::LuaRef( L ), "KING" );
        
        // Rollback Function
        EntityManager.DestroyEntity( NewPlayer );
        return nullptr;
    }
    
    auto* newKing = EntityManager.CreateEntity< Game::KingEntity >();
    if( !newKing )
    {
        // Rollback Function
        EntityManager.DestroyEntity( NewPlayer );
        luabridge::setGlobal( L, luabridge::LuaRef( L ), "KING" );
        return nullptr;
    }
    
    if( !newKing->Load( KingTable, NewPlayer, cache, bOpponent ) )
    {
        EntityManager.DestroyEntity( NewPlayer );
        luabridge::setGlobal( L, luabridge::LuaRef( L ), "KING" );
        return nullptr;
    }
    
    // Reset KING global in Lua
    luabridge::setGlobal( L, luabridge::LuaRef( L ), "KING" );
    
    NewPlayer->AddChild( newKing );
    NewPlayer->King = newKing;
    
    newKing->UpdateHealth( NewPlayer->Health );
    
    // Create Deck
    auto* NewDeck = EntityManager.CreateEntity< Game::DeckEntity >();
    if( !NewDeck )
    {
        // Cleanup player
        EntityManager.DestroyEntity( NewPlayer );
        return nullptr;
    }
    
    auto dir = cocos2d::Director::getInstance();
    auto Origin = dir->getVisibleOrigin();
    
    // Set Deck Traits
    NewDeck->DisplayName    = inDeck.Name;
    NewDeck->DeckId         = inDeck.Id;
    
    NewPlayer->AddChild( NewDeck );
    NewPlayer->Deck = NewDeck;
    
    // Begin Creating Cards
    for( auto& C : inDeck.Cards )
    {
        if( C.Ct <= 0 )
            continue;
        
        // We need to load the lua table for this card, and then we will just perform a copy for each
        // instance of this card, instead of reloading the table multiple times
        // First, we need to create the global 'CARD' table
        auto CardTable = luabridge::newTable( L );
        luabridge::setGlobal( L, CardTable, "CARD" );
        
        // Now we need to load the file
        if( !Lua->RunScript( "cards/" + std::to_string( C.Id ) + ".lua" ) )
        {
            cocos2d::log( "[Launcher] Failed to load card with id '%d'", C.Id );
            
            // Set CARD to nil
            luabridge::setGlobal( L, luabridge::LuaRef( L ), "CARD" );
            continue;
        }
        
        for( int i = 0; i < C.Ct; i++ )
        {
            // Copy table
            luabridge::LuaRef newTable( L );
            
            try
            {
                newTable = DeepCopy( CardTable );
            }
            catch( std::exception& ex )
            {
                cocos2d::log( "[Launcher] Failed to create card, couldnt call DeepCopy!" );
                continue;
            }
            
            // Create new card entity and load from Lua
            auto* NewCard = EntityManager.CreateEntity< CardEntity >();
            if( !NewCard )
            {
                cocos2d::log( "[Launcher] Failed to create card entity!" );
                continue;
            }
            
            if( !NewCard->Load( newTable, NewPlayer, cache ) )
            {
                cocos2d::log( "[Launcher] Failed to load a card '%d' for player!", C.Id );
                EntityManager.DestroyEntity( NewCard );
                continue;
            }
            
            // Add to deck
            // Were going to set ownership to Player instead of Deck, since the cards will be moving between
            // containers, exchanging ownership on every move wouldnt be as elegant
            NewPlayer->AddChild( NewCard );
            NewDeck->AddAtRandom( NewCard, false );
        }
        
        // Reset CARD global to nil
        luabridge::setGlobal( L, luabridge::LuaRef( L ), "CARD" );
    }
    
    // Create Hand
    auto* NewHand = EntityManager.CreateEntity< HandEntity >();
    
    if( !NewHand )
    {
        EntityManager.DestroyEntity( NewPlayer );
        return nullptr;
    }
    
    NewPlayer->AddChild( NewHand );
    NewPlayer->Hand = NewHand;

    // Create playing field
    auto* NewField = EntityManager.CreateEntity< FieldEntity >();
    
    if( !NewField )
    {
        EntityManager.DestroyEntity( NewPlayer );
        return nullptr;
    }
    
    NewPlayer->AddChild( NewField );
    NewPlayer->Field = NewField;
    
    // Create Graveyard
    auto* NewGraveyard = EntityManager.CreateEntity< GraveyardEntity >();
    
    if( !NewGraveyard )
    {
        EntityManager.DestroyEntity( NewPlayer );
        return nullptr;
    }
    
    NewPlayer->AddChild( NewGraveyard );
    NewPlayer->Graveyard = NewGraveyard;
    
    // TODO: Graveyard setup?
    
    
    return NewPlayer;
    
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
