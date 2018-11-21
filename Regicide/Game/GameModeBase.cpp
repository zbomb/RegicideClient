//
//    GameModeBase.cpp
//    Regicide Mobile
//
//    Created: 11/10/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#include "GameModeBase.hpp"
#include "ICardContainer.hpp"
#include "FieldEntity.hpp"
#include "HandEntity.hpp"
#include "AuthorityBase.hpp"
#include "DeckEntity.hpp"
#include "GraveyardEntity.hpp"
#include "KingEntity.hpp"
#include "Scenes/GameScene.hpp"

using namespace Game;


GameModeBase::GameModeBase()
: EntityBase( "GameMode" )
{
    using namespace std::placeholders;
    
    SetActionCallback( "CoinFlip",      std::bind( &GameModeBase::Action_CoinFlip,      this, _1, _2 ) );
    SetActionCallback( "BlitzStart",    std::bind( &GameModeBase::Action_BlitzStart,    this, _1, _2 ) );
    SetActionCallback( "BlitzQuery",    std::bind( &GameModeBase::Action_BlitzQuery,    this, _1, _2 ) );
    SetActionCallback( "MatchStart",    std::bind( &GameModeBase::Action_MainStart,     this, _1, _2 ) );
    SetActionCallback( "BlitzError",    std::bind( &GameModeBase::Action_BlitzError,    this, _1, _2 ) );
    SetActionCallback( "MarshalStart",  std::bind( &GameModeBase::Action_MarshalStart,  this, _1, _2 ) );
    SetActionCallback( "AttackStart",   std::bind( &GameModeBase::Action_AttackStart,   this, _1, _2 ) );
    SetActionCallback( "BlockStart",    std::bind( &GameModeBase::Action_BlockStart,    this, _1, _2 ) );
    SetActionCallback( "PostTurnStart", std::bind( &GameModeBase::Action_PostTurnStart, this, _1, _2 ) );
    SetActionCallback( "TurnStart",     std::bind( &GameModeBase::Action_TurnStart,     this, _1, _2 ) );
    SetActionCallback( "CardDamage",    std::bind( &GameModeBase::Action_CardDamage,    this, _1, _2 ) );
    SetActionCallback( "DamageStart",   std::bind( &GameModeBase::Action_DamageStart,   this, _1, _2 ) );
    
    // Default State
    mState  = MatchState::PreMatch;
    tState  = TurnState::PreTurn;
    pState  = PlayerTurn::LocalPlayer;
}

GameModeBase::~GameModeBase()
{

}

void GameModeBase::Cleanup()
{
    EntityBase::Cleanup();
    
    // We need to stop all action queues
    for( auto It = ActiveQueues.begin(); It != ActiveQueues.end(); It++ )
    {
        It->second.Callback = nullptr;
    }
    
    ActiveQueues.clear();
}

void GameModeBase::UpdateMatchState( MatchState In )
{
    mState = In;
    
    // Update Label
    auto ParentScene = GetScene();
    if( ParentScene )
    {
        auto Scene = dynamic_cast< GameScene* >( ParentScene );
        if( Scene )
        {
            auto mName = std::string();
            switch( In )
            {
                case MatchState::PreMatch:
                    mName = "Pre-Match"; break;
                case MatchState::CoinFlip:
                    mName = "Coin Toss"; break;
                case MatchState::Blitz:
                    mName = "Blitz"; break;
                case MatchState::PostMatch:
                    mName = "Post-Match"; break;
                case MatchState::Main:
                    break;
            }
            
            if( !mName.empty() )
            {
                Scene->UpdateTurnState( mName );
            }
        }
    }
}

