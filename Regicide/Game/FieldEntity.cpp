//
//    FieldEntity.cpp
//    Regicide Mobile
//
//    Created: 11/11/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#include "FieldEntity.hpp"
#include "Game/World.hpp"
#include "Game/SingleplayerAuthority.hpp"

#define REG_CARDS_PER_ROW 8

using namespace Game;

FieldEntity::FieldEntity()
: EntityBase( "Field" )
{
    SetTag( TAG_FIELD );
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


void FieldEntity::AddToTop( CardEntity *Input, bool bMoveSprite, std::function< void() > Callback )
{
    if( Input )
    {
        Cards.push_front( Input );
        InvalidateCards( Input );
        SetCardContainer( Input );
        
        if( bMoveSprite )
        {
            InvalidateZOrder();
            auto goodPos = CalcPos( 0 );
            MoveCard( Input, goodPos, Callback );
        }
        else if( Callback )
            Callback();
    }
}

void FieldEntity::AddToBottom( CardEntity* Input, bool bMoveSprite, std::function< void() > Callback )
{
    if( Input )
    {
        Cards.push_back( Input );
        InvalidateCards( Input );
        SetCardContainer( Input );
        
        int Index = (int)Cards.size() - 1;
        
        if( bMoveSprite )
        {
            InvalidateZOrder();
            MoveCard( Input, CalcPos( Index ), Callback );
        }
        else if( Callback )
            Callback();
    }
}

void FieldEntity::AddAtRandom( CardEntity* Input, bool bMoveSprite, std::function< void() > Callback )
{
    if( Input )
    {
        // Generate random index
        auto Iter = Cards.begin();
        std::advance( Iter, cocos2d::random< int >( 0, (int) Cards.size() ) );
        
        auto It = Cards.insert( Iter, Input );
        InvalidateCards( Input );
        SetCardContainer( Input );
        
        int Index = (int)( It - Cards.begin() );
        
        if( bMoveSprite )
        {
            InvalidateZOrder();
            MoveCard( Input, CalcPos( Index ), Callback );
        }
        else if( Callback )
            Callback();
    }
}

void FieldEntity::AddAtIndex( CardEntity* Input, uint32 Index, bool bMoveSprite, std::function< void() > Callback )
{
    if( Input )
    {
        // TODO: Evalulate if this assert is needed
        CC_ASSERT( Index <= Cards.size() );
        
        auto It = Cards.begin();
        std::advance( It, Index );
        
        Cards.insert( It, Input );
        InvalidateCards( Input );
        SetCardContainer( Input );
        
        if( bMoveSprite )
        {
            InvalidateZOrder();
            MoveCard( Input, CalcPos( Index ), Callback );
        }
        else if( Callback )
            Callback();
    }
}

bool FieldEntity::Remove( CardEntity* inCard, bool bDestroy )
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
        ClearCardContainer( Cards.front() );
    }
    
    Cards.pop_front();
    InvalidateCards();
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
        ClearCardContainer( Cards.back() );
    }
    
    Cards.pop_back();
    InvalidateCards();
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
        ClearCardContainer( *It );
    }
    
    Cards.erase( It );
    InvalidateCards();
    InvalidateZOrder();
    
    return true;
}

bool FieldEntity::RemoveRandom( bool bDestroy /* = false */ )
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
        ClearCardContainer( *It );
    }
    
    Cards.erase( It );
    InvalidateCards();
    InvalidateZOrder();
    
    return true;
}

void FieldEntity::InvalidateCards( CardEntity* Ignore /* = nullptr */ )
{
    if( _bInvalidatePaused )
        return;
    
    int Index = 0;
    
    // If were ignoring a card and dont want to fill the place, we need to subtract one
    // from total card count in CalcPos
    for( auto It = Cards.begin(); It != Cards.end(); It++ )
    {
        if( (*It) && *It != Ignore && !(*It)->GetIsDragging() )
        {
            (*It)->MoveAnimation( CalcPos( Index, 0 ), 0.3f );
        }
        
        Index++;
    }
}

void FieldEntity::PauseInvalidate()
{
    _bInvalidatePaused = true;
}

void FieldEntity::ResumeInvalidate()
{
    _bInvalidatePaused = false;
    InvalidateCards();
}

void FieldEntity::Invalidate()
{
    EntityBase::Invalidate();
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

void FieldEntity::MoveCard( CardEntity* inCard, const cocos2d::Vec2& inPos, std::function< void() > Callback )
{
    if( inCard )
    {
        // Move card
        inCard->MoveAnimation( inPos, CARD_DEFAULT_MOVE_TIME );
        
        // Flip face up if it isnt already
        if( !inCard->FaceUp )
            inCard->Flip( true, CARD_DEFAULT_MOVE_TIME );
        
        // On field, card should always be upright
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

void FieldEntity::InvalidateZOrder()
{
    // Z Order between 1 and 50
    int Top = (int)Cards.size() + 1;
    Top = Top > 50 ? 50 : Top;
    
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


int FieldEntity::AttemptDrop( CardEntity *inCard, const cocos2d::Vec2 &inPos )
{
    if( !inCard )
    {
        return -1;
    }
    
    // Were going to get the number of cards, add one and determine where each card would
    // be if we added this additional card, we will then determine which point the card is
    // closest to, and if its too far away from the closest point, then we will return false
    // To screen the validity of the attempts, we will build a quick rect around where we consider
    // valid drop positions, and if the card is outside of that then return false
    
    auto thisPos = GetAbsolutePosition();
    auto dir = cocos2d::Director::getInstance();
    auto size = dir->getVisibleSize();
    
    float thisW = size.width * 0.7f;
    float thisH = size.height * 0.42f;
    
    cocos2d::Rect Bounds( thisPos.x - thisW / 2.f, thisPos.y - thisH / 2.f, thisW, thisH );
    if( !Bounds.containsPoint( inPos ) )
    {
        return -1;
    }

    int CardCount       = (int)Count() + 1;
    float ShortestDist  = size.height / 3.f; // Also min required distance
    int BestIndex       = -1;
    cocos2d::Vec2 Abs   = GetOwner() ? GetOwner()->GetAbsolutePosition() : cocos2d::Vec2::ZERO;
    
    for( int i = 0; i < CardCount; i++ )
    {
        float Dist = ( CalcPos( i ) + Abs ).getDistance( inPos );

        if( Dist < ShortestDist )
        {
            ShortestDist = Dist;
            BestIndex = i;
        }
    }
    
    // Return the best index we found
    return BestIndex >= 0 ? BestIndex : -1;
}


luabridge::LuaRef FieldEntity::_lua_GetCards()
{
    auto Engine = Regicide::LuaEngine::GetInstance();
    CC_ASSERT( Engine );
    
    luabridge::LuaRef Output = luabridge::newTable( Engine->State() );
    
    int Index = 1;
    for( auto It = Begin(); It != End(); It++ )
    {
        if( *It )
        {
            Output[ Index++ ] = (*It);
        }
    }
    
    return Output;
}

void FieldEntity::Clear()
{
    auto& Ent = IEntityManager::GetInstance();
    for( auto It = Cards.begin(); It != Cards.end(); It++ )
    {
        if( *It )
            Ent.DestroyEntity( *It );
    }
    
    Cards.clear();
}
