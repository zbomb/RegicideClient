//
//  HandEntity.cpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/11/18.
//

#include "HandEntity.hpp"
#include "cryptolib/Random.hpp"

using namespace Game;


HandEntity::HandEntity()
: EntityBase( "Hand" )
{
    
}

HandEntity::~HandEntity()
{
    
}

void HandEntity::Cleanup()
{
    EntityBase::Cleanup();
}

bool HandEntity::IndexValid( uint32 Index ) const
{
    return Cards.size() > Index;
}

CardEntity* HandEntity::At( uint32 Index )
{
    if( !IndexValid( Index ) )
        return nullptr;
    
    return Cards.at( Index );
}

CardEntity* HandEntity::operator[]( uint32 Index )
{
    return At( Index );
}


void HandEntity::AddToTop( CardEntity *Input, bool bMoveSprite )
{
    if( Input )
    {
        Cards.push_front( Input );
        Reposition( Input, false );
        ICardContainer::SetCardContainer( Input );
        
        if( bMoveSprite )
        {
            InvalidateZOrder();
            MoveCard( Input, CalcPos( 0 ) );
        }
    }
}

void HandEntity::AddToBottom( CardEntity* Input, bool bMoveSprite )
{
    if( Input )
    {
        Cards.push_back( Input );
        Reposition( Input, false );
        ICardContainer::SetCardContainer( Input );
        
        int Index = (int)Cards.size() - 1;
        
        if( bMoveSprite )
        {
            InvalidateZOrder();
            MoveCard( Input, CalcPos( Index ) );
        }
    }
}

void HandEntity::AddAtRandom( CardEntity* Input, bool bMoveSprite )
{
    if( Input )
    {
        // Generate random index
        using Random = effolkronium::random_static;
        auto Iter = Random::get( Cards.begin(), Cards.end() );
        //CC_ASSERT( Iter != Cards.end() );
        
        auto It = Cards.insert( Iter, Input );
        Reposition( Input, false );
        ICardContainer::SetCardContainer( Input );
        
        int Index = (int)( It - Cards.begin() );
        
        if( bMoveSprite )
        {
            InvalidateZOrder();
            MoveCard( Input, CalcPos( Index ) );
        }
    }
}

void HandEntity::AddAtIndex( CardEntity* Input, uint32 Index, bool bMoveSprite )
{
    if( Input )
    {
        // TODO: Evalulate if this assert is needed
        CC_ASSERT( Index <= Cards.size() );
        
        auto It = Cards.begin();
        std::advance( It, Index );
        
        Cards.insert( It, Input );
        Reposition( Input, false );
        ICardContainer::SetCardContainer( Input );
        
        if( bMoveSprite )
        {
            InvalidateZOrder();
            MoveCard( Input, CalcPos( Index ) );
        }
    }
}

bool HandEntity::RemoveTop( bool bDestroy /* = false */ )
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
    Reposition( nullptr );
    InvalidateZOrder();
    
    return true;
}

bool HandEntity::RemoveBottom( bool bDestroy /* = false */ )
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
    Reposition( nullptr );
    InvalidateZOrder();
    
    return true;
}

bool HandEntity::RemoveAtIndex( uint32 Index, bool bDestroy /* = false */ )
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
    Reposition( nullptr );
    InvalidateZOrder();
    
    return true;
}

bool HandEntity::RemoveRandom( bool bDestroy /* = false */ )
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
    Reposition( nullptr );
    InvalidateZOrder();
    
    return true;
}

void HandEntity::Reposition( CardEntity* Ignore, bool bFillIgnoredSpace )
{
    int Index = 0;
    auto dir = cocos2d::Director::getInstance();
    auto size = dir->getVisibleSize();
    
    // If were ignoring a card and dont want to fill the place, we need to subtract one
    // from total card count in CalcPos
    for( auto It = Cards.begin(); It != Cards.end(); It++ )
    {
        if( (*It) && *It != Ignore )
        {
            (*It)->MoveAnimation( CalcPos( Index, 0 ), 0.4f );
        }
        
        Index++;
    }
}

void HandEntity::Invalidate()
{
    EntityBase::Invalidate();
    
    // Cards and decks share the same parent, instead of the cards being children
    // of decks. So on invalidate we need to update the card positions manually
    
    // The cards will have some overlap when in hand, the more cards, the more overlap
    // Then, we have to spread the cards out evenly, also with an increasing with although a smaller factor
    // Overlap = Count * 1px;
    // TotalWidth = Count * ( CardWidth - ( Count * 1px ) )
    if( !Cards.empty() && Cards.front() )
    {
        float OverlapFactor = 2.f;
        auto ct = Count() - 1;
        
        float CardWidth = Cards.front()->GetWidth();
        
        float Overlap = ct * OverlapFactor - 6.f;
        float Spacing = CardWidth - Overlap;
        float TotalWidth = ct * Spacing;
        
        // Now we can space the cards out over this width, since the cards are
        // anchored by the mid point, we subtracted an extra card width from the result
        int Index = 0;
        for( auto It = Begin(); It != End(); It++ )
        {
            if( *It )
            {
                float PositionX = ( - TotalWidth / 2.f ) + Index * Spacing;
                //(*It)->SetPosition( GetPosition() + cocos2d::Vec2( PositionX, 0.f ) );
                //(*It)->SetRotation( GetRotation() );
            }
            
            Index++;
        }
    }
}

cocos2d::Vec2 HandEntity::CalcPos( int Index, int CardDelta )
{
    if( Cards.empty() || !Cards.front() )
        return GetPosition();
    
    float OverlapFactor = 2.f;
    auto ct = Count() - 1;
    float CardWidth = Cards.front()->GetWidth();
    float Overlap = ct * OverlapFactor - 6.f;
    float Spacing = CardWidth - Overlap;
    float TotalWidth = ct * Spacing;
    
    auto dir = cocos2d::Director::getInstance();
    auto size = dir->getVisibleSize();
    
    return GetPosition() + cocos2d::Vec2( ( -TotalWidth / 2.f ) + Index * Spacing, bExpanded ? size.height * 0.15f : 0.f );
}

void HandEntity::MoveCard( CardEntity* inCard, const cocos2d::Vec2& AbsPos )
{
    if( inCard )
    {
        // Move card
        inCard->MoveAnimation( AbsPos, 0.4f );
        
        // Flip face up if it isnt already
        if( bVisibleLocally && !inCard->IsFaceUp() )
            inCard->Flip( true, 0.4f );
        
        // If were flipped upside down, then ensure the card is also upside down
        auto rot = GetAbsoluteRotation();
        if( rot > 90.f || rot < -90.f )
        {
            float cardRot = inCard->GetAbsoluteRotation();
            if( cardRot < 1.f || cardRot > -1.f )
            {
                inCard->RotateAnimation( GetAbsoluteRotation(), 0.4f );
            }
        }
    }
}

void HandEntity::InvalidateZOrder()
{
    // Top of deck gets a higher Z Order
    // So, we need to order the cards
    // Default card Z order is 10
    
    int Top = (int)Cards.size() + 100;
    
    for( int i = 0; i < Cards.size(); i++ )
    {
        int Order = Top - i;
        if( Cards[ i ] )
        {
            Cards[ i ]->SetZ( Order );
        }
    }
}

void HandEntity::SetExpanded( bool bExpand )
{
    if( bExpand == bExpanded )
        return;
    
    bExpanded = bExpand;
    cocos2d::log( "[DEBUG] EXPANDING %s", bExpanded ? "true" : "false" );
    Reposition( nullptr );
}
