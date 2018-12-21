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
#include "SingleplayerAuthority.hpp"

using namespace Game;


GameModeBase::GameModeBase()
: EntityBase( "GameMode" )
{
    using namespace std::placeholders;
    
    AddAction( "PlayCard",      std::bind( &GameModeBase::OnCardPlay,       this, _1, _2 ) );
    AddAction( "UpdateMana",    std::bind( &GameModeBase::OnManaUpdate,     this, _1, _2 ) );
    AddAction( "DrawCard",      std::bind( &GameModeBase::OnCardDraw,       this, _1, _2 ) );
    AddAction( "CoinFlip",      std::bind( &GameModeBase::OnCoinFlip,       this, _1, _2 ) );
    AddAction( "BlitzStart",    std::bind( &GameModeBase::OnBlitzStart,     this, _1, _2 ) );
    AddAction( "BlitzQuery",    std::bind( &GameModeBase::OnBlitzQuery,     this, _1, _2 ) );
    AddAction( "BlitzError",    std::bind( &GameModeBase::OnBlitzError,     this, _1, _2 ) );
    AddAction( "BlitzSuccess",  std::bind( &GameModeBase::OnBlitzSuccess,   this, _1, _2 ) );
    AddAction( "AttackError",   std::bind( &GameModeBase::OnAttackError,    this, _1, _2 ) );
    AddAction( "MatchStart",    std::bind( &GameModeBase::OnMatchStart,     this, _1, _2 ) );
    AddAction( "TurnStart",     std::bind( &GameModeBase::OnTurnStart,      this, _1, _2 ) );
    AddAction( "MarshalStart",  std::bind( &GameModeBase::OnMarshalStart,   this, _1, _2 ) );
    AddAction( "AttackStart",   std::bind( &GameModeBase::OnAttackStart,    this, _1, _2 ) );
    AddAction( "BlockStart",    std::bind( &GameModeBase::OnBlockStart,     this, _1, _2 ) );
    AddAction( "TurnFinish",    std::bind( &GameModeBase::OnTurnFinish,     this, _1, _2 ) );
    AddAction( "DamageStart",   std::bind( &GameModeBase::OnDamageStart,    this, _1, _2 ) );
    AddAction( "Damage",        std::bind( &GameModeBase::OnDamage,         this, _1, _2 ) );
    AddAction( "Combat",        std::bind( &GameModeBase::OnCombat,         this, _1, _2 ) );
    AddAction( "UpdateStamina", std::bind( &GameModeBase::OnStaminaUpdate,  this, _1, _2 ) );
    AddAction( "CleanupBoard",  std::bind( &GameModeBase::OnBoardCleanup,   this, _1, _2 ) );
    AddAction( "BattleMatrix",  std::bind( &GameModeBase::OnMatrixUpdate,   this, _1, _2 ) );
    
    State.mState = MatchState::PreMatch;
    State.pState = PlayerTurn::None;
    State.tState = TurnState::None;
    
    lastDamageId        = 0;
    lastDrainId         = 0;
    
    _Viewer         = nullptr;
    _touchedCard    = nullptr;
    _viewCard       = nullptr;
    _graveViewer    = nullptr;
    
    _bSelectionEnabled  = true;
    TurnFinished        = false;
    // TODO: Require UI Textures Here
    
}

GameModeBase::~GameModeBase()
{
    _Viewer         = nullptr;
    _touchedCard    = nullptr;
    _viewCard       = nullptr;
    _graveViewer    = nullptr;
}

void GameModeBase::Initialize()
{
    EntityBase::Initialize();
    
    _bSelectionEnabled = true;
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
    
    _DoCloseViewer();
    CloseGraveyardViewer();
}


/*==========================================================================================================
    GameModeBase -> Input Logic
==========================================================================================================*/
void GameModeBase::TouchBegan( cocos2d::Touch *inTouch, CardEntity *inCard )
{
    if( !_bSelectionEnabled )
    {
        _bDrag = false;
        if( _touchedCard )
            _touchedCard->SetIsDragging( false );
        
        _touchedCard = nullptr;
        return;
    }
    
    _touchedCard = inCard;
    _TouchStart = inTouch->getLocation();
    
    if( inCard && inCard->Sprite && inTouch )
    {
        // Set drag offset to the difference of the card                                                                                                               center
        // and the click position
        _DragOffset = inCard->Sprite->getPosition() - inTouch->getLocation();
    }
}


void GameModeBase::TouchEnd( cocos2d::Touch *inTouch, CardEntity *inCard, cocos2d::DrawNode* inDraw )
{
    if( !_bSelectionEnabled )
    {
        _bDrag = false;
        if( _touchedCard )
            _touchedCard->SetIsDragging( false );
        
        _touchedCard = nullptr;
        return;
    }
    
    auto dir = cocos2d::Director::getInstance();
    
    // First, check if we were dragging a card
    if( _bDrag )
    {
        if( _touchedCard )
        {
            // Call DragDrop handler to see if this was a valid operation
            // If not, were going to release the card back to the original container
            bool bResult = OnCardDragDrop( _touchedCard, inTouch );
            _touchedCard->SetIsDragging( false );
            auto cont = _touchedCard->GetContainer();
            
            if( !bResult )
            {
                if( cont )
                    cont->InvalidateCards();
            }
        }
        
        _bDrag = false;
    }
    else
    {
        // Check if were setting blockers
        if( _bBlockerDrag )
        {
            if( !OnBlockerSelect( _touchedCard, inCard ) )
            {
                if( BlockMatrix.count( _touchedCard ) > 0 )
                {
                    BlockMatrix.erase( _touchedCard );
                    RedrawBlockers();
                }
            }
            
            if( inDraw )
                inDraw->clear();
            
            _bBlockerDrag = false;
        }
        else
        {
            auto delta = inTouch->getLocation() - _TouchStart;
            auto Size = dir->getVisibleSize();
            
            if( _touchedCard && _touchedCard->OnField() && !_TouchStart.isZero() &&
               abs( delta.x ) < Size.width / 50.f && delta.length() > Size.width / 20.f )
            {
                OnCardSwipedUp( _touchedCard );
            }
            else if( inCard && inCard == _touchedCard )
            {
                OnCardClicked( inCard );
            }
            else
            {
                CloseCardViewer();
                CloseHandViewer();
                CloseGraveyardViewer();
            }
        }
    }
    
    _DragOffset = cocos2d::Vec2::ZERO;
    _TouchStart = cocos2d::Vec2::ZERO;
    _touchedCard = nullptr;
}


