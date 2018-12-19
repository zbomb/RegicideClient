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
#include "CardEntity.hpp"


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
    
    // Begin Loading Authority
    if( !Authority->LoadPlayers( PlayerName, 20, 8, PlayerDeck, OpponentName, 20, 8, OpponentDeck ) )
    {
        EntityManager.DestroyEntity( NewWorld );
        Error( "Failed to load authority" );
        return;
    }
    
    // Now we need to stream AuthState into the client-side of the game
    // During this process, all the entities will be created
    if( !StreamEntities( GM, Authority ) )
    {
        EntityManager.DestroyEntity( NewWorld );
        Error( "Failed to stream entities" );
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
    
    NewAI->Difficulty = Difficulty;
     
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

bool StreamContainer( std::vector< CardState >& Source, Player* Target, ICardContainer* Container )
{
    auto& CM = CardManager::GetInstance();
    
    if( !Target || !Container )
    {
        cocos2d::log( "[Launcher] Failed to stream card container.. player or container was null" );
        return false;
    }
    
    for( auto It = Source.begin(); It != Source.end(); It++ )
    {
        if( It->EntId <= 0 )
        {
            cocos2d::log( "[Launcher] Warning! Auth card has an invalid entity id!" );
            continue;
        }
        
        auto* Card = CM.CreateCard( *It, Target, true );
        if( !Card )
        {
            cocos2d::log( "[Launcher] Warning! Failed to create new card!" );
            continue;
        }
        
        Container->AddToTop( Card, true );
    }
    
    return true;
}

bool SingleplayerLauncher::StreamPlayer( PlayerState* Source, Player* Target, bool bOpponent )
{
    if( !Source || !Target )
    {
        cocos2d::log( "[Launcher] Failed to stream player.. null!" );
        return false;
    }
    
    Target->SetHealth( Source->Health );
    Target->SetMana( Source->Mana );
    
    // Stream King Entity
    auto King = Target->GetKing();
    if( !King )
    {
        cocos2d::log( "[Launcher] Failed to stream player.. king is null!" );
        return false;
    }
    
    King->OwningPlayer = Target;
    King->Load( Source->King, bOpponent );

    return( StreamContainer( Source->Deck, Target, Target->GetDeck() ) &&
            StreamContainer( Source->Hand, Target, Target->GetHand() ) &&
            StreamContainer( Source->Field, Target, Target->GetField() ) &&
            StreamContainer( Source->Graveyard, Target, Target->GetGraveyard() ) );
}

bool SingleplayerLauncher::StreamEntities( GameModeBase* Target, AuthorityBase* Source )
{
    if( !Target || !Source )
        return false;
    
    // We need to build the GameMode state from the Authority State
    auto& AuthState = Source->GetState();
    auto& ClientState = Target->GetState();
    
    ClientState.mState = AuthState.mState;
    ClientState.pState = AuthState.pState;
    ClientState.tState = AuthState.tState;
    
    auto& Ent = IEntityManager::GetInstance();
    
    auto AuthPlayer    = AuthState.GetPlayer();
    auto AuthOpponent  = AuthState.GetOpponent();
    
    if( !AuthPlayer || AuthPlayer->EntId <= 0 || !AuthOpponent || AuthOpponent->EntId <= 0 )
    {
        cocos2d::log( "[Launcher] Failed to stream entities! Auth version is invalid (Player ID)" );
        return false;
    }
    
    ClientState.LocalPlayer     = Ent.CreateEntity< Player >( AuthPlayer->EntId );
    ClientState.Opponent        = Ent.CreateEntity< Player >( AuthOpponent->EntId );
    
    if( !ClientState.LocalPlayer || !ClientState.Opponent )
    {
        cocos2d::log( "[Launcher] Failed to stream entities! Couldnt create players" );
        
        if( ClientState.LocalPlayer )
            Ent.DestroyEntity( ClientState.LocalPlayer );
        if( ClientState.Opponent )
            Ent.DestroyEntity( ClientState.Opponent );
        
        return false;
    }
    
    Target->AddChild( ClientState.LocalPlayer );
    Target->AddChild( ClientState.Opponent );
    
    auto LocalHand = ClientState.LocalPlayer->GetHand();
    auto OppHand = ClientState.Opponent->GetHand();
    
    if( LocalHand )
        LocalHand->bVisibleLocally = true;
    if( OppHand )
        OppHand->bVisibleLocally = false;
    
    // Now we just need to stream the card containers for each player
    if( !StreamPlayer( AuthPlayer, ClientState.LocalPlayer, false ) )
    {
        cocos2d::log( "[Launcher] Failed to stream local player info!" );
        return false;
    }
    
    if( !StreamPlayer( AuthOpponent, ClientState.Opponent, true ) )
    {
        cocos2d::log( "[Launcher] Fialed to stream opponent info!" );
        return false;
    }
    
    return true;
}