void GameModeBase::UpdateTurnState( TurnState In )
{
    tState = In;
    
    // Update Label
    if( mState == MatchState::Main )
    {
        std::string tName;
        auto Scene = dynamic_cast< GameScene* >( GetScene() );
        if( Scene )
        {
            switch( In )
            {
                case TurnState::PreTurn:
                    tName = "Pre-Turn"; break;
                case TurnState::Marshal:
                    tName = "Marshal"; break;
                case TurnState::Attack:
                    tName = "Attack"; break;
                case TurnState::Block:
                    tName = "Block"; break;
                case TurnState::Damage:
                    tName = "Damage"; break;
                case TurnState::PostTurn:
                    tName = "Post-Turn"; break;
                case TurnState::None:
                    tName = "None"; break;
            }
            
            Scene->UpdateTurnState( tName );
        }
    }
}

void GameModeBase::UpdatePlayerTurn( PlayerTurn In )
{
    pState = In;
    
    // Update Label
    if( mState == MatchState::Main )
    {
        auto Scene = dynamic_cast< GameScene* >( GetScene() );
        if( Scene )
        {
            auto pName = std::string();
            switch( In )
            {
                case PlayerTurn::LocalPlayer:
                    pName = "Your Turn"; break;
                case PlayerTurn::Opponent:
                    pName = "Opponent's Turn"; break;
                default:
                    break;
            }
            
            if( !pName.empty() )
            {
                Scene->UpdatePlayerTurn( pName );
            }
        }
    }
}

void GameModeBase::PostInit()
{
    EntityBase::PostInit();
    
    auto dir = cocos2d::Director::getInstance();
    auto Origin = dir->getVisibleOrigin();
    auto Size = dir->getVisibleSize();
    
    auto LocalPlayer = GetPlayer();
    auto Opponent = GetOpponent();
    
    CC_ASSERT( LocalPlayer && Opponent );
    
    // Setup field
    // Set player object location
    LocalPlayer->SetPosition( cocos2d::Vec2( Origin.x + Size.width / 2.f, Origin.y + Size.height * 0.f ) );
    Opponent->SetPosition( cocos2d::Vec2( Origin.x + Size.width / 2.f, Origin.y + Size.height * 1.f ) );
    
    // Set locations of decks
    auto PlayerDeck = LocalPlayer->GetDeck();
    auto OpponentDeck = Opponent->GetDeck();
    
    PlayerDeck->SetPosition( cocos2d::Vec2( Size.width * 0.45f, Size.height * 0.15f ) );
    PlayerDeck->SetRotation( 0.f );
    
    OpponentDeck->SetPosition( cocos2d::Vec2( Size.width * -0.45f, -Size.height * 0.15f ) );
    OpponentDeck->SetRotation( 180.f );
    
    // Invalidate deck Z-Order
    PlayerDeck->InvalidateZOrder();
    OpponentDeck->InvalidateZOrder();
    
    // Set hand positions
    auto PlayerHand = LocalPlayer->GetHand();
    auto OpponentHand = Opponent->GetHand();
    
    PlayerHand->SetPosition( cocos2d::Vec2( 0.f, 0.f) );
    PlayerHand->SetRotation( 0.f );
    OpponentHand->SetPosition( cocos2d::Vec2( 0.f, 0.f ) );
    OpponentHand->SetRotation( 180.f );
    
    auto PlayerField = LocalPlayer->GetField();
    auto OpponentField = Opponent->GetField();
    
    PlayerField->SetPosition( cocos2d::Vec2( 0.f, Size.height * 0.3f ) );
    PlayerField->SetRotation( 0.f );
    OpponentField->SetPosition( cocos2d::Vec2( 0.f, -Size.height * 0.3f ) );
    OpponentField->SetRotation( 180.f );
    
    auto PlayerGrave = LocalPlayer->GetGraveyard();
    auto OpponentGrave = Opponent->GetGraveyard();
    
    PlayerGrave->SetPosition( cocos2d::Vec2( -Size.width * 0.4f, Size.height * 0.25f ) );
    PlayerGrave->SetRotation( 0.f );
    OpponentGrave->SetPosition( cocos2d::Vec2( -Size.width * 0.35f, -Size.height * 0.25f ) );
    OpponentGrave->SetRotation( 180.f );
    
    // Setup starting state on everything
    LocalPlayer->SetMana( 10 );
    Opponent->SetMana( 10 );
    
    auto PlayerKing = LocalPlayer->GetKing();
    auto OpponentKing = Opponent->GetKing();
    
    PlayerKing->SetPosition( cocos2d::Vec2( Size.width / -2.f, 0.f ) );
    OpponentKing->SetPosition( cocos2d::Vec2( Size.width / 2.f, 0.f ) );
    
    UpdateMatchState( MatchState::PreMatch );
    UpdateTurnState( TurnState::None );
    UpdatePlayerTurn( PlayerTurn::None );
    
    // Schedule Tick Function
    cocos2d::Director::getInstance()->getScheduler()->schedule( std::bind( &GameModeBase::Tick, this, std::placeholders::_1 ), this, 0.f, CC_REPEAT_FOREVER, 0.f, false, "GMTick" );
}