void GameModeBase::TouchMoved( cocos2d::Touch *inTouch, cocos2d::DrawNode* inDraw )
{
    if( !_bSelectionEnabled )
    {
        _bDrag = false;
        if( _touchedCard )
            _touchedCard->SetIsDragging( false );
        
        _touchedCard = nullptr;
        return;
    }
    
    // Check if card is being dragged & in player's hand
    if( _touchedCard && !_touchedCard->GetIsDragging() && _touchedCard->InHand() )
    {
        // Check if the gamemode thinks we can even play this card
        if( CanPlayCard( _touchedCard ) )
        {
            // We need to make sure that its fairly explicit that the user is trying to drag and not click
            auto deltaVec = inTouch->getDelta();
            
            if( deltaVec.length() > 9.f )
            {
                _touchedCard->SetIsDragging( true );
                _bDrag = true;
            }
        }
    }
    else if( _touchedCard && !_bBlockerDrag && State.tState == TurnState::Block && State.pState == PlayerTurn::Opponent )
    {
        if( CanCardBlock( _touchedCard ) )
        {
            auto delta = inTouch->getDelta();
            if( delta.length() > 9.f )
            {
                _bBlockerDrag = true;
            }
        }
    }
    
    // Position card while dragging
    if( _bDrag && _touchedCard && inTouch &&_touchedCard->Sprite )
    {
        // Move card with the touch
        auto pos = inTouch->getLocation();
        _touchedCard->Sprite->setPosition( pos + _DragOffset );
    }
    else if( _bBlockerDrag && _touchedCard && inTouch && inDraw )
    {
        inDraw->clear();
        inDraw->drawSolidCircle( inTouch->getLocation(), 32.f, 360.f, 32, 1.f, 1.f, cocos2d::Color4F( 0.2f, 0.2f, 0.95f, 0.6f ) );
        inDraw->drawSolidCircle( _touchedCard->GetAbsolutePosition(), 32.f, 360.f, 32, 1.f, 1.f, cocos2d::Color4F( 0.2f, 0.2f, 0.95f, 0.6f ) );
        inDraw->drawSegment( _touchedCard->GetAbsolutePosition(), inTouch->getLocation(), 10.f, cocos2d::Color4F( 0.2f, 0.2f, 0.95f, 0.6f ) );
        
    }
}


void GameModeBase::TouchCancel( cocos2d::Touch* inTouch, cocos2d::DrawNode* inDraw )
{
    if( !_bSelectionEnabled )
    {
        _bDrag = false;
        if( _touchedCard )
            _touchedCard->SetIsDragging( false );
        
        _touchedCard = nullptr;
        return;
    }
    
    // Stop dragging
    if( _bDrag && _touchedCard )
    {
        _touchedCard->SetIsDragging( false );
        
        // Force the container that holds this card to reposition
        // so the sprite will automtaically animate back
        auto cont = _touchedCard->GetContainer();
        if( cont )
            cont->InvalidateCards();
    }
    
    if( inDraw )
        inDraw->clear();
    
    _DragOffset = cocos2d::Vec2::ZERO;
    _bDrag = false;
    _touchedCard = nullptr;
    _bBlockerDrag = false;
}

void GameModeBase::RedrawBlockers()
{
    auto GameScene = dynamic_cast< class GameScene* >( GetScene() );
    if( GameScene )
    {
        // Force scene to redraw blocker FX
        GameScene->RedrawBlockers();
    }
}

bool GameModeBase::OnBlockerSelect( CardEntity *inBlocker, CardEntity *inAttacker )
{
    // Check Pointers
    if( !inBlocker || !inAttacker || inBlocker == inAttacker )
        return false;
    
    // Check Ownership
    if( inBlocker->GetOwningPlayer() != GetPlayer() || inAttacker->GetOwningPlayer() != GetOpponent() )
        return false;

    // Check Stamina
    if( inBlocker->Stamina <= 0 )
        return false;

    // Ensure Attacker
    if( !inAttacker->bAttacking )
        return false;

    // Call 'CanBeBlock' and 'CanBlock' hooks
    // TODO: Should move to Authority? Or different system for checking this?
    
    // Looks good, store this choice
    BlockMatrix[ inBlocker ] = inAttacker;
    RedrawBlockers();

    return true;
}

bool GameModeBase::OnCardDragDrop( CardEntity *inCard, cocos2d::Touch *Info )
{
    // Determine if the player attempted to drop the card on the field
    // TODO: Maybe handle spells different, since they arent static, and dont need to stay on
    // the field, maybe just display something on the screen and query any user input without placing the
    // card in the battlefield
    
    if( !inCard || !Info )
        return false;
    
    // In the future, we can alter this method to allow dropping cards onto other entities
    // besides just the field & hand
    
    auto DropPos = Info->getLocation(); // Were not going to factor the offset when dropping, use actual touch pos
    auto pl = GetPlayer();
    auto hand = pl ? pl->GetHand() : nullptr;
    auto field = pl ? pl->GetField() : nullptr;
    
    // First, attempt to drop into different hand position
    if( hand && hand->AttemptDrop( inCard, DropPos ) )
        return true;
    
    for( auto It = hand->Begin(); It != hand->End(); It++ )
    {
        
    }
    
    for( auto It = field->Begin(); It != field->End(); It++ )
    {
        
    }
    
    // If not, then try the field
    if( field )
    {
        int BestIndex = field->AttemptDrop( inCard, DropPos );
        if( BestIndex < 0 )
            return false;
        
        // Player attempted to drop card onto field, so we need to pass call along
        // to the authority to be processed
        auto Auth = GetAuthority< AuthorityBase >();
        CC_ASSERT( Auth );
        
        Auth->PlayCard( inCard->GetEntityId(), BestIndex );
    }
    
    return false;
}


void GameModeBase::OnCardClicked( CardEntity* inCard )
{
    auto LocalPlayer = GetPlayer();
    bool bLocalOwner = inCard->GetOwningPlayer() == LocalPlayer;
    
    if( inCard->InGrave() )
    {
        CloseCardViewer();
        CloseHandViewer();
        OpenGraveyardViewer( inCard->GetOwningPlayer()->GetGraveyard() );
        
        return;
    }
    else if( inCard->OnField() )
    {
        // Check if were in attack phase
        if( State.tState == TurnState::Attack )
        {
            if( inCard && inCard->bAttacking )
            {
                inCard->bAttacking = false;
                inCard->ClearOverlay();
            }
            else if( inCard && !inCard->bAttacking )
            {
                // Check if the card is able to attack
                if( CanCardAttack( inCard ) )
                {
                    inCard->bAttacking = true;
                    inCard->SetOverlay( "icon_attack.png", 180 );
                }
            }
        }
        else if( State.tState == TurnState::Block && State.pState == PlayerTurn::Opponent )
        {
            // If we click on a card thats blocking then we will stop blocking
            if( BlockMatrix.count( inCard ) > 0 )
            {
                BlockMatrix.erase( inCard );
                RedrawBlockers();
            }
        }
        else
        {
            CloseGraveyardViewer();
            CloseHandViewer();
            OpenCardViewer( inCard );
        }
        
        return;
    }
    else if( inCard->InHand() && bLocalOwner )
    {
        CloseGraveyardViewer();
        CloseCardViewer();
        OpenHandViewer( inCard );
        
        return;
    }
    
    CloseGraveyardViewer();
    CloseCardViewer();
    CloseHandViewer();
}


