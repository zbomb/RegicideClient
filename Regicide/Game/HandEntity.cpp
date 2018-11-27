//
//    HandEntity.cpp
//    Regicide Mobile
//
//    Created: 11/10/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#include "HandEntity.hpp"
#include "UI/CardSelector.hpp"
#include "Game/Player.hpp"
#include "Game/World.hpp"
#include "Game/AuthorityBase.hpp"

using namespace Game;


HandEntity::HandEntity()
: EntityBase( "Hand" ), Selector( nullptr )
{
    SetTag( TAG_HAND );
    
    bExpanded       = false;
    bVisibleLocally = false;
    bBlitzMode      = false;
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


void HandEntity::AddToTop( CardEntity *Input, bool bMoveSprite, std::function< void() > Callback )
{
    if( Input )
    {
        Cards.push_front( Input );
        InvalidateCards( Input );
        ICardContainer::SetCardContainer( Input );
        
        if( bMoveSprite )
        {
            InvalidateZOrder();
            MoveCard( Input, CalcPos( 0 ), Callback );
        }
        else if( Callback )
            Callback();
    }
}

void HandEntity::AddToBottom( CardEntity* Input, bool bMoveSprite, std::function< void() > Callback )
{
    if( Input )
    {
        Cards.push_back( Input );
        InvalidateCards( Input );
        ICardContainer::SetCardContainer( Input );
        
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

void HandEntity::AddAtRandom( CardEntity* Input, bool bMoveSprite, std::function< void() > Callback )
{
    if( Input )
    {
        // Generate random index
        auto Iter = Cards.begin();
        std::advance( Iter, cocos2d::random< int >( 0, (int) Cards.size() ) );
        
        auto It = Cards.insert( Iter, Input );
        InvalidateCards( Input );
        ICardContainer::SetCardContainer( Input );
        
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

void HandEntity::AddAtIndex( CardEntity* Input, uint32 Index, bool bMoveSprite, std::function< void() > Callback )
{
    if( Input )
    {
        // TODO: Evalulate if this assert is needed
        CC_ASSERT( Index <= Cards.size() );
        
        auto It = Cards.begin();
        std::advance( It, Index );
        
        Cards.insert( It, Input );
        InvalidateCards( Input );
        ICardContainer::SetCardContainer( Input );
        
        if( bMoveSprite )
        {
            InvalidateZOrder();
            MoveCard( Input, CalcPos( Index ), Callback );
        }
        else if( Callback )
            Callback();
    }
}

bool HandEntity::Remove( CardEntity* inCard, bool bDestroy )
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
    InvalidateCards();
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
    InvalidateCards();
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
    InvalidateCards();
    InvalidateZOrder();
    
    return true;
}

bool HandEntity::RemoveRandom( bool bDestroy /* = false */ )
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

void HandEntity::InvalidateCards( CardEntity* Ignore, bool bExpanding )
{
    int Index = 0;
    
    // If were ignoring a card and dont want to fill the place, we need to subtract one
    // from total card count in CalcPos
    for( auto It = Cards.begin(); It != Cards.end(); It++ )
    {
        if( (*It) && *It != Ignore && !(*It)->GetIsDragging() )
        {
            // If were expanding, then were going to increate the animation speed
            (*It)->MoveAnimation( CalcPos( Index, 0 ), 0.5f, nullptr );
        }
        
        Index++;
    }
}

void HandEntity::Invalidate()
{
    EntityBase::Invalidate();
    InvalidateCards();
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

void HandEntity::MoveCard( CardEntity* inCard, const cocos2d::Vec2& inPos, std::function< void() > Callback )
{
    if( inCard )
    {
        // Move card
        inCard->MoveAnimation( inPos, 0.5f, Callback );
        
        // Flip face up if it isnt already
        if( bVisibleLocally && !inCard->IsFaceUp() )
            inCard->Flip( true, 0.5f );
        
        // If were flipped upside down, then ensure the card is also upside down
        auto rot = GetAbsoluteRotation();
        if( rot > 90.f || rot < -90.f )
        {
            float cardRot = inCard->GetAbsoluteRotation();
            if( cardRot < 1.f || cardRot > -1.f )
            {
                inCard->RotateAnimation( GetAbsoluteRotation(), 0.5f );
            }
        }
    }
}

void HandEntity::InvalidateZOrder()
{
    // Top of deck gets a higher Z Order
    // So, we need to order the cards
    // Default card Z order is 10
    
    int Top = ( (int)Cards.size() * 2 ) + 300;
    
    for( int i = 0; i < Cards.size(); i++ )
    {
        int Order = Top - ( i * 2 );
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
    InvalidateCards( nullptr, true );
}

void HandEntity::OpenBlitzMode()
{
    if( bBlitzMode )
        return;
    
    bBlitzMode = true;
    
    // Reset State
    SelectedCards.clear();
    SelectedMana = 0;

    // Create Blitz UI
    Selector = CardSelector::Create( Begin(), End() );
    Selector->SetSelectionChanged( [=] ( std::vector< Game::CardEntity* > In ) -> void
    {
        // Update Consumed Mana and store currently selected cards
        SelectedCards   = In;
        SelectedMana    = 0;
        
        for( auto It = SelectedCards.begin(); It != SelectedCards.end(); It++ )
        {
            if( *It )
            {
                SelectedMana += (*It)->ManaCost;
            }
        }
        
    } );
    
    Selector->SetSelectCheck( [=]( Game::CardEntity* In ) -> bool
    {
        // Check if this card is able to be selected
        if( !In )
        {
            return false;
        }
        
        auto Pl = dynamic_cast< Game::Player* >( GetOwner() );
        if( !Pl )
        {
            return false;
        }

        if( SelectedMana + In->ManaCost > Pl->GetMana() )
        {
            return false;
        }
        
        return true;
    } );
    
    Selector->SetConfirm( std::bind( &HandEntity::ConfirmBlitz, this ) );
    Selector->setOpacity( 0 );
    
    GetScene()->addChild( Selector );
    
    // Fade out cards
    for( auto It = Begin(); It != End(); It++ )
    {
        if( *It && (*It)->Sprite )
        {
            (*It)->Sprite->runAction( cocos2d::FadeOut::create( 0.25f ) );
        }
    }
    
    // Fade in selector
    Selector->runAction( cocos2d::Sequence::create( cocos2d::FadeIn::create( 0.25f ), NULL ) );
}

void HandEntity::ConfirmBlitz()
{
    if( bBlitzMode )
    {
        // Pass along our selection
        auto world = Game::World::GetWorld();
        auto Auth = world ? world->GetAuthority() : nullptr;
        
        if( Auth )
        {
            if( Selector )
                Selector->Lock();
            
            Auth->SetBlitzCards( SelectedCards );
        }
    }
}

void HandEntity::DeselectBlitz( Game::CardEntity* In )
{
    if( Selector )
    {
        if( In == nullptr )
            Selector->DeselectAll();
        else
            Selector->Deselect( In );
    }
}

void HandEntity::EnabledBlitzMenu()
{
    if( Selector )
        Selector->UnLock();
}

void HandEntity::CloseBlitzMode()
{
    if( !bBlitzMode )
        return;
    
    bBlitzMode = false;
    
    // Reset State
    SelectedMana = 0;
    SelectedCards.clear();
    
    // Fade Cards Back In
    for( auto It = Begin(); It != End(); It++ )
    {
        if( (*It) && (*It)->Sprite )
        {
            (*It)->Sprite->runAction( cocos2d::FadeIn::create( 0.25f ) );
        }
    }
    
    if( Selector )
    {
        // Fade out selector, and remove
        Selector->runAction( cocos2d::Sequence::create(
                   cocos2d::FadeOut::create( 0.25f ),
                   cocos2d::RemoveSelf::create(), NULL ) );
    }
    
    Selector = nullptr;
}

bool HandEntity::AttemptDrop( CardEntity *inCard, const cocos2d::Vec2 &inPos )
{
    if( !inCard )
        return false;
    
    // Determine, if this card was added, a list of points, and find the one which is
    // closest to the drop position, and have a minimum required distance
    auto dir = cocos2d::Director::getInstance();
    auto size = dir->getVisibleSize();
    
    // We also need to determine if this card is coming from a different container (shouldnt right now)
    int CardCount = (int) Count();
    
    auto cont = inCard->GetContainer();
    if( !cont || cont->GetTag() != TAG_HAND )
        CardCount++;
    
    int BestIndex       = -1;
    float IndexDist     = size.width / 16.f;
    cocos2d::Vec2 Abs   = GetOwner() ? GetOwner()->GetAbsolutePosition() : cocos2d::Vec2::ZERO;
    
    for( int i = 0; i < CardCount; i++ )
    {
        float Dist = ( CalcPos( i ) + Abs ).getDistance( inPos );
        
        if( Dist < IndexDist )
        {
            BestIndex = i;
            IndexDist = Dist;
        }
    }
    
    // Check if anything was found
    if( BestIndex < 0 )
        return false;
    
    // Move the card
    if( cont )
    {
        if( cont->GetTag() == TAG_HAND )
        {
            // This means were just re-arranging the card, so we need to
            // move the card in the list manually
            bool bFound = false;
            for( auto It = Cards.begin(); It != Cards.end(); It++ )
            {
                if( *It && *It == inCard )
                {
                    Cards.erase( It );
                    bFound = true;
                    break;
                }
            }
            
            // If the card didnt end up being in the list, then somethings wrong
            if( !bFound )
            {
                cocos2d::log( "[UI] WARNING: Hand re-arrange failed.. couldnt find specified card in list" );
                return false;
            }
        }
        else
            cont->Remove( inCard );
    }
    
    AddAtIndex( inCard, BestIndex );
    return true;
}