void GameModeBase::RunActionQueue( ActionQueue&& In )
{
    // If were in post match, stop all actions
    if( mState == MatchState::PostMatch )
        return;
    
    // Validate
    if( In.ActionTree.empty() )
    {
        cocos2d::log( "[GM] Warning: Attempt to run empty action queue!" );
        if( In.Callback )
            In.Callback();
        return;
    }
    
    // Check if identifier is in use
    auto Identifier = In.Identifier;
    if( ActiveQueues.count( Identifier ) > 0 )
    {
        cocos2d::log( "[GM] Warning: Attempt to run action queue with duplicate id!" );
        return;
    }
    
    // Update execution counter, to track how many of the total actions have been executed
    // This function counts up the total number, and sets the member var 'Counter' to that value
    In.CountActions();

    // Move the action tree into the active list
    In.Executing = true;
    auto NewEntry = ActiveQueues.insert( std::make_pair( Identifier, std::move( In ) ) );
    
    if( NewEntry.second == false )
    {
        cocos2d::log( "[GM] Warning: Failed to add new action to the execution list!" );
        return;
    }

    // Begin execution
    ExecuteActions( NewEntry.first->second.ActionTree, NewEntry.first->first );
    NewEntry.first->second.Executing = false;
    
    // Check if it completed syncronously, and delete it if so
    if( NewEntry.first->second.Counter <= 0 )
    {
        ActiveQueues.erase( NewEntry.first );
    }
}

void GameModeBase::_OnEndOfBranch( uint32_t QueueId )
{
    // Decrement action counter and check if were done executing
    auto It = ActiveQueues.find( QueueId );
    if( It == ActiveQueues.end() )
    {
        cocos2d::log( "[GM] Failed to decrement action counter for queue (%d)", QueueId );
    }
    else
    {
        It->second.Counter--;
        
        if( It->second.Counter <= 0 )
        {
            // Execution finished!
            if( It->second.Callback )
                It->second.Callback();
            
            // If the actions run syncronously, erasing the entry will cause the loop
            // to continue, which will cause an exception to be thrown
            if( !It->second.Executing )
            {
                ActiveQueues.erase( It );
            }
        }
    }
}

