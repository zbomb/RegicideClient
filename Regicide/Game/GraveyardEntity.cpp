//
//  GraveyardEntity.cpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/11/18.
//

#include "GraveyardEntity.hpp"
#include "cryptolib/Random.hpp"


using namespace Game;

GraveyardEntity::GraveyardEntity()
: EntityBase( "Graveyard" )
{
    
}

GraveyardEntity::~GraveyardEntity()
{
    
}

void GraveyardEntity::Cleanup()
{
    
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

void GraveyardEntity::AddToTop( CardEntity *Input, bool bMoveSprite )
{
    if( Input )
    {
        Cards.push_front( Input );
        ICardContainer::SetCardContainer( Input );
        
        if( bMoveSprite )
        {
            InvalidateZOrder();
            MoveCard( Input );
        }
    }
}

void GraveyardEntity::AddToBottom( CardEntity* Input, bool bMoveSprite )
{
    if( Input )
    {
        Cards.push_back( Input );
        ICardContainer::SetCardContainer( Input );
        
        if( bMoveSprite )
        {
            InvalidateZOrder();
            MoveCard( Input );
        }
    }
}

void GraveyardEntity::AddAtRandom( CardEntity* Input, bool bMoveSprite )
{
    if( Input )
    {
        // Generate random index
        using Random = effolkronium::random_static;
        auto Iter = Random::get( Cards.begin(), Cards.end() );
        //CC_ASSERT( Iter != Cards.end() );
        
        Cards.insert( Iter, Input );
        ICardContainer::SetCardContainer( Input );
        
        if( bMoveSprite )
        {
            InvalidateZOrder();
            MoveCard( Input );
        }
    }
}

void GraveyardEntity::AddAtIndex( CardEntity* Input, uint32 Index, bool bMoveSprite )
{
    if( Input )
    {
        // TODO: Evalulate if this assert is needed
        CC_ASSERT( Index <= Cards.size() );
        
        auto It = Cards.begin();
        std::advance( It, Index );
        
        Cards.insert( It, Input );
        ICardContainer::SetCardContainer( Input );
        
        if( bMoveSprite )
        {
            InvalidateZOrder();
            MoveCard( Input );
        }
    }
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
    InvalidateZOrder();
    
    return true;
}

bool GraveyardEntity::RemoveRandom( bool bDestroy /* = false */ )
{
    if( Cards.empty() )
        return false;
    
    // Choose random card
    using Random = effolkronium::random_static;
    auto It = Random::get( Cards.begin(), Cards.end() );
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
    InvalidateZOrder();
    
    return true;
}

void GraveyardEntity::Invalidate()
{
    EntityBase::Invalidate();
    
    // Cards and decks share the same parent, instead of the cards being children
    // of decks. So on invalidate we need to update the card positions manually
    for( auto It = Begin(); It != End(); It++ )
    {
        if( *It )
        {
            (*It)->SetPosition( GetPosition() );
            (*It)->SetRotation( GetRotation() );
        }
    }
}

void GraveyardEntity::MoveCard( CardEntity* inCard )
{
    if( inCard )
    {
        // Move card
        inCard->MoveAnimation( GetPosition(), 0.4f );
        
        // Flip face up if it isnt already
        if( !inCard->IsFaceUp() )
            inCard->Flip( true, 0.4f );
        
        // Card should always be face up
        float absRot = inCard->GetAbsoluteRotation();
        if( absRot > 1.f || absRot < -1.f )
        {
            inCard->RotateAnimation( 0.f, 0.4f );
        }
    }
}

void GraveyardEntity::InvalidateZOrder()
{
    // Top of deck gets a higher Z Order
    // So, we need to order the cards
    // Default card Z order is 10
    
    int Top = (int)Cards.size() + 11;
    
    for( int i = 0; i < Cards.size(); i++ )
    {
        int Order = Top - i;
        if( Cards[ i ] )
        {
            Cards[ i ]->SetZ( Order );
        }
    }
}