void GameModeBase::CloseCardViewer()
{
    // We need to force the hand and field to invalidate, which will
    // move the card back to where it should be
    _DoCloseViewer();
    _viewCard = nullptr;
}


void GameModeBase::CloseGraveyardViewer()
{
    if( _graveViewer )
    {
        _graveViewer->removeFromParent();
        _graveViewer = nullptr;
    }
}


void GameModeBase::CloseHandViewer()
{
    auto pl = GetPlayer();
    auto hand = pl ? pl->GetHand() : nullptr;
    
    if( hand && hand->IsExpanded() )
        hand->SetExpanded( false );
}


void GameModeBase::OpenCardViewer( CardEntity *inCard )
{
    if( !inCard || inCard == _viewCard )
        return;
    
    _viewCard = inCard;
    auto scene = GetScene();
    
    if( !scene )
        return;
    
    // For now, were just going to create a new UI menu, that fades in containing
    // the full sized card image, instead of trying to perform a sprite animation
    // TODO: Better looking animation
    
    // Determine if the card is in the hand or not
    bool bCanPlay = CanPlayCard( inCard );
    
    _DoCloseViewer();
    _Viewer = CardViewer::create( inCard, bCanPlay );
    _Viewer->SetCloseCallback( std::bind( &GameModeBase::_DoCloseViewer, this ) );
    if( bCanPlay )
        _Viewer->SetPlayCallback( std::bind( &GameModeBase::PlayCard, this, inCard, -1 ) );
    
    scene->addChild( _Viewer, 200 );
}


void GameModeBase::_DoCloseViewer()
{
    if( _Viewer )
    {
        _Viewer->removeFromParent();
        _Viewer = nullptr;
    }
}


void GameModeBase::OpenHandViewer( CardEntity *inCard )
{
    auto Player = GetPlayer();
    if( Player == inCard->GetOwningPlayer() )
    {
        auto hand = Player->GetHand();
        
        // If the hand isnt expanded, then exapnd it
        if( !hand->IsExpanded() )
        {
            hand->SetExpanded( true );
        }
        else
        {
            // Fully open card viewer for this card
            OpenCardViewer( inCard );
        }
    }
}


void GameModeBase::OpenGraveyardViewer( GraveyardEntity *Grave )
{
    if( _graveViewer )
    {
        _graveViewer->removeFromParent();
        _graveViewer = nullptr;
    }
    
    auto Scene = GetScene();
    
    if( !Scene )
        return;
    
    _graveViewer = CardSelector::Create( Grave->Begin(), Grave->End() );
    _graveViewer->Lock();
    _graveViewer->SetConfirmLabel( "Close" );
    _graveViewer->SetConfirm( [ = ]()
    {
        CloseGraveyardViewer();
    } );
    
    Scene->addChild( _graveViewer, 199 );
}


void GameModeBase::OnCardSwipedUp( CardEntity *inCard )
{
    if( !inCard )
        return;
    
    if( State.pState == PlayerTurn::LocalPlayer && State.tState == TurnState::Attack )
    {
        if( inCard && inCard->bAttacking )
        {
            inCard->bAttacking = false;
            inCard->ClearOverlay();
        }
        else if( inCard && !inCard->bAttacking )
        {
            // Check if the card is able to attack
            if( CanCardAttack( inCard ) )
            {
                inCard->bAttacking = true;
                inCard->SetOverlay( "icon_attack.png", 180 );
            }
        }
    }
    else if( State.pState == PlayerTurn::Opponent && State.tState == TurnState::Block )
    {
        
    }
}


void GameModeBase::OnActionQueue()
{
    CloseGraveyardViewer();
    CloseHandViewer();
    CloseCardViewer();
}


void GameModeBase::EnableSelection()
{
    _bSelectionEnabled = true;
}

void GameModeBase::DisableSelection()
{
    _bSelectionEnabled = false;
    
    if( _bDrag && _touchedCard )
        _touchedCard->SetIsDragging( false );
}

/*==========================================================================================================
    GameModeBase -> End of Input Logic
 ==========================================================================================================*/

void GameModeBase::UpdateMatchState( MatchState In )
{
    State.mState = In;
    
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
    State.tState = In;
    
    // Update Label
    if( State.mState == MatchState::Main )
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
    
    TurnFinished = false;
}