void GameModeBase::ExecuteActions( const std::vector<std::unique_ptr< Game::Action > >& In, uint32_t QueueId, bool bRootAction )
{
    // Get iter to the entry in ActiveQueues so we can decremement the action counter
    auto queueIt = ActiveQueues.find( QueueId );
    if( queueIt == ActiveQueues.end() )
    {
        cocos2d::log( "[GM] Failed to decrement action counter for queue (%d)", QueueId );
    }
    else if( !bRootAction )
    {
        // Just decrement, we dont need to check for completion, since there is still
        // children actions to execute
        queueIt->second.Counter--;
    }
    
    for( auto It = In.begin(); It != In.end(); It++ )
    {
        // Check for null actions
        if( !( *It ) )
        {
            cocos2d::log( "[GM] Null action found in action tree (%d)", QueueId );
            
            if( queueIt != ActiveQueues.end() )
                queueIt->second.Counter--;
            
            continue;
        }
        
        // Check if this action has already been started
        if( (*It)->bHasStarted )
        {
            cocos2d::log( "[GM] This action has already started! %s", (*It)->ActionName.c_str() );
            continue;
        }
        

        // Lookup Target Entity
        auto* Target = IEntityManager::GetInstance().GetEntity( (*It)->Target );
        if( !Target )
        {
            cocos2d::log( "[GM] Failed to execute game action \"%s\" in queue (%d) because the target wasnt found [%d]", (*It)->ActionName.c_str(), QueueId, (*It)->Target );
            
            if( queueIt != ActiveQueues.end() )
                queueIt->second.Counter--;
            
            continue;
        }
        
        (*It)->bHasStarted = true;
        
        // Execute Action
        if( (*It)->Children.size() > 0 )
        {
            Target->PerformAction( (*It).get(), std::bind( &GameModeBase::ExecuteActions, this, std::ref( (*It)->Children ), QueueId, false ) );
        }
        else
        {
            Target->PerformAction( It->get(), std::bind( &GameModeBase::_OnEndOfBranch, this, QueueId ) );
        }
    }
}

bool GameModeBase::CanPlayCard( CardEntity* In )
{
    // Dont let player play any cards while actions are in progress
    if( ActiveQueues.size() > 0 )
        return false;
    else
        return CouldPlayCard( In );
}

// We needed a seperate function, that doesnt check if action queues are in progress
bool GameModeBase::CouldPlayCard( CardEntity* In )
{
    if( !In || !In->InHand() )
        return false;
    
    Player* LocalPlayer = GetPlayer();
    
    if( mState != MatchState::Main || tState != TurnState::Marshal || pState != PlayerTurn::LocalPlayer )
        return false;
    
    return In->GetOwningPlayer() == LocalPlayer && In->ManaCost <= LocalPlayer->GetMana();
}

bool GameModeBase::CanCardAttack( CardEntity *In )
{
    if( !In || !In->OnField() )
        return false;
    
    Player* LocalPlayer = GetPlayer();
    if( mState != MatchState::Main || tState != TurnState::Attack || pState != PlayerTurn::LocalPlayer )
        return false;
    
    return In->GetOwningPlayer() == LocalPlayer;
}

bool GameModeBase::CanCardBlock( CardEntity *In, CardEntity *Target )
{
    if( !In || !In->OnField() )
        return false;
    
    Player* LocalPlayer = GetPlayer();
    if( mState != MatchState::Main || tState != TurnState::Block || pState != PlayerTurn::Opponent )
        return false;
    
    return In->GetOwningPlayer() == LocalPlayer;
}

bool GameModeBase::CanTriggerAbility( CardEntity* In )
{
    return false;
}

bool GameModeBase::PlayCard( CardEntity* In, int Index )
{
    if( !CanPlayCard( In ) )
        return false;
    
    auto Auth = GetAuthority< AuthorityBase >();
    if( !Auth )
        return false;
    
    Auth->PlayCard( In, Index );
    return true;
}


