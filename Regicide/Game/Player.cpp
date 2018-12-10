//
//    Player.cpp
//    Regicide Mobile
//
//    Created: 11/10/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#include "Player.hpp"
#include "DeckEntity.hpp"
#include "HandEntity.hpp"
#include "FieldEntity.hpp"
#include "GraveyardEntity.hpp"
#include "Actions.hpp"
#include "World.hpp"
#include "GameModeBase.hpp"
#include "KingEntity.hpp"

using namespace Game;


Player::Player()
: EntityBase( "Player" )
{
    State.Mana          = 10;
    CardBackTexture     = "CardBack.png";
    
    auto& Ent = IEntityManager::GetInstance();
    
    Deck                = Ent.CreateEntity< DeckEntity >();
    Field               = Ent.CreateEntity< FieldEntity >();
    Hand                = Ent.CreateEntity< HandEntity >();
    Graveyard           = Ent.CreateEntity< GraveyardEntity >();
    King                = Ent.CreateEntity< KingEntity >();
    
    AddChild( Deck );
    AddChild( Field );
    AddChild( Hand );
    AddChild( Graveyard );
    AddChild( King );
    
    using namespace std::placeholders;
    
    SetActionCallback( "PlayCard", std::bind( &Player::Action_PlayCard, this, _1, _2 ) );
    SetActionCallback( "UpdateMana", std::bind( &Player::Action_UpdateMana, this, _1, _2 ) );
    SetActionCallback( "DrawCard", std::bind( &Player::Action_DrawCard, this, _1, _2 ) );
    SetActionCallback( "KingDamage", std::bind( &Player::Action_KingDamage, this, _1, _2 ) );
}


Player::~Player()
{
    Deck                = nullptr;
    Field               = nullptr;
    Hand                = nullptr;
    Graveyard           = nullptr;
}

void Player::Cleanup()
{
    EntityBase::Cleanup();
}

std::vector< CardEntity* > Player::GetAllCards()
{
    // Loop through each card container, and build a list
    std::vector< CardEntity* > Output;
    
    if( Deck )
        for( auto It = Deck->Begin(); It != Deck->End(); It++ )
            if( *It )
                Output.push_back( *It );
    
    if( Field )
        for( auto It = Field->Begin(); It != Field->End(); It++ )
            if( *It )
                Output.push_back( *It );
    
    if( Hand )
        for( auto It = Hand->Begin(); It != Hand->End(); It++ )
            if( *It )
                Output.push_back( *It );
    
    if( Graveyard )
        for( auto It = Graveyard->Begin(); It != Graveyard->End(); It++ )
            if( *It )
                Output.push_back( *It );
    
    return Output;
}

CardEntity* Player::PerformTouchTrace( const cocos2d::Vec2 &inPos )
{
    // Instead of building a vector of all cards, we can perform the
    // lookup in this class, and return the result back to the touch layer
    if( Hand )
    {
        auto ret = _Impl_TraceTouch( Hand->Begin(), Hand->End(), inPos );
        if( ret )
            return ret;
    }
    if( Field )
    {
        auto ret = _Impl_TraceTouch( Field->Begin(), Field->End(), inPos );
        if( ret )
            return ret;
    }
    if( Graveyard )
    {
        auto ret = _Impl_TraceTouch( Graveyard->Begin(), Graveyard->End(), inPos );
        if( ret )
            return ret;
    }
    if( Deck )
    {
        auto ret = _Impl_TraceTouch( Deck->Begin(), Deck->End(), inPos );
        if( ret )
            return ret;
    }
    
    return nullptr;
}

CardEntity* Player::_Impl_TraceTouch( std::deque< CardEntity* >::iterator Begin, std::deque< CardEntity* >::iterator End, const cocos2d::Vec2 &inPos )
{
    for( auto It = Begin; It != End; It++ )
    {
        if( *It && (*It)->Sprite && (*It)->Sprite->getBoundingBox().containsPoint( inPos ) )
            return *It;
    }
    
    return nullptr;
}

