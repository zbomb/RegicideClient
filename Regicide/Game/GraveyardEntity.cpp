//
//    GraveyardEntity.cpp
//    Regicide Mobile
//
//    Created: 11/11/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#include "GraveyardEntity.hpp"


using namespace Game;

GraveyardEntity::GraveyardEntity()
: EntityBase( "Graveyard" )
{
    SetTag( TAG_GRAVE );
}

GraveyardEntity::~GraveyardEntity()
{
    
}

void GraveyardEntity::Cleanup()
{
    EntityBase::Cleanup();
}


bool GraveyardEntity::IndexValid( uint32 Index ) const
{
    return Cards.size() > Index;
}

CardEntity* GraveyardEntity::At( uint32 Index )
{
    if( !IndexValid( Index ) )
        return nullptr;
    
    return Cards.at( Index );
}

CardEntity* GraveyardEntity::operator[]( uint32 Index )
{
    return At( Index );
}

CardEntity* GraveyardEntity::DrawCard()
{
    if( Cards.empty() )
        return nullptr;
    
    CardEntity* Output = Cards.front();
    Cards.pop_front();
    
    ICardContainer::ClearCardContainer( Output );
    InvalidateZOrder();
    
    return Output;
}

void GraveyardEntity::AddToTop( CardEntity *Input, bool bMoveSprite, std::function< void() > Callback )
{
    if( Input )
    {
        Cards.push_front( Input );
        ICardContainer::SetCardContainer( Input );
        InvalidateCards( Input );
        
        if( bMoveSprite )
        {
            InvalidateZOrder();
            MoveCard( Input, Callback );
        }
        else if( Callback )
            Callback();
    }
}

void GraveyardEntity::AddToBottom( CardEntity* Input, bool bMoveSprite, std::function< void() > Callback )
{
    if( Input )
    {
        Cards.push_back( Input );
        ICardContainer::SetCardContainer( Input );
        InvalidateCards( Input );
        
        if( bMoveSprite )
        {
            InvalidateZOrder();
            MoveCard( Input, Callback );
        }
        else if( Callback )
            Callback();
    }
}

void GraveyardEntity::AddAtRandom( CardEntity* Input, bool bMoveSprite, std::function< void() > Callback )
{
    if( Input )
    {
        // Generate random index
        auto Iter = Cards.begin();
        std::advance( Iter, cocos2d::random< int>( 0, (int)Cards.size() ) );
        
        Cards.insert( Iter, Input );
        ICardContainer::SetCardContainer( Input );
        InvalidateCards( Input );
        
        if( bMoveSprite )
        {
            InvalidateZOrder();
            MoveCard( Input, Callback );
        }
        else if( Callback )
            Callback();
    }
}

void GraveyardEntity::AddAtIndex( CardEntity* Input, uint32 Index, bool bMoveSprite, std::function< void() > Callback )
{
    if( Input )
    {
        // TODO: Evalulate if this assert is needed
        CC_ASSERT( Index <= Cards.size() );
        
        auto It = Cards.begin();
        std::advance( It, Index );
        
        Cards.insert( It, Input );
        ICardContainer::SetCardContainer( Input );
        InvalidateCards( Input );
        
        if( bMoveSprite )
        {
            InvalidateZOrder();
            MoveCard( Input, Callback );
        }
        else if( Callback )
            Callback();
    }
}

bool GraveyardEntity::Remove( CardEntity* inCard, bool bDestroy )
{
    if( !inCard || Cards.empty() )
        return false;
    
    // Lookup card
    for( auto It = Cards.begin(); It != Cards.end(); It++ )
    {
        if( *It && *It == inCard )
        {
            if( bDestroy )
            {
                IEntityManager::GetInstance().DestroyEntity( inCard );
            }
            else
            {
                ClearCardContainer( inCard );
            }
            
            Cards.erase( It );
            InvalidateCards();
            InvalidateZOrder();
            
            return true;
        }
    }
    
    return false;
}