void GameModeBase::FinishTurn()
{
    auto Auth = GetAuthority< AuthorityBase >();
    auto Scene = dynamic_cast< GameScene* >( GetScene() );
    CC_ASSERT( Auth && Scene );
    
    // If were in the attack or block phase, we need to send card list
    if( mState == MatchState::Main )
    {
        if( pState == PlayerTurn::LocalPlayer )
        {
            if( tState == TurnState::Attack )
            {
                std::vector< CardEntity* > AttackCards;
                auto pl = GetPlayer();
                auto field = pl ? pl->GetField() : nullptr;
                
                if( field )
                {
                    for( auto It = field->Begin(); It != field->End(); It++ )
                    {
                        if( *It && (*It)->bAttacking )
                            AttackCards.push_back( *It );
                    }
                }
                
                // Check if any cards are selected
                if( AttackCards.size() <= 0 )
                {
                    Auth->FinishTurn();
                }
                else
                {
                    DisableSelection();
                    Auth->SetAttackers( AttackCards );
                }
            }
            else if( tState == TurnState::Marshal )
            {
                Auth->FinishTurn();
            }
        }
        else if( pState == PlayerTurn::Opponent )
        {
            if( tState == TurnState::Block )
            {
                /*
                std::vector< CardEntity* > BlockCards;
                auto pl = GetPlayer();
                auto field = pl ? pl->GetField() : nullptr;
                
                if( field )
                {
                    for( auto It = field->Begin(); It != field->End(); It++ )
                    {
                        if( *It && (*It)->bAttacking )
                            BlockCards.push_back( *It );
                    }
                }
                
                // Check if any cards are selected
                if( BlockCards.size() <= 0 )
                {
                    Auth->FinishTurn();
                }
                else
                {
                    //Auth->SetBlockers( BlockCards );
                }
                 */
                
                // For now, were just going to advnace turn state
                Auth->FinishTurn();
            }
        }
    }
}


void GameModeBase::Action_CoinFlip( Action* In, std::function< void() > Callback )
{
    UpdateMatchState( MatchState::CoinFlip );
    
    // Cast to CoinFlipAction
    auto coinFlip = dynamic_cast< CoinFlipAction* >( In );
    if( !coinFlip )
    {
        cocos2d::log( "[GM] Failed to run coin flip.. invalid action!" );
        Callback();
        return;
    }
    
    if( coinFlip->StartingPlayer == PlayerTurn::LocalPlayer )
        cocos2d::log( "[GM] LOCAL PLAYER STARTS FIRST" );
    else
        cocos2d::log( "[GM] OPPONENT STARTS FIRST" );
    
    UpdatePlayerTurn( coinFlip->StartingPlayer );
    
    // TODO: Run animation
    cocos2d::Director::getInstance()->getScheduler()->schedule( [=]( float d ) { if( Callback ) Callback(); }, this, 1.f, 0, 0.f, false, "CoinFlipCallback" );
}

void GameModeBase::Action_BlitzStart( Action *In, std::function<void ()> Callback )
{
    cocos2d::log( "[GM] Blitz Start!" );
    // Update match state
    UpdateMatchState( MatchState::Blitz );
    
    // TODO: UI Updates?
    Callback();
}

void GameModeBase::Action_BlitzQuery( Action *In, std::function<void ()> Callback )
{
    // Cast to TimedQueryAction
    auto blitzStart = dynamic_cast< TimedQueryAction* >( In );
    if( !blitzStart )
    {
        cocos2d::log( "[GM] Failed to start blitz query! Invalid action cast" );
        Callback();
        return;
    }
    
    cocos2d::log( "[GM] Starting Blitz Query.." );
    
    // Display Timer (TODO)
    // Open Blitz-Mode hand viewer
    auto Pl = GetPlayer();
    auto Hand = Pl ? Pl->GetHand() : nullptr;
    
    if( Hand )
    {
        Hand->OpenBlitzMode();
    }
    
    Callback();
}

void GameModeBase::Action_BlitzError( Action *In, std::function<void ()> Callback )
{
    auto& EntManager = IEntityManager::GetInstance();
    auto Pl = GetPlayer();
    auto Hand = Pl ? Pl->GetHand() : nullptr;
    
    // Re-enable blitz selection
    if( Hand )
        Hand->EnabledBlitzMenu();
    
    auto Error = dynamic_cast< CardErrorAction* >( In );
    if( !Error )
    {
        cocos2d::log( "[GM] Invalid Blitz cards selected, but couldnt read error message!" );
        Callback();
        return;
    }
    
    for( auto It = Error->Errors.begin(); It != Error->Errors.end(); It++ )
    {
        // Attempt to lookup card
        auto Card = EntManager.GetEntity< CardEntity >( It->first );
        if( Card )
        {
            cocos2d::log( "[GM] Blitz Error: Card (%s) couldnt be selected.. error message #%d", Card->DisplayName.c_str(), It->second );
            
            // Deselect the card
            if( Hand )
                Hand->DeselectBlitz( Card );
        }
        else
        {
            cocos2d::log( "[GM] Blitz Error: Card (%d) cant be selected.. error message #%d", It->first, It->second );
        }
    }
}

