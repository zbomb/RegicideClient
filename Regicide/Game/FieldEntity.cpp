//
//  FieldEntity.cpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/11/18.
//

#include "FieldEntity.hpp"
#include "cryptolib/Random.hpp"

#define REG_CARDS_PER_ROW 8

using namespace Game;

FieldEntity::FieldEntity()
: EntityBase( "Field" )
{
    
}

FieldEntity::~FieldEntity()
{
    
}

void FieldEntity::Cleanup()
{
    EntityBase::Cleanup();
}

bool FieldEntity::IndexValid( uint32 Index ) const
{
    return Cards.size() > Index;
}

CardEntity* FieldEntity::At( uint32 Index )
{
    if( !IndexValid( Index ) )
        return nullptr;
    
    return Cards.at( Index );
}

CardEntity* FieldEntity::operator[]( uint32 Index )
{
    return At( Index );
}


void FieldEntity::AddToTop( CardEntity *Input, bool bMoveSprite )
{
    if( Input )
    {
        Cards.push_front( Input );
        Reposition( Input, false );
        ICardContainer::SetCardContainer( Input );
        
        if( bMoveSprite )
        {
            InvalidateZOrder();
            auto goodPos = CalcPos( 0 );
            MoveCard( Input, goodPos );
        }
    }
}

void FieldEntity::AddToBottom( CardEntity* Input, bool bMoveSprite )
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

void FieldEntity::AddAtRandom( CardEntity* Input, bool bMoveSprite )
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

void FieldEntity::AddAtIndex( CardEntity* Input, uint32 Index, bool bMoveSprite )
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

bool FieldEntity::RemoveTop( bool bDestroy /* = false */ )
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

bool FieldEntity::RemoveBottom( bool bDestroy /* = false */ )
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

bool FieldEntity::RemoveAtIndex( uint32 Index, bool bDestroy /* = false */ )
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

bool FieldEntity::RemoveRandom( bool bDestroy /* = false */ )
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

void FieldEntity::Reposition( CardEntity* Ignore, bool bFillIgnoredSpace )
{
    int Index = 0;
    
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

void FieldEntity::Invalidate()
{
    EntityBase::Invalidate();
    
    // Cards and decks share the same parent, instead of the cards being children
    // of decks. So on invalidate we need to update the card positions manually
    
    auto dir = cocos2d::Director::getInstance();
    auto size = dir->getVisibleSize();
    
    if( !Cards.empty() && Cards.front() )
    {
        // Were going to have a set spacing for cards, and if we go over 10 cards,
        // we will break it out into another row
        auto ct = (int)Count() - 1;
        float CardWidth = Cards.front()->GetWidth();

        float Avail = size.width * 0.7f;
        int FirstRow    = ct > 10 ? 10 : ct;
        int SecondRow = ct > 10 ? ct - 10 : 0;
        
        float Spacing = Avail / 10.f;
        
        float TotalWide = Spacing * FirstRow;
        float TotalWideTwo = Spacing * SecondRow;
        float Start = TotalWide * -0.5f;
        
        // Now we can space the cards out over this width, since the cards are
        // anchored by the mid point, we subtracted an extra card width from the result
        int Index = 0;
        int Row = 0;
        for( auto It = Begin(); It != End(); It++ )
        {
            if( Index >= 10 )
            {
                Index = 0;
                Row = 1;
            }
            
            if( *It )
            {
                //float PositionX = ( - TotalWidth / 2.f ) + Index * Spacing;
                //(*It)->SetPosition( GetPosition() + cocos2d::Vec2( PositionX, 0.f ) );
                //(*It)->SetRotation( GetRotation() );
            }
            
            Index++;
        }
    }
}

cocos2d::Vec2 FieldEntity::CalcPos( int Index, int CardDelta )
{
    if( Cards.empty() || !Cards.front() )
        return GetPosition();
    
    auto size = cocos2d::Director::getInstance()->getVisibleSize();
    
    auto ct = (int)Count() - 1;
    float CardWidth = Cards.front()->GetWidth();
    
    float Avail = size.width * 0.65f;
    int FirstRow    = ct > REG_CARDS_PER_ROW - 1 ? REG_CARDS_PER_ROW - 1 : ct;
    int SecondRow = ct > REG_CARDS_PER_ROW - 1 ? ct - REG_CARDS_PER_ROW : 0;
    
    // If were overloading, which should be prevented at the gamemode/authority level
    // then we will just cram more cards into the second row, reduce spacing to fit them all
    float Spacing = Index > REG_CARDS_PER_ROW - 1 && SecondRow > REG_CARDS_PER_ROW - 1 ? Avail / (float) ( SecondRow + 1 ) : Avail / (float) REG_CARDS_PER_ROW;
    
    float TotalWide = Spacing * FirstRow;
    float TotalWideTwo = Spacing * SecondRow;
    
    float Start     = Index > REG_CARDS_PER_ROW - 1 ? TotalWideTwo * -0.5f : TotalWide * -0.5f;
    int RowIndx     = Index > REG_CARDS_PER_ROW - 1 ? Index - REG_CARDS_PER_ROW : Index;
    int RowNum      = Index > REG_CARDS_PER_ROW - 1 ? -1 : 1;
    
    // Check if were rotated, because the second row would extend in a different direction
    float rot = GetAbsoluteRotation();
    if( rot > 90.f || rot < -90.f )
        RowNum *= -1;
    
    return GetPosition() + cocos2d::Vec2( Start + RowIndx * Spacing, RowNum * CardWidth * 0.79f );

}

void FieldEntity::MoveCard( CardEntity* inCard, const cocos2d::Vec2& AbsPos )
{
    if( inCard )
    {
        // Move card
        inCard->MoveAnimation( AbsPos, 0.4f );
        
        // Flip face up if it isnt already
        if( !inCard->IsFaceUp() )
            inCard->Flip( true, 0.4f );
        
        // On field, card should always be upright
        float absRot = inCard->GetAbsoluteRotation();
        if( absRot > 1.f || absRot < -1.f )
        {
            inCard->RotateAnimation( 0.f, 0.4f );
        }
    }
}

void FieldEntity::InvalidateZOrder()
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