bool GraveyardEntity::RemoveTop( bool bDestroy /* = false */ )
{
    if( Cards.empty() )
        return false;
    
    if( bDestroy )
    {
        IEntityManager::GetInstance().DestroyEntity( Cards.front() );
    }
    else
    {
        ICardContainer::ClearCardContainer( Cards.front() );
    }
    
    Cards.pop_front();
    InvalidateCards();
    InvalidateZOrder();
    
    return true;
}

bool GraveyardEntity::RemoveBottom( bool bDestroy /* = false */ )
{
    if( Cards.empty() )
        return false;
    
    if( bDestroy )
    {
        IEntityManager::GetInstance().DestroyEntity( Cards.back() );
    }
    else
    {
        ICardContainer::ClearCardContainer( Cards.back() );
    }
    
    Cards.pop_back();
    InvalidateCards();
    InvalidateZOrder();
    
    return true;
}

bool GraveyardEntity::RemoveAtIndex( uint32 Index, bool bDestroy /* = false */ )
{
    if( Cards.size() < Index )
        return false;
    
    auto It = Cards.begin();
    std::advance( It, Index );
    
    if( bDestroy && *It )
    {
        IEntityManager::GetInstance().DestroyEntity( *It );
    }
    else if( *It )
    {
        ICardContainer::ClearCardContainer( *It );
    }
    
    Cards.erase( It );
    InvalidateCards();
    InvalidateZOrder();
    
    return true;
}

bool GraveyardEntity::RemoveRandom( bool bDestroy /* = false */ )
{
    if( Cards.empty() )
        return false;
    
    // Choose random card
    auto It = Cards.begin();
    std::advance( It, cocos2d::random< int >( 0, (int) Cards.size() - 1 ) );
    CC_ASSERT( It != Cards.end() );
    
    if( bDestroy && *It )
    {
        IEntityManager::GetInstance().DestroyEntity( *It );
    }
    else if( *It )
    {
        ICardContainer::ClearCardContainer( *It );
    }
    
    Cards.erase( It );
    InvalidateCards();
    InvalidateZOrder();
    
    return true;
}

void GraveyardEntity::Invalidate()
{
    EntityBase::Invalidate();
    
    // Cards and decks share the same parent, instead of the cards being children
    // of decks. So on invalidate we need to update the card positions manually
    InvalidateCards();
}

void GraveyardEntity::InvalidateCards( CardEntity* inCard )
{
    int Index = 0;
    for( auto It = Begin(); It != End(); It++ )
    {
        float Offset = Index * 0.1f;
        if( *It && *It != inCard && !(*It)->GetIsDragging() )
        {
            (*It)->SetPosition( GetPosition() - cocos2d::Vec2( Offset, 0.f ) );
            (*It)->SetRotation( 0.f );
        }
        
        Index++;
    }
}

void GraveyardEntity::MoveCard( CardEntity* inCard, std::function< void() > Callback )
{
    if( inCard )
    {
        // Move card
        inCard->MoveAnimation( GetPosition(), CARD_DEFAULT_MOVE_TIME );
        
        // Flip face up if it isnt already
        if( !inCard->FaceUp )
            inCard->Flip( true, CARD_DEFAULT_MOVE_TIME );
        
        // Card should always be face up
        float absRot = inCard->GetAbsoluteRotation();
        if( absRot > 1.f || absRot < -1.f )
        {
            inCard->RotateAnimation( 0.f, CARD_DEFAULT_MOVE_TIME );
        }
        
        FinishAction( Callback, CARD_DEFAULT_MOVE_TIME + 0.1f );
    }
    else
    {
        FinishAction( Callback );
    }
}

void GraveyardEntity::InvalidateZOrder()
{
    // Z Order between 1 and 100
    int Top = (int)Cards.size() + 1;
    Top = Top > 100 ? 100 : Top;
    
    for( int i = 0; i < Cards.size(); i++ )
    {
        int Order = Top - i;
        Order = Order < 1 ? 1 : Order;
        
        if( Cards[ i ] )
        {
            Cards[ i ]->SetZ( Order );
        }
    }
}

void GraveyardEntity::Clear()
{
    auto& Ent = IEntityManager::GetInstance();
    for( auto It = Cards.begin(); It != Cards.end(); It++ )
    {
        if( *It )
            Ent.DestroyEntity( *It );
    }
    
    Cards.clear();
}