void GameModeBase::Action_AttackError( Action *In, std::function<void ()> Callback )
{
    EnableSelection();
    
    auto Error = dynamic_cast< CardErrorAction* >( In );
    if( !Error )
    {
        cocos2d::log( "[GM] Invalid attack cards selected, but couldnt read error message!" );
        Callback();
        return;
    }
    
    auto& EntManager = IEntityManager::GetInstance();
    
    for( auto It = Error->Errors.begin(); It != Error->Errors.end(); It++ )
    {
        // Attempt to lookup card
        auto Card = EntManager.GetEntity< CardEntity >( It->first );
        if( Card )
        {
            cocos2d::log( "[GM] Attack Error: Card (%s) couldnt attack.. error message #%d", Card->DisplayName.c_str(), It->second );
            
            // Deselect the card
            Card->bAttacking = false;
            Card->ClearOverlay();
            Card->ClearHighlight();
        }
        else
        {
            cocos2d::log( "[GM] Blitz Error: Card (%d) cant be selected.. error message #%d", It->first, It->second );
        }
    }
}

void GameModeBase::Action_MainStart( Action *In, std::function<void ()> Callback )
{
    // Advance Match State
    UpdateMatchState( MatchState::Main );
    cocos2d::log( "[GM] Match Started..." );
    
    // Close blitz menu
    auto Pl = GetPlayer();
    auto Hand = Pl ? Pl->GetHand() : nullptr;
    
    if( Hand )
    {
        Hand->CloseBlitzMode();
    }
    
    // Allow 0.25s of grace time for blitz-selector animation
    cocos2d::Director::getInstance()->getScheduler()->schedule( [=] ( float f ) { if( Callback ) Callback(); }, this, 0.3f, 0, 0.f, false, "FinishSelector" );
}

void GameModeBase::Action_TurnStart( Action* In, std::function< void() > Callback )
{
    auto turnStart = dynamic_cast< TurnStartAction* >( In );
    if( !turnStart )
    {
        cocos2d::log( "[GM] Action Error: Cast to TurnStartAction failed!" );
        Callback();
        return;
    }
    
    // Update State
    UpdateMatchState( MatchState::Main );
    UpdatePlayerTurn( turnStart->pState );
    UpdateTurnState( TurnState::PreTurn );
    
    // TODO: Update UI
    
    Callback();
}

void GameModeBase::Action_MarshalStart( Action* In, std::function< void() > Callback )
{
    UpdateMatchState( MatchState::Main );
    UpdateTurnState( TurnState::Marshal );
    
    // TODO: Update UI
    cocos2d::log( "[GM] Marshal Started..." );
    _bCheckPossibleActions = true;
    
    EnableSelection();
    
    // Enable Finish Button
    auto scene = dynamic_cast< GameScene* >( GetScene() );
    
    if( pState == PlayerTurn::LocalPlayer )
    {
        if( scene )
            scene->ShowFinishButton();
    }
    else
    {
        if( scene )
            scene->HideFinishButton();
    }
    
    cocos2d::Director::getInstance()->getScheduler()->schedule( [=]( float d ) { if( Callback ) Callback(); }, this, 0.25f, 0, 0.f, false, "FinishMarshalStart" );
}