void GameModeBase::UpdatePlayerTurn( PlayerTurn In )
{
    State.pState = In;
    
    // Update Label
    if( State.mState == MatchState::Main )
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
    
    OpponentDeck->SetPosition( cocos2d::Vec2( Size.width * -0.45f, -Size.height * 0.112f ) );
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
    
    PlayerGrave->SetPosition( cocos2d::Vec2( Size.width * -0.45f, Size.height * 0.35f ) );
    PlayerGrave->SetRotation( 0.f );
    OpponentGrave->SetPosition( cocos2d::Vec2( -Size.width * -0.45f, -Size.height * 0.35f ) );
    OpponentGrave->SetRotation( 180.f );
    
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

void GameModeBase::AddAction( const std::string& In, std::function< void( Action*, std::function< void() > ) > Handler )
{
    ActionHandlers[ In ] = Handler;
}

void GameModeBase::RunAction( Action& Target, std::function< void() > Callback )
{
    // Check if a callback exists for this action name
    if( ActionHandlers.count( Target.Name ) <= 0 )
    {
        cocos2d::log( "[GM] No action handler bound to '%s'", Target.Name.c_str() );
        Callback();
        return;
    }
    
    auto& Func = ActionHandlers[ Target.Name ];
    if( Func )
    {
        Func( &Target, Callback );
    }
}

void GameModeBase::PopQueue( ActionQueue& Target )
{
    if( Target.Actions.empty() || Target.Position >= Target.Actions.size() )
    {
        // Run queue callback
        if( Target.Callback )
            Target.Callback();
        
        // Delete Action Queue
        if( ActiveQueues.count( Target.Identifier ) > 0 )
        {
            ActiveQueues.erase( Target.Identifier );
        }
        else
        {
            cocos2d::log( "[GM] Failed to delete action queue on completion!" );
        }
        
        // Check for possible actions
        if( ActiveQueues.empty() )
        {
            _bCheckPossibleActions = true;
            EnableSelection();
        }
        
        return;
    }
    
    if( Target.Actions[ Target.Position ]->Name == "Parallel" )
    {
        // Parallel Actions!
        ParallelAction* Parallel = dynamic_cast< ParallelAction* >( Target.Actions[ Target.Position ].get() );
        if( !Parallel )
        {
            cocos2d::log( "[GM] Invalid parallel action in queue!" );
            
            Target.Position++;
            PopQueue( Target );
            return;
        }
        
        for( auto It = Parallel->Actions.begin(); It != Parallel->Actions.end(); It++ )
        {
            if( *It )
            {
                RunAction( **It, [ =, &Target ]()
                {
                    Parallel->Counter++;
                    
                    if( Parallel->Counter >= Parallel->Actions.size() )
                    {
                        // Parallel action complete!
                        // Pop and continue to next action
                        Target.Position++;
                        PopQueue( Target );
                    }
                } );
            }
        }
    }
    else
    {
        RunAction( *Target.Actions[ Target.Position ], [ & ]()
        {
            // Pop and continue to next action
            Target.Position++;
            PopQueue( Target );
        } );
    }
}

void GameModeBase::RunActionQueue( ActionQueue&& In )
{
    if( State.mState == MatchState::PostMatch )
        return;
    
    if( In.Actions.empty() )
    {
        cocos2d::log( "[GM] Warning: Attempt to run empty action queue!" );
        if( In.Callback )
            In.Callback();
        
        return;
    }
    
    // Check for duplicate identifier
    if( ActiveQueues.count( In.Identifier ) > 0 )
    {
        cocos2d::log( "[GM] Warning: Attempt to run duplicate action queue" );
        if( In.Callback )
            In.Callback();
        
        return;
    }
    
    // Create new entry
    auto Entry = ActiveQueues.insert( std::make_pair( In.Identifier, std::move( In ) ) );
    
    // Check for failure
    if( !Entry.second )
    {
        cocos2d::log( "[GM] Warning: Failed to add new action queue to list!" );
        if( In.Callback )
            In.Callback();
        
        return;
    }
    
    // Disable Input
    DisableSelection();
    
    // Start popping actions
    PopQueue( Entry.first->second );
    
}

bool GameModeBase::CanPlayCard( CardEntity* In )
{
    // Dont let player play any cards while actions are in progress
    if( ActiveQueues.size() > 0 )
    {
        cocos2d::log( "Cant play card.. active queues!" );
        return false;
    }
    else
        return CouldPlayCard( In );
}

// We needed a seperate function, that doesnt check if action queues are in progress
bool GameModeBase::CouldPlayCard( CardEntity* In )
{
    if( !In || !In->InHand() )
        return false;
    
    auto LocalPlayer = State.GetPlayer();
    
    if( !LocalPlayer || State.mState != MatchState::Main ||
       State.tState != TurnState::Marshal || State.pState != PlayerTurn::LocalPlayer )
        return false;
    
    return In->GetOwningPlayer() == LocalPlayer && In->ManaCost <= LocalPlayer->Mana;
}

bool GameModeBase::CanCardAttack( CardEntity *In )
{
    if( !In || !In->OnField() )
        return false;

    auto LocalPlayer = State.GetPlayer();
    if( !LocalPlayer )
        return false;

    if( State.mState != MatchState::Main || State.tState != TurnState::Attack ||
       State.pState != PlayerTurn::LocalPlayer )
        return false;

    if( In->Stamina <= 0 )
        return false;

    if( In->GetOwningPlayer() != LocalPlayer )
        return false;
    
    return true;
}

bool GameModeBase::CanCardBlock( CardEntity *In, CardEntity *Target )
{
    if( !In || !In->OnField() )
        return false;
    
    auto LocalPlayer = State.GetPlayer();
    if( !LocalPlayer )
        return false;
    
    if( State.mState != MatchState::Main || State.tState != TurnState::Block ||
       State.pState != PlayerTurn::Opponent )
        return false;
    
    if( In->Stamina <= 0 )
        return false;
    
    if( In->GetOwningPlayer() != LocalPlayer )
        return false;
    
    return true;
}

bool GameModeBase::CanTriggerAbility( CardEntity* In )
{
    // Check if any abilities can be activated
    if( !In )
        return false;
    
    auto Player = GetPlayer();
    if( !Player )
        return false;
    
    auto Info = In->GetInfo();
    if( Info )
    {
        for( auto It = Info->Abilities.begin(); It != Info->Abilities.end(); It++ )
        {
            auto& Ability = It->second;
            if( Ability.ManaCost > Player->GetMana() || Ability.StaminaCost > In->Stamina )
                continue;
            
            if( Ability.CheckFunc && Ability.CheckFunc->isFunction() )
            {
                if( !( *Ability.CheckFunc )( std::addressof( State ), In ) )
                    continue;
            }
            
            return true;
        }
    }
    
    return false;
}

bool GameModeBase::CanTriggerAbility( CardEntity* In, uint8_t AbilityId )
{
    if( !In )
        return false;
    
    auto Player = In->GetOwningPlayer();
    auto Info = In->GetInfo();
    
    if( !Player || !Info )
        return false;
    
    if( Info->Abilities.count( AbilityId ) <= 0 )
        return false;
    
    auto& Ability = Info->Abilities[ AbilityId ];
    if( Ability.ManaCost > Player->GetMana() || Ability.StaminaCost > In->Stamina )
        return false;
    
    if( Ability.CheckFunc && Ability.CheckFunc->isFunction() )
    {
        if( !( *Ability.CheckFunc )( std::addressof( State ), In ) )
            return false;
    }
    
    return true;
}

bool GameModeBase::TriggerAbility( CardEntity *Target, uint8_t AbilityId )
{
    if( !Target )
        return false;
    
    if( !CanTriggerAbility( Target, AbilityId ) )
    {
        cocos2d::log( "[GM] Failed to trigger ability.. chcek failed!" );
        return false;
    }
    
    // Looks good to trigger
    auto Auth = GetAuthority< AuthorityBase >();
    if( !Auth )
        return false;
    
    CloseCardViewer();
    
    Auth->TriggerAbility( Target->GetEntityId(), AbilityId );
    return true;
}


bool GameModeBase::PlayCard( CardEntity* In, int Index )
{
    if( !CanPlayCard( In ) )
        return false;
    
    auto Auth = GetAuthority< AuthorityBase >();
    if( !Auth )
        return false;
    
    Auth->PlayCard( In->GetEntityId(), Index );
    return true;
}


void GameModeBase::FinishTurn()
{
    auto Auth = GetAuthority< AuthorityBase >();
    auto Scene = dynamic_cast< GameScene* >( GetScene() );
    CC_ASSERT( Auth && Scene );
    
    cocos2d::log( "[GM] Finishing Turn" );
    
    // If were in the attack or block phase, we need to send card list
    if( State.mState == MatchState::Main )
    {
        if( State.pState == PlayerTurn::LocalPlayer )
        {
            if( State.tState == TurnState::Attack )
            {
                std::vector< uint32_t > AttackCards;
                auto pl = GetPlayer();
                auto field = pl ? pl->GetField() : nullptr;
                
                if( field )
                {
                    for( auto It = field->Begin(); It != field->End(); It++ )
                    {
                        if( *It )
                        {
                            if( (*It)->bAttacking )
                                AttackCards.push_back( (*It)->GetEntityId() );
                        }
                    }
                }
                
                TurnFinished = true;
                
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
            else if( State.tState == TurnState::Marshal )
            {
                Auth->FinishTurn();
            }
        }
        else if( State.pState == PlayerTurn::Opponent )
        {
            if( State.tState == TurnState::Block )
            {
                TurnFinished = true;
                
                if( BlockMatrix.size() <= 0 )
                {
                    Auth->FinishTurn();
                }
                else
                {
                    std::map< uint32_t, uint32_t > FinalMatrix;
                    for( auto It = BlockMatrix.begin(); It != BlockMatrix.end(); It++ )
                    {
                        if( It->first && It->second )
                            FinalMatrix.insert( std::make_pair( It->first->GetEntityId(), It->second->GetEntityId() ) );
                    }
                    
                    Auth->SetBlockers( FinalMatrix );
                }
            }
        }
    }
    
    _bCheckPossibleActions = true;
}


void GameModeBase::Tick( float Delta )
{
    if( _bCheckPossibleActions )
    {
        _bCheckPossibleActions = false;
        std::vector< CardEntity* > ActionableCards;
        std::vector< CardEntity* > AbilityCards;
        bool bPlayersMove = false;
        auto Pl = GetPlayer();
        auto Hand = Pl ? Pl->GetHand() : nullptr;
        auto Field = Pl ? Pl->GetField() : nullptr;
        
        if( State.mState == MatchState::Main )
        {
            if( State.pState == PlayerTurn::LocalPlayer )
            {
                if( State.tState == TurnState::Marshal )
                {
                    // Check if any cards in hand can be played
                    if( Hand )
                    {
                        for( auto It = Hand->Begin(); It != Hand->End(); It++ )
                        {
                            if( *It && CanTriggerAbility( *It ) && !TurnFinished )
                            {
                                AbilityCards.push_back( *It );
                            }
                            else if( *It && CouldPlayCard( *It ) && !TurnFinished )
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
                            if( *It && CanTriggerAbility( *It ) && !TurnFinished )
                            {
                                AbilityCards.push_back( *It );
                            }
                        }
                    }
                    
                    bPlayersMove = true;
                }
                else if( State.tState == TurnState::Attack )
                {
                    // Check if the player can attack with any cards
                    if( Field )
                    {
                        for( auto It = Field->Begin(); It != Field->End(); It++ )
                        {
                            if( *It && CanCardAttack( *It ) && !TurnFinished )
                            {
                                ActionableCards.push_back( *It );
                            }
                            else if( *It && CanTriggerAbility( *It ) && !TurnFinished )
                            {
                                AbilityCards.push_back( *It );
                            }
                            
                        }
                    }
                    
                    // Check if any abilities can be played in-hand
                    if( Hand )
                    {
                        for( auto It = Hand->Begin(); It != Hand->End(); It++ )
                        {
                            if( *It && CanTriggerAbility( *It ) && !TurnFinished )
                            {
                                AbilityCards.push_back( *It );
                            }
                        }
                    }
                    
                    bPlayersMove = true;
                }
            }
            else if( State.pState == PlayerTurn::Opponent )
            {
                if( State.tState == TurnState::Block )
                {
                    // Check if the player can block with any cards
                    if( Field )
                    {
                        for( auto It = Field->Begin(); It != Field->End(); It++ )
                        {
                            if( *It && CanCardBlock( *It ) && !TurnFinished )
                            {
                                ActionableCards.push_back( *It );
                            }
                            else if( *It && CanTriggerAbility( *It ) && !TurnFinished )
                            {
                                AbilityCards.push_back( *It );
                            }
                            
                        }
                    }
                    
                    if( Hand )
                    {
                        for( auto It = Hand->Begin(); It != Hand->End(); It++ )
                        {
                            if( *It && CanTriggerAbility( *It ) && !TurnFinished )
                            {
                                AbilityCards.push_back( *It );
                            }
                        }
                    }
                    
                    bPlayersMove = true;
                }
            }
        }
        
        // If the game is waiting on the player, check if they have any possible
        // moves they can make, highlight them, or if not, advance round state
        if( bPlayersMove && !TurnFinished )
        {
            if( ActionableCards.empty() && AbilityCards.empty() )
            {
                TurnFinished = true;
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
                if( State.tState == TurnState::PostTurn || State.tState == TurnState::PreTurn )
                {
                    (*It)->ClearOverlay();
                    (*It)->bAttacking = false;
                }
                
                // Allow highlight to stay if were attacking, it will still get cleared
                // when TurnState reaches Post/PreTurn
                if( !(*It)->bAttacking )
                {
                    (*It)->ClearHighlight();
                }
            }
        }
        
        if( !ActionableCards.empty() )
        {
            auto highlightColor = cocos2d::Color3B();
            if( State.tState == TurnState::Marshal )
                highlightColor = cocos2d::Color3B( 30, 50, 240 );
            else if( State.tState == TurnState::Attack )
                highlightColor = cocos2d::Color3B( 240, 30, 30 );
            else if( State.tState == TurnState::Block )
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
        
        if( !AbilityCards.empty() )
        {
            for( auto It = AbilityCards.begin(); It != AbilityCards.end(); It++ )
            {
                if( *It )
                {
                    (*It)->SetHighlight( cocos2d::Color3B( 250, 250, 45 ), 180 );
                }
            }
        }
    }
}

void GameModeBase::OnCardDied( CardEntity *Target )
{
    if( !Target )
        return;
    
    auto Container = Target->GetContainer();
    if( Container )
        Container->Remove( Target );
    
    auto Owner = Target->GetOwningPlayer();
    auto Grave = Owner ? Owner->GetGraveyard() : nullptr;
    
    Target->DestroyOverlays();
    
    if( Grave )
    {
        Grave->AddToTop( Target );
    }
    else
    {
        Target->SetPosition( cocos2d::Vec2( 0.f, -1000.f ) );
        Target->Invalidate();
    }
}

void GameModeBase::OnCardPlay( Action* In, std::function< void() > Callback )
{
    PlayCardAction* PlayAction = dynamic_cast< PlayCardAction* >( In );
    if( !PlayAction )
    {
        cocos2d::log( "[GM] Invalid Action! Attempt to run 'PlayCardAction' with invalid action" );
        FinishAction( Callback, 0.2f );
        return;
    }
    
    // Move card onto field
    // First, we need to lookup card by entity id
    auto& Ent = IEntityManager::GetInstance();
    auto* Card = Ent.GetEntity< CardEntity >( PlayAction->TargetCard );
    auto* Owner = Ent.GetEntity< Player >( PlayAction->TargetPlayer );
    
    if( !Card || !Owner || Card->GetOwningPlayer() != Owner || !Card->InHand() )
    {
        cocos2d::log( "[GM] Invalid play card action! Entity numbers were not correct!" );
        FinishAction( Callback, 0.2f );
        return;
    }
    
    // Now were going to actually move the card to the field
    auto Container = Card->GetContainer();
    
    if( PlayAction->bWasSuccessful )
    {
        if( PlayAction->bNeedsMove )
        {
            if( Container )
                Container->Remove( Card );
            
            auto Index = PlayAction->TargetIndex;
            auto Field = Owner->GetField();
            
            if( Field )
            {
                if( Index > Field->Count() )
                    Field->AddToTop( Card, true, Callback );
                else if( Index < 0 )
                    Field->AddToBottom( Card, true, Callback );
                else
                    Field->AddAtIndex( Card, Index, true, Callback );
                
                return;
            }
        }
    }
    else
    {
        if( Container )
        {
            Container->InvalidateCards();
        }
    }
    
    FinishAction( Callback, 0.2f );
}

void GameModeBase::OnManaUpdate( Action *In, std::function<void ()> Callback )
{
    UpdateManaAction* Update = dynamic_cast< UpdateManaAction* >( In );
    if( !Update )
    {
        cocos2d::log( "[GM] Invalid Action! Couldnt cast to UpdateManaAction!" );
        FinishAction( Callback, 0.2f );
        return;
    }
    
    // Find targeted player
    auto Target = State.FindPlayer( Update->TargetPlayer );
    if( !Target )
    {
        cocos2d::log( "[GM] Invalid Mana Update! Target Id Invalid!" );
        FinishAction( Callback, 0.2f );
        return;
    }
    
    Target->SetMana( Update->Amount );
    FinishAction( Callback, 0.2f );
}

void GameModeBase::OnCardDraw( Action *In, std::function< void () > Callback )
{
    DrawCardAction* Draw = dynamic_cast< DrawCardAction* >( In );
    if( !Draw )
    {
        cocos2d::log( "[GM] Invalid Action! Couldnt cast to DrawCardAction" );
        FinishAction( Callback, 0.2f );
        return;
    }
    
    auto& Ent = IEntityManager::GetInstance();
    auto Target = State.FindPlayer( Draw->TargetPlayer );
    auto Card = Ent.GetEntity< CardEntity >( Draw->TargetCard );
    
    if( !Target || !Card || Card->GetOwningPlayer() != Target || !Card->InDeck() )
    {
        cocos2d::log( "[GM] Invalid Draw Action! Entity Ids Invalid!" );
        FinishAction( Callback, 0.2f );
        return;
    }
    
    // Move Card Into Hand
    auto Container = Card->GetContainer();
    auto Hand = Target->GetHand();
    
    if( Container )
        Container->Remove( Card );
    
    Card->CreateOverlays();
    
    if( Hand )
        Hand->AddToBottom( Card, true, Callback );
    else
        FinishAction( Callback, 0.2f );
}

void GameModeBase::OnCoinFlip( Action* In, std::function< void() > Callback )
{
    CoinFlipAction* CoinFlip = dynamic_cast< CoinFlipAction* >( In );
    if( !CoinFlip )
    {
        cocos2d::log( "[GM] Invalid Coin Flip Action! Cast Failed!" );
        FinishAction( Callback, 0.2f );
        return;
    }
    
    auto LocalPlayer = State.GetPlayer();
    if( LocalPlayer && LocalPlayer->GetEntityId() == CoinFlip->Player )
        UpdatePlayerTurn( PlayerTurn::LocalPlayer );
    else
        UpdatePlayerTurn( PlayerTurn::Opponent );
    
    FinishAction( Callback, 0.2f );
}

void GameModeBase::OnBlitzStart( Action* In, std::function< void() > Callback )
{
    cocos2d::log( "[GM] Blitz Start" );
    UpdateMatchState( MatchState::Blitz );
    FinishAction( Callback, 0.2f );
}

void GameModeBase::OnBlitzQuery( Action* In, std::function< void() > Callback )
{
    TimedQueryAction* Query = dynamic_cast< TimedQueryAction* >( In );
    if( !Query )
    {
        cocos2d::log( "[GM] Invalid Blitz Query! Cast Failed!" );
        FinishAction( Callback, 0.2f );
        return;
    }
    
    auto Player = State.GetPlayer();
    auto Hand = Player ? Player->GetHand() : nullptr;
    
    if( !Hand )
    {
        cocos2d::log( "[GM] Failed to start blitz query! Couldnt get local players hand!" );
        FinishAction( Callback, 0.2f );
        return;
    }
    
    Hand->OpenBlitzMode();
    FinishAction( Callback, 0.2f );
}

void GameModeBase::OnBlitzError( Action* In, std::function< void() > Callback )
{
    CardErrorAction* Err = dynamic_cast< CardErrorAction* >( In );
    if( !Err )
    {
        cocos2d::log( "[GM] Invalid Blitz Error! Cast Failed" );
        FinishAction( Callback, 0.2f );
        return;
    }
    
    auto LocalPlayer = State.GetPlayer();
    auto Hand = LocalPlayer ? LocalPlayer->GetHand() : nullptr;
    
    if( !Hand )
    {
        cocos2d::log( "[GM] Invalid Blitz Error! Couldnt get hand!" );
        FinishAction( Callback, 0.2f );
        return;
    }
    
    for( auto i = Err->Errors.begin(); i != Err->Errors.end(); i++ )
    {
        cocos2d::log( "[GM] Failed to select card '%d' for blitz.. error code %d", i->first, i->second );
        
        for( auto j = Hand->Begin(); j != Hand->End(); j++ )
        {
            if( *j && (*j)->GetEntityId() == i->first )
            {
                Hand->DeselectBlitz( *j );
                break;
            }
        }
    }
    
    FinishAction( Callback, 0.2f );
}

void GameModeBase::OnBlitzSuccess( Action *In, std::function<void ()> Callback )
{
    // Check which player finished selecting blitz cards
    PlayerEventAction* Event = dynamic_cast< PlayerEventAction* >( In );
    if( !Event )
    {
        cocos2d::log( "[GM] Invalid Blitz Success! Cast Failed!" );
        FinishAction( Callback, 0.2f );
        return;
    }
    
    auto Target = State.FindPlayer( Event->Player );
    if( !Target )
    {
        cocos2d::log( "[GM] Invalid Blitz Success! Invalid Target!" );
        FinishAction( Callback, 0.2f );
        return;
    }
    
    if( Target == GetPlayer() )
    {
        // Close Blitz Menu
        auto Hand = Target->GetHand();
        if( !Hand )
        {
            cocos2d::log( "[GM] Blitz Success Failed! Couldnt get local player hand" );
            FinishAction( Callback, 0.2f );
            return;
        }
        
        Hand->CloseBlitzMode();
    }
    else if( Target == GetOpponent() )
    {
        // TODO: Update UI
    }
    
    FinishAction( Callback, 0.2f );
}

void GameModeBase::OnAttackError( Action* In, std::function< void() > Callback )
{
    CardErrorAction* Err = dynamic_cast< CardErrorAction* >( In ); 
    if( !Err )
    {
        cocos2d::log( "[GM] Invalid Attack Error! Cast Failed!" );
        FinishAction( Callback, 0.2f );
        return;
    }
    
    auto& Ent = IEntityManager::GetInstance();
    
    for( auto It = Err->Errors.begin(); It != Err->Errors.end(); It++ )
    {
        auto Card = Ent.GetEntity< CardEntity >( It->first );
        if( Card )
        {
            // Deselect the card
            Card->bAttacking = false;
            Card->ClearOverlay();
            Card->ClearHighlight();
        }
        
        cocos2d::log( "[GM] Failed to attack with '%d' with error code %d", It->first, It->second );
    }
    
    FinishAction( Callback, 0.2f );
}

void GameModeBase::OnMatchStart( Action* In, std::function< void() > Callback )
{
    cocos2d::log( "[GM] Match Starting" );
    
    UpdateMatchState( MatchState::Main );
    auto LocalPlayer = State.GetPlayer();
    auto Hand = LocalPlayer ? LocalPlayer->GetHand() : nullptr;
    
    if( Hand )
        Hand->CloseBlitzMode();
    
    FinishAction( Callback, 0.2f );
}

void GameModeBase::OnTurnStart( Action* In, std::function< void() > Callback )
{
    TurnStartAction* TurnStart = dynamic_cast< TurnStartAction* >( In );
    if( !TurnStart )
    {
        cocos2d::log( "[GM] Invalid Turn Start! Cast Failed!" );
        FinishAction( Callback, 0.2f );
        return;
    }
    
    UpdateMatchState( MatchState::Main );
    UpdateTurnState( TurnState::PreTurn );
    
    auto LocalPlayer = State.GetPlayer();
    if( LocalPlayer && LocalPlayer->GetEntityId() == TurnStart->Player )
    {
        UpdatePlayerTurn( PlayerTurn::LocalPlayer );
    }
    else
    {
        UpdatePlayerTurn( PlayerTurn::Opponent );
    }
    
    FinishAction( Callback, 0.2f );
}

void GameModeBase::OnMarshalStart( Action* In, std::function< void() > Callback )
{
    cocos2d::log( "[GM] Marshal Started" );
    
    UpdateMatchState( MatchState::Main );
    UpdateTurnState( TurnState::Marshal );
    
    EnableSelection();
    
    GameScene* Scene = dynamic_cast< GameScene* >( GetScene() );
    if( State.pState == PlayerTurn::LocalPlayer && Scene )
        Scene->ShowFinishButton();
    else
        Scene->HideFinishButton();
    
    FinishAction( Callback, 0.2f );
}

void GameModeBase::OnAttackStart( Action* In, std::function< void() > Callback )
{
    cocos2d::log( "[GM] Attacking Started" );
    
    UpdateMatchState( MatchState::Main );
    UpdateTurnState( TurnState::Attack );
    
    EnableSelection();
    
    GameScene* Scene = dynamic_cast< GameScene* >( GetScene() );
    if( State.pState == PlayerTurn::LocalPlayer && Scene )
        Scene->ShowFinishButton();
    else
        Scene->HideFinishButton();
    
    FinishAction( Callback, 0.2f );
}

void GameModeBase::OnBlockStart( Action* In, std::function< void() > Callback )
{
    cocos2d::log( "[GM] Block Started" );
    
    UpdateMatchState( MatchState::Main );
    UpdateTurnState( TurnState::Block );
    
    EnableSelection();
    
    GameScene* Scene = dynamic_cast< GameScene* >( GetScene() );
    if( State.pState == PlayerTurn::Opponent && Scene )
        Scene->ShowFinishButton();
    else
        Scene->HideFinishButton();
    
    FinishAction( Callback, 0.2f );
}

void GameModeBase::OnTurnFinish( Action* In, std::function< void() > Callback )
{
    cocos2d::log( "[GM] Post Turn Started!" );
    
    UpdateMatchState( MatchState::Main );
    UpdateTurnState( TurnState::PostTurn );
    
    GameScene* Scene = dynamic_cast< GameScene* >( GetScene() );
    if( Scene )
        Scene->HideFinishButton();
    
    FinishAction( Callback, 0.2f );
}

void GameModeBase::OnDamageStart( Action* In, std::function< void() > Callback )
{
    cocos2d::log( "[GM] Damage Started!" );
    
    UpdateMatchState( MatchState::Main );
    UpdateTurnState( TurnState::Damage );
    
    GameScene* Scene = dynamic_cast< GameScene* >( GetScene() );
    if( Scene )
        Scene->HideFinishButton();
    
    auto LocalPlayer = State.GetPlayer();
    auto Opponent = State.GetOpponent();
    auto PlayerField = LocalPlayer ? LocalPlayer->GetField() : nullptr;
    auto OpponentField = Opponent ? Opponent->GetField() : nullptr;
    
    if( PlayerField )
    {
        PlayerField->PauseInvalidate();
    }
    if( OpponentField )
    {
        OpponentField->PauseInvalidate();
    }
    
    FinishAction( Callback, 0.2f );
}

void GameModeBase::OnDamage( Action *In, std::function<void ()> Callback )
{
    DamageAction* Damage = dynamic_cast< DamageAction* >( In );
    if( !Damage )
    {
        cocos2d::log( "[GM] Invalid Card Damage Action! Cast Failed!" );
        FinishAction( Callback, 0.2f );
        return;
    }
    
    bool IsDirty    = false;
    auto Target     = State.FindPlayer( Damage->Target );
    
    if( Target )
    {
        // Damage Player
        Target->SetHealth( Damage->UpdatedPower );
    }
    else
    {
        auto Card = State.FindCard( Damage->Target, nullptr, CardPos::FIELD );
        if( !Card )
        {
            cocos2d::log( "[GM] Invalid Card Damage Action! Couldnt find specified player" );
            FinishAction( Callback, 0.2f );
            return;
        }
        
        Card->UpdatePower( Damage->UpdatedPower );
        Card->ClearOverlay();
        Card->ClearHighlight();
        
        if( Card->Power <= 0 || Card->Stamina <= 0 )
        {
            // Move Card To Grave
            OnCardDied( Card );
        }
        
        if( BlockMatrix.count( Card ) > 0 )
        {
            BlockMatrix.erase( Card );
            IsDirty = true;
        }
    }
    
    auto Inflictor = State.FindCard( Damage->Inflictor, nullptr, CardPos::FIELD );
    if( Inflictor )
    {
        Inflictor->ClearOverlay();
        Inflictor->ClearHighlight();
        
        if( BlockMatrix.count( Inflictor ) > 0 )
        {
            BlockMatrix.erase( Inflictor );
            IsDirty = true;
        }
    }
    
    if( IsDirty )
        RedrawBlockers();
    
    FinishAction( Callback, 0.2f );
}

void GameModeBase::OnCombat( Action *In, std::function< void() > Callback )
{
    CombatAction* Combat = dynamic_cast< CombatAction* >( In );
    if( !Combat )
    {
        cocos2d::log( "[GM] Invalid Combat Action! Cast Failed!" );
        FinishAction( Callback, 0.2f );
        return;
    }
    
    auto Attacker = State.FindCard( Combat->Attacker, nullptr, CardPos::FIELD );
    auto Blocker = State.FindCard( Combat->Blocker, nullptr, CardPos::FIELD );
    
    if( !Attacker || !Blocker )
    {
        cocos2d::log( "[GM] Invalid Combat Action! Couldnt Find Combat Pair!" );
        FinishAction( Callback, 0.2f );
        return;
    }
    
    Attacker->UpdatePower( Combat->AttackerPower );
    Blocker->UpdatePower( Combat->BlockerPower );
    
    Attacker->ClearOverlay();
    Attacker->ClearHighlight();
    Blocker->ClearOverlay();
    Blocker->ClearHighlight();
    
    bool IsDirty = false;
    if( Attacker->Power <= 0 || Attacker->Stamina <= 0 )
    {
        // When an attacker died, erase all entries
        for( auto It = BlockMatrix.cbegin(); It != BlockMatrix.cend(); )
        {
            if( It->second == Attacker )
            {
                IsDirty = true;
                It = BlockMatrix.erase( It );
                
            }
            else
            {
                ++It;
            }
        }
        
        OnCardDied( Attacker );
    }
    
    if( Blocker->Power <= 0 || Blocker->Stamina <= 0 )
    {
        OnCardDied( Blocker );
    }
    
    if( BlockMatrix.count( Attacker ) > 0 )
    {
        BlockMatrix.erase( Attacker );
        IsDirty = true;
    }
    
    if( BlockMatrix.count( Blocker ) > 0 )
    {
        BlockMatrix.erase( Blocker );
        IsDirty = true;
    }
    
    if( IsDirty )
    {
        RedrawBlockers();
    }
    
    FinishAction( Callback, 0.2f );
}

void GameModeBase::OnStaminaUpdate( Action* In, std::function< void() > Callback )
{
    UpdateStaminaAction* Update = dynamic_cast< UpdateStaminaAction* >( In );
    if( !Update )
    {
        cocos2d::log( "[GM] Invalid Stamina Update! Cast Failed!" );
        FinishAction( Callback, 0.2f );
        return;
    }
    
    auto Target = State.FindCard( Update->Target, nullptr, CardPos::NONE );
    if( !Target )
    {
        cocos2d::log( "[GM] Invalid Stamina Update! Couldnt Find Target!" );
        FinishAction( Callback, 0.2f );
        return;
    }
    
    Target->UpdateStamina( Update->UpdatedAmount );
    
    if( Target->Stamina <= 0 || Target->Power <= 0 )
    {
        OnCardDied( Target );
    }
    
    FinishAction( Callback, 0.2f );
}

void GameModeBase::OnBoardCleanup( Action* In, std::function< void() > Callback )
{
    auto LocalPlayer = State.GetPlayer();
    auto Opponent = State.GetOpponent();
    auto PlayerField = LocalPlayer ? LocalPlayer->GetField() : nullptr;
    auto OpponentField = Opponent ? Opponent->GetField() : nullptr;
    
    if( PlayerField )
    {
        PlayerField->ResumeInvalidate();
        
        for( auto It = PlayerField->Begin(); It != PlayerField->End(); It++ )
            if( *It )
            {
                (*It)->ClearOverlay();
                (*It)->ClearHighlight();
            }
    }
    if( OpponentField )
    {
        OpponentField->ResumeInvalidate();
        
        for( auto It = OpponentField->Begin(); It != OpponentField->End(); It++ )
            if( *It )
            {
                (*It)->ClearOverlay();
                (*It)->ClearHighlight();
            }
    }
    
    FinishAction( Callback, 0.75f );
}

void GameModeBase::OnMatrixUpdate( Action *In, std::function<void ()> Callback )
{
    BattleMatrixAction* Update = dynamic_cast< BattleMatrixAction* >( In );
    if( !Update )
    {
        cocos2d::log( "[GM] Invalid 'BattleMatrix' event received! Cast Failed!" );
        return;
    }
    
    auto AttackingPlayer = State.GetActivePlayer();
    auto BlockingPlayer = State.GetInactivePlayer();
    CC_ASSERT( AttackingPlayer && BlockingPlayer );
    
    BlockMatrix.clear();
    
    for( auto It = Update->Matrix.begin(); It != Update->Matrix.end(); It++ )
    {
        // Lookup Entity
        CardEntity* Attacker = State.FindCard( It->first, AttackingPlayer, CardPos::FIELD );
        if( Attacker )
        {
            if( !Attacker->bAttacking )
            {
                Attacker->bAttacking = true;
                Attacker->SetHighlight( cocos2d::Color3B( 240, 30, 30 ) );
                Attacker->SetOverlay( "icon_attack.png", 180 );
            }
            
            // Loop through blockers
            for( auto In = It->second.begin(); In != It->second.end(); In++ )
            {
                // Lookup Entity
                CardEntity* Blocker = State.FindCard( *In, BlockingPlayer, CardPos::FIELD );
                if( Blocker )
                {
                    BlockMatrix[ Blocker ] = Attacker;
                    Blocker->SetHighlight( cocos2d::Color3B( 30, 50, 240 ) );
                }
                else
                {
                    cocos2d::log( "[GM] Failed to find blocker in battle matrix update! %d", *In );
                }
            }
        }
        else
        {
            cocos2d::log( "[GM] Failed to find attacker in battle matrix update! %d", It->first );
        }
    }
    
    RedrawBlockers();
    FinishAction( Callback, 0.3f );
}
