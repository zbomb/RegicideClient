//
//    KingEntity.cpp
//    Regicide Mobile
//
//    Created: 11/19/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#include "KingEntity.hpp"
#include "Player.hpp"

using namespace Game;


KingEntity::KingEntity()
: EntityBase( "king" ), Sprite( nullptr ), HealthLabel( nullptr ), ManaLabel( nullptr )
{
    bAddedToScene   = false;
    bIsOpponent     = false;
    
    Sprite          = nullptr;
    HealthLabel     = nullptr;
    ManaLabel       = nullptr;
    OwningPlayer    = nullptr;
    
    cocos2d::log( "[DEBUG] CREATING KING ENTITY" );
}

KingEntity::~KingEntity()
{
    // Free Game Resources
    if( Sprite )
    {
        Sprite->removeFromParent();
    }
    
    Sprite          = nullptr;
    HealthLabel     = nullptr;
    ManaLabel       = nullptr;
    OwningPlayer    = nullptr;
}

void KingEntity::Cleanup()
{
    EntityBase::Cleanup();
}

void KingEntity::AddToScene( cocos2d::Node *inNode )
{
    if( bAddedToScene )
        return;
    
    CC_ASSERT( inNode );
    bAddedToScene = true;
    
    Sprite = cocos2d::Sprite::createWithTexture( Texture );
    Sprite->setAnchorPoint( bIsOpponent ? cocos2d::Vec2( 1.f, 1.f ) : cocos2d::Vec2( 0.f, 0.f ) );
    Sprite->setName( "king" );
    
    HealthLabel = cocos2d::Label::createWithTTF( "", "fonts/arial.ttf", 45 );
    HealthLabel->setAnchorPoint( bIsOpponent ? cocos2d::Vec2( 1.f, 1.f ) : cocos2d::Vec2( 0.f, 0.f ) );
    HealthLabel->setTextColor( cocos2d::Color4B( 250, 30, 30, 255 ) );
    
    ManaLabel = cocos2d::Label::createWithTTF( "", "fonts/arial.ttf", 45 );
    ManaLabel->setAnchorPoint( bIsOpponent ? cocos2d::Vec2( 1.f, 1.f ) : cocos2d::Vec2( 0.f, 0.f ) );
    ManaLabel->setTextColor( cocos2d::Color4B( 40, 50, 250, 255 ) );
    
    auto Owner = GetOwningPlayer();
    if( Owner )
    {
        HealthLabel->setString( std::to_string( Owner->GetHealth() ) );
        ManaLabel->setString( std::to_string( Owner->GetMana() ) );
    }
    
    inNode->addChild( Sprite );
    Sprite->addChild( HealthLabel );
    Sprite->addChild( ManaLabel );
    
    if( bIsOpponent )
    {
        auto Size = Sprite->getContentSize() * Sprite->getScale();
        HealthLabel->setPosition( cocos2d::Vec2( Size.width - 10.f, Size.height - 10.f ) );
        ManaLabel->setPosition( cocos2d::Vec2( Size.width - 10.f - HealthLabel->getContentSize().width * 2.f - 10.f, Size.height - 10.f ) );
    }
    else
    {
        HealthLabel->setPosition( cocos2d::Vec2( 10.f, 10.f ) );
        ManaLabel->setPosition( cocos2d::Vec2( 10.f + HealthLabel->getContentSize().width * 2.f + 10.f, 10.f ) );
    }
}

void KingEntity::UpdateHealth( int inHealth )
{
    if( HealthLabel )
    {
        HealthLabel->setString( std::to_string( inHealth ) );
        if( bIsOpponent )
        {
            auto Size = Sprite->getContentSize() * Sprite->getScale();
            HealthLabel->setPosition( cocos2d::Vec2( Size.width - 10.f, Size.height - 10.f ) );
        }
        else
        {
            HealthLabel->setPosition( cocos2d::Vec2( 10.f, 10.f ) );
        }
    }
}

void KingEntity::Invalidate()
{
    EntityBase::Invalidate();
    
    if( Sprite )
    {
        Sprite->setPosition( GetAbsolutePosition() );
        Sprite->setRotation( GetAbsoluteRotation() );
        
        if( bIsOpponent )
        {
            auto Size = Sprite->getContentSize() * Sprite->getScale();
            HealthLabel->setPosition( cocos2d::Vec2( Size.width - 10.f, Size.height - 10.f ) );
        }
        else
        {
            HealthLabel->setPosition( cocos2d::Vec2( 10.f, 10.f ) );
        }
    }
}

void KingEntity::UpdateMana( int InMana )
{
    if( ManaLabel )
    {
        ManaLabel->setString( std::to_string( InMana ) );
    }
}

void KingEntity::Load( KingState& Source, bool bOpponent )
{
    bIsOpponent = bOpponent;
    
    DisplayName = Source.DisplayName;
    TextureName = bOpponent ? Source.OpponentTexture : Source.PlayerTexture;
    
    RequireTexture( TextureName, [ = ]( cocos2d::Texture2D* t )
    {
        if( t )
        {
            Texture = t;
        }
        else
        {
            cocos2d::log( "[King] Failed to load desired texture! '%s'", TextureName.c_str() );
        }
    } );
}