void GameModeBase::Action_AttackStart( Action* In, std::function< void() > Callback )
{
    UpdateMatchState( MatchState::Main );
    UpdateTurnState( TurnState::Attack );
    
    // TODO: Update UI
    cocos2d::log( "[GM] Attack Started..." );
    _bCheckPossibleActions = true;
    
    EnableSelection();
    
    auto scene = dynamic_cast< GameScene* >( GetScene() );
    
    if( pState == PlayerTurn::LocalPlayer )
    {
        if( scene )
            scene->ShowFinishButton();
    }
    else
    {
        if( scene )
            scene->HideFinishButton();
    }
    
    cocos2d::Director::getInstance()->getScheduler()->schedule( [=]( float d ) { if( Callback ) Callback(); }, this, 0.25f, 0, 0.f, false, "FinishAttackStart" );
}

void GameModeBase::Action_BlockStart( Action* In, std::function< void() > Callback )
{
    UpdateMatchState( MatchState::Main );
    UpdateTurnState( TurnState::Block );
    
    EnableSelection();
    
    auto scene = dynamic_cast< GameScene* >( GetScene() );
    
    if( pState == PlayerTurn::Opponent )
    {
        if( scene )
            scene->ShowFinishButton();
    }
    else
    {
        if( scene )
            scene->HideFinishButton();
    }
    
    cocos2d::log( "[GM] Block Started..." );
    _bCheckPossibleActions = true;
    
    cocos2d::Director::getInstance()->getScheduler()->schedule( [=]( float d ) { if( Callback ) Callback(); }, this, 0.25f, 0, 0.f, false, "FinishBlockStart" );
}

void GameModeBase::Action_PostTurnStart( Action *In, std::function<void ()> Callback )
{
    UpdateMatchState( MatchState::Main );
    UpdateTurnState( TurnState::PostTurn );
    
    auto scene = dynamic_cast< GameScene* >( GetScene() );
    if( scene )
        scene->HideFinishButton();
    
    cocos2d::log( "[GM] Post Turn Started..." );
    _bCheckPossibleActions = true;
    
    cocos2d::Director::getInstance()->getScheduler()->schedule( [=]( float d ) { if( Callback ) Callback(); }, this, 0.25f, 0, 0.f, false, "FinishPostTurnStart" );
}

void GameModeBase::Action_DamageStart( Action* In, std::function< void() > Callback )
{
    UpdateMatchState( MatchState::Main );
    UpdateTurnState( TurnState::Damage );
    
    auto scene = dynamic_cast< GameScene* >( GetScene() );
    if( scene )
        scene->HideFinishButton();
    
    cocos2d::log( "[GM] Damage Phase Started" );
    Callback();
}

void GameModeBase::Action_CardDamage( Action *In, std::function<void ()> Callback )
{
    auto Damage = dynamic_cast< DamageAction* >( In );
    if( !Damage )
    {
        cocos2d::log( "[GM] Received card damage, but the event was invalid!" );
        Callback();
        return;
    }
    
    // Update Health of both attacker, and blocker
    if( Damage->Target )
    {
        Damage->Target->Power = Damage->TargetPower;
        
        // Check for death
        if( Damage->Target->Power <= 0 )
        {
            cocos2d::log( "[GM] Blocking Card Died!" );
            auto cont = Damage->Target->GetContainer();
            if( cont )
                cont->Remove( Damage->Target );
            
            IEntityManager::GetInstance().DestroyEntity( Damage->Target );
        }
    }
    
    if( Damage->Inflictor )
    {
        // Update Power
        Damage->Inflictor->Power = Damage->InflictorPower;
        
        if( Damage->Inflictor->Power <= 0 )
        {
            cocos2d::log( "[GM] Attacking Card Died!" );
            
            auto cont = Damage->Inflictor->GetContainer();
            if( cont )
                cont->Remove( Damage->Inflictor );
            
            IEntityManager::GetInstance().DestroyEntity( Damage->Inflictor );
        }
    }
    
    cocos2d::Director::getInstance()->getScheduler()->schedule( [=]( float f ) { if( Callback ) Callback(); }, this, 0.5f, 0, 0.f, false, "DamageCallback" );
}