void Player::Action_PlayCard( Action *In, std::function<void ()> Callback )
{
    // Cast action to PlayCardAction
    auto playAction = dynamic_cast< PlayCardAction* >( In );
    if( !playAction )
    {
        cocos2d::log( "[Player] Invalid play card action! Cast failed" );
        Callback();
        return;
    }
    
    // Lookup card by entity id
    auto Card = IEntityManager::GetInstance().GetEntity< CardEntity >( playAction->TargetCard );
    if( !Card )
    {
        cocos2d::log( "[Player] Failed to play card animation! Couldnt find card by ID" );
        Callback();
        return;
    }
    
    if( playAction->bWasSuccessful )
    {
        if( playAction->bNeedsMove )
        {
            // Card was played, move onto the field
            auto Cont = Card->GetContainer();
            if( Cont )
                Cont->Remove( Card );
            
            // Determine Index
            if( playAction->TargetIndex > Field->Count() )
            {
                Field->AddToTop( Card, true, Callback );
            }
            else if( playAction->TargetIndex < 0 )
            {
                Field->AddToBottom( Card, true, Callback );
            }
            else
            {
                Field->AddAtIndex( Card, playAction->TargetIndex, true, Callback );
            }
        }
    }
    else
    {
        // Couldnt play card, so move it back to hand
        auto Cont = Card->GetContainer();
        if( Cont )
            Cont->InvalidateCards();
    }

}

void Player::Action_UpdateMana( Action* In, std::function<void ()> Callback )
{
    // Cast to UpdateManaAction
    auto* updateMana = dynamic_cast< UpdateManaAction* >( In );
    if( !updateMana )
    {
        cocos2d::log( "[Player] Invalid update mana action! Cast failed!" );
        Callback();
        return;
    }
    
    State.Mana += updateMana->Amount;
    
    auto King = GetKing();
    if( King )
        King->UpdateMana( State.Mana );
    
    Callback();
    
}

void Player::Action_DrawCard( Action* In, std::function< void() > Callback )
{
    // Cast to DrawCardAction
    auto* drawCard = dynamic_cast< DrawCardAction* >( In );
    if( !drawCard )
    {
        cocos2d::log( "[Player] Invalid draw card action! Cast Failed!" );
        Callback();
        return;
    }
    
    // Find card by entity ID
    auto Card = IEntityManager::GetInstance().GetEntity< CardEntity >( drawCard->TargetCard );
    if( !Card )
    {
        cocos2d::log( "[Player] Failed to run DrawCardAction because the target card was not found!" );
        
        FinishAction( Callback, 0.25f );
        return;
    }
    
    Card->CreateOverlays();
    
    // Move card into hand
    auto Cont = Card->GetContainer();
    if( Cont )
        Cont->Remove( Card );
    
    if( bOpponent )
        Hand->AddToTop( Card, true, Callback );
    else
        Hand->AddToBottom( Card, true, Callback );
}

void Player::Action_KingDamage( Action* In, std::function< void() > Callback )
{
    auto damage = dynamic_cast< DamageAction* >( In );
    if( !damage )
    {
        cocos2d::log( "[Player] Received king damage action that was invalid!" );
        
        FinishAction( Callback, 0.25f );
        return;
    }
    
    if( damage->Damage <= 0 )
    {
        cocos2d::log( "[Player] Received king damage action with invalid damage amount" );
        
        FinishAction( Callback, 0.25f );
        return;
    }
    
    // Perform Damage
    State.Health -= damage->Damage;
    cocos2d::log( "[Player] %d damage dealt to king", damage->Damage );
    
    auto king = GetKing();
    if( king )
        king->UpdateHealth( State.Health );
    
    // TODO: Animation!
    
    // If we died, the Authority will detect it and send the needed actions
    FinishAction( Callback, 0.5f );
}


void Player::SetMana( int In )
{
    State.Mana = In;
    
    if( King )
        King->UpdateMana( In );
}

void Player::SetHealth( int In )
{
    State.Health = In;
    
    if( King )
        King->UpdateHealth( In );
}


