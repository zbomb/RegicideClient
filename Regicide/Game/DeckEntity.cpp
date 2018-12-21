//
//    DeckEntity.cpp
//    Regicide Mobile
//
//    Created: 11/10/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#include "DeckEntity.hpp"

using namespace Game;


DeckEntity::DeckEntity()
    : EntityBase( "Deck" ), Counter( nullptr )
{
    SetTag( TAG_DECK );
    bAddedToScene = false;
}

DeckEntity::~DeckEntity()
{
    if( Counter )
    {
        Counter->removeFromParent();
    }
    
    Counter = nullptr;
}

void DeckEntity::Cleanup()
{
    EntityBase::Cleanup();
}

void DeckEntity::AddToScene( cocos2d::Node* In )
{
    if( bAddedToScene )
        return;
    
    bAddedToScene = true;

    Counter = cocos2d::Label::createWithTTF( std::to_string( Count() ), "fonts/arial.ttf", 50.f );
    Counter->setAnchorPoint( cocos2d::Vec2( 0.5f, 0.5f ) );
    Counter->setTextColor( cocos2d::Color4B( 255, 255, 255, 255 ) );
    
    In->addChild( Counter, 100 );
}

bool DeckEntity::IndexValid( uint32 Index ) const
{
    return Cards.size() > Index;
}

CardEntity* DeckEntity::At( uint32 Index )
{
    if( !IndexValid( Index ) )
        return nullptr;
    
    return Cards.at( Index );
}

CardEntity* DeckEntity::operator[]( uint32 Index )
{
    return At( Index );
}

CardEntity* DeckEntity::DrawCard()
{
    if( Cards.empty() )
        return nullptr;
    
    CardEntity* Output = Cards.front();
    Cards.pop_front();
    
    ClearCardContainer( Output );
    InvalidateZOrder();
    InvalidateCards();
    UpdateCounter();
    
    return Output;
}

void DeckEntity::AddToTop( CardEntity *Input, bool bMoveSprite, std::function< void() > Callback )
{
    if( Input )
    {
        Cards.push_front( Input );
        SetCardContainer( Input );
        InvalidateCards( Input );
        UpdateCounter();
        
        if( bMoveSprite )
        {
            InvalidateZOrder();
            MoveCard( Input, Callback );
        }
        else if( Callback )
            Callback();
    }
}

void DeckEntity::AddToBottom( CardEntity* Input, bool bMoveSprite, std::function< void() > Callback )
{
    if( Input )
    {
        Cards.push_back( Input );
        SetCardContainer( Input );
        InvalidateCards( Input );
        UpdateCounter();
        
        if( bMoveSprite )
        {
            InvalidateZOrder();
            MoveCard( Input, Callback );
        }
        else if( Callback )
            Callback();
    }
}

void DeckEntity::AddAtRandom( CardEntity* Input, bool bMoveSprite, std::function< void() > Callback )
{
    if( Input )
    {
        // Generate random index
        auto Iter = Cards.begin();
        std::advance( Iter, cocos2d::random< int >( 0, (int) Cards.size() ) );
        
        Cards.insert( Iter, Input );
        SetCardContainer( Input );
        InvalidateCards( Input );
        UpdateCounter();
        
        if( bMoveSprite )
        {
            InvalidateZOrder();
            MoveCard( Input, Callback );
        }
        else if( Callback )
            Callback();
    }
}

void DeckEntity::AddAtIndex( CardEntity* Input, uint32 Index, bool bMoveSprite, std::function< void() > Callback )
{
    if( Input )
    {
        // TODO: Evalulate if this assert is needed
        CC_ASSERT( Index <= Cards.size() );
        
        auto It = Cards.begin();
        std::advance( It, Index );
        
        Cards.insert( It, Input );
        SetCardContainer( Input );
        InvalidateCards( Input );
        UpdateCounter();
        
        if( bMoveSprite )
        {
            InvalidateZOrder();
            MoveCard( Input, Callback );
        }
        else if( Callback )
            Callback();
    }
}

bool DeckEntity::Remove( CardEntity* inCard, bool bDestroy )
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
            UpdateCounter();
            
            return true;
        }
    }
    
    return false;
}

bool DeckEntity::RemoveTop( bool bDestroy /* = false */ )
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
    UpdateCounter();
    
    return true;
}

bool DeckEntity::RemoveBottom( bool bDestroy /* = false */ )
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
    UpdateCounter();
    
    return true;
}

bool DeckEntity::RemoveAtIndex( uint32 Index, bool bDestroy /* = false */ )
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
    UpdateCounter();
    
    return true;
}

bool DeckEntity::RemoveRandom( bool bDestroy /* = false */ )
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
    UpdateCounter();
    
    return true;
}

void DeckEntity::UpdateCounter()
{
    if( Counter )
    {
        Counter->setString( std::to_string( Count() ) );
    }
}

void DeckEntity::Invalidate()
{
    EntityBase::Invalidate();
    
    // Cards and decks share the same parent, instead of the cards being children
    // of decks. So on invalidate we need to update the card positions manually
    InvalidateCards();
    
    // Update the Counter label
    if( Counter )
    {
        float CardHeight = 0.f;
        for( auto It = Begin(); It != End(); It++ )
        {
            if( *It && (*It)->Sprite )
            {
                CardHeight = (*It)->Sprite->getContentSize().height;
                break;
            }
        }
        
        Counter->setPosition( GetAbsolutePosition() + cocos2d::Vec2( 0.f, -CardHeight * 0.5f - Counter->getContentSize().height * 0.5f - 5.f ) );
    }
}

void DeckEntity::InvalidateCards( CardEntity* Ignore )
{
    int Index = 0;
    for( auto It = Begin(); It != End(); It++ )
    {
        float Offset = Index * 0.1f;
        if( *It && *It != Ignore && !(*It)->GetIsDragging() )
        {
            (*It)->SetPosition( GetPosition() - cocos2d::Vec2( Offset, 0.f ) );
            (*It)->SetRotation( 0.f );
        }
        
        Index++;
    }
}

void DeckEntity::MoveCard( CardEntity* inCard, std::function< void() > Callback )
{
    if( inCard )
    {
        // Move card
        inCard->MoveAnimation( GetPosition(), CARD_DEFAULT_MOVE_TIME );
        
        // Make sure its face down if in deck
        if( inCard->FaceUp )
            inCard->Flip( false, CARD_DEFAULT_MOVE_TIME );
        
        // Card should always be rotated face up, irregardless of parent
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

void DeckEntity::InvalidateZOrder()
{
    // Z Order Between 1-100
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

void DeckEntity::Clear()
{
    auto& Ent = IEntityManager::GetInstance();
    for( auto It = Cards.begin(); It != Cards.end(); It++ )
    {
        if( *It )
            Ent.DestroyEntity( *It );
    }
    
    Cards.clear();
}