void GameModeBase::Tick( float Delta )
{
    if( _bCheckPossibleActions )
    {
        _bCheckPossibleActions = false;
        std::vector< CardEntity* > ActionableCards;
        bool bPlayersMove = false;
        auto Pl = GetPlayer();
        auto Hand = Pl ? Pl->GetHand() : nullptr;
        auto Field = Pl ? Pl->GetField() : nullptr;
        
        if( mState == MatchState::Main )
        {
            if( pState == PlayerTurn::LocalPlayer )
            {
                if( tState == TurnState::Marshal )
                {
                    // Check if any cards in hand can be played
                    if( Hand )
                    {
                        for( auto It = Hand->Begin(); It != Hand->End(); It++ )
                        {
                            if( *It && CouldPlayCard( *It ) )
                            {
                                ActionableCards.push_back( *It );
                            }
                        }
                    }
                    
                    // Check if any card abilities can be triggered
                    if( Field )
                    {
                        for( auto It = Field->Begin(); It != Field->End(); It++ )
                        {
                            if( *It && CanTriggerAbility( *It ) )
                            {
                                ActionableCards.push_back( *It );
                            }
                        }
                    }
                    
                    bPlayersMove = true;
                }
                else if( tState == TurnState::Attack )
                {
                    // Check if the player can attack with any cards
                    if( Field )
                    {
                        for( auto It = Field->Begin(); It != Field->End(); It++ )
                        {
                            if( *It && CanCardAttack( *It ) )
                            {
                                ActionableCards.push_back( *It );
                            }
                        }
                    }
                    
                    bPlayersMove = true;
                }
            }
            else if( pState == PlayerTurn::Opponent )
            {
                if( tState == TurnState::Block )
                {
                    // Check if the player can block with any cards
                    if( Field )
                    {
                        for( auto It = Field->Begin(); It != Field->End(); It++ )
                        {
                            if( *It && CanCardBlock( *It ) )
                            {
                                ActionableCards.push_back( *It );
                            }
                        }
                    }
                    
                    bPlayersMove = true;
                }
            }
        }
        
        // If the game is waiting on the player, check if they have any possible
        // moves they can make, highlight them, or if not, advance round state
        if( bPlayersMove )
        {
            if( ActionableCards.empty() )
            {
                FinishTurn();
            }
        }
        
        // Loop through all cards, and clear the highlight
        auto Cards = IEntityManager::GetInstance().GetAllCards();
        for( auto It = Cards.begin(); It != Cards.end(); It++ )
        {
            if( *It )
            {
                // Check if we can clear attack/block overlay
                if( tState == TurnState::PostTurn || tState == TurnState::PreTurn )
                {
                    (*It)->ClearOverlay();
                    (*It)->bAttacking = false;
                }
                
                // Allow highlight to stay if were attacking, it will still get cleared
                // when TurnState reaches Post/PreTurn
                if( !(*It)->bAttacking )
                    (*It)->ClearHighlight();
            }
        }
        
        if( !ActionableCards.empty() )
        {
            auto highlightColor = cocos2d::Color3B();
            if( tState == TurnState::Marshal )
                highlightColor = cocos2d::Color3B( 30, 50, 240 );
            else if( tState == TurnState::Attack )
                highlightColor = cocos2d::Color3B( 240, 30, 30 );
            else if( tState == TurnState::Block )
                highlightColor = cocos2d::Color3B( 30, 50, 240 );
            else
                return;
            
            // Loop through the actionable cards and highlight in blue
            for( auto It = ActionableCards.begin(); It != ActionableCards.end(); It++ )
            {
                if( *It )
                {
                    (*It)->SetHighlight( highlightColor, 180 );
                }
            }
        }
    }
}
