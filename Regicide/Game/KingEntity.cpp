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

using namespace Game;


KingEntity::KingEntity()
: EntityBase( "king" ), Sprite( nullptr ), HealthLabel( nullptr )
{
    bAddedToScene = false;
    bIsOpponent = false;
}

KingEntity::~KingEntity()
{
    // Free Game Resources
    if( Sprite )
    {
        Sprite->removeFromParent();
    }
    
    Sprite = nullptr;
    HealthLabel = nullptr;
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
    
    HealthLabel = cocos2d::Label::createWithTTF( "30", "fonts/arial.ttf", 45 );
    HealthLabel->setAnchorPoint( bIsOpponent ? cocos2d::Vec2( 1.f, 1.f ) : cocos2d::Vec2( 0.f, 0.f ) );
    HealthLabel->setTextColor( cocos2d::Color4B( 0, 0, 0, 255 ) );
    
    inNode->addChild( Sprite );
    Sprite->addChild( HealthLabel );
    
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

bool KingEntity::Load( luabridge::LuaRef &inLua, Player *inOwner, bool bOpponent )
{
    bIsOpponent = bOpponent;
    
    // Validate Lua
    if( !inLua.isTable() )
    {
        cocos2d::log( "[King] ERROR: Failed to load.. invalid lua table!" );
        return false;
    }
    
    if( !inLua[ "Name" ].isString() ||
        ( bOpponent && !inLua[ "OpponentTexture" ].isString() ) ||
        ( !bOpponent && !inLua[ "PlayerTexture" ].isString() ))
    {
        cocos2d::log( "[King] ERROR: Failed to load.. invalid lua table!" );
        return false;
    }
    
    // Set Data Members
    DisplayName = inLua[ "Name" ].tostring();
    TextureName = bOpponent ? inLua[ "OpponentTexture" ].tostring() : inLua[ "PlayerTexture" ].tostring();
    
    RequireTexture( TextureName, [ = ]( cocos2d::Texture2D* InTex )
    {
        if( !InTex )
        {
            cocos2d::log( "[King] ERROR! Failed to load king texture '%s'", TextureName.c_str() );
            Texture = nullptr;
        }
        else
        {
            Texture = InTex;
        }
    } );
    
    // Store Hook Table
    if( inLua[ "Hooks" ].isTable() )
    {
        Hooks = std::make_shared< luabridge::LuaRef >( luabridge::newTable( inLua[ "Hooks" ] ) );
    }
    else
    {
        Hooks = std::make_shared< luabridge::LuaRef >( luabridge::newTable( inLua.state() ) );
    }
    
    return true;
}

bool KingEntity::GetHook( const std::string &Name, luabridge::LuaRef &Out )
{
    if( !Hooks || !Hooks->isTable() )
        return false;
    
    if( (*Hooks)[ Name ].isFunction() )
    {
        Out = (*Hooks)[ Name ];
        return true;
    }
    
    return false;
}
