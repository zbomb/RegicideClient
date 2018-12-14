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
    Mana                = 10;
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


void Player::SetMana( int In )
{
    Mana = In;
    
    if( King )
        King->UpdateMana( In );
}

void Player::SetHealth( int In )
{
    Health = In;
    
    if( King )
        King->UpdateHealth( In );
}


