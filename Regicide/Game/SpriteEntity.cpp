//
//	SpriteEntity.cpp
//	Regicide Mobile
//
//	Created: 11/21/18
//	Updated: 11/21/18
//
//	© 2018 Zachary Berry, All Rights Reserved
//

#include "SpriteEntity.hpp"

using namespace Game;



SpriteEntity::SpriteEntity( const std::string& inName, const std::string& baseTexture )
: EntityBase( inName ), BaseTextureName( baseTexture )
{
    BaseTexture     = nullptr;
    Sprite          = nullptr;
    ZOrder          = 1;
}

SpriteEntity::~SpriteEntity()
{
    BaseTexture = nullptr;
    Sprite      = nullptr;
}


void SpriteEntity::AddToScene( cocos2d::Node *inNode )
{
    EntityBase::AddToScene( inNode );
    
    // Create The Sprite
    if( Sprite )
        Sprite->removeFromParent();
    
    if( BaseTexture )
    {
        Sprite = cocos2d::Sprite::createWithTexture( BaseTexture );
        Sprite->setAnchorPoint( cocos2d::Vec2( 0.5f, 0.5f ) );
        Sprite->setName( "BaseSprite" );
        
        inNode->addChild( Sprite );
    }
    else
    {
        cocos2d::log( "[SpriteEntity] Failed to create Cocos Sprite! Base texture was not loaded! (%s)", BaseTextureName.c_str() );
    }
    
}

void SpriteEntity::Invalidate()
{
    EntityBase::Invalidate();
    
    if( Sprite )
    {
        Sprite->setPosition( GetAbsolutePosition() );
        Sprite->setRotation( GetAbsoluteRotation() );
    }
}

void SpriteEntity::Cleanup()
{
    EntityBase::Cleanup();
    
    // Destroy Sprite
    if( Sprite )
    {
        Sprite->removeFromParent();
    }
    
    Sprite = nullptr;
}

int SpriteEntity::LoadResources( const std::function<void ()> &Callback )
{
    if( BaseTextureName.empty() )
    {
        cocos2d::log( "[SpriteEntity] Warning: No texture set for entity! Entity Name: %s", GetEntityName().c_str() );
        return 0;
    }
    
    auto Cache = cocos2d::Director::getInstance()->getTextureCache();
    if( !Cache )
    {
        cocos2d::log( "[SpriteEntity] ERROR: Couldnt pre-load textures.. Texture Cache was null!" );
        return 0;
    }
    
    int TextureCount = 1;
    Cache->addImageAsync( BaseTextureName, [ = ]( cocos2d::Texture2D* loadedTexture ) -> void
    {
        if( !loadedTexture )
        {
            cocos2d::log( "[SpriteEntity] Warning: Failed to load base texture for '%s'.. Texture Name: %s", this->GetEntityName().c_str(), this->BaseTextureName.c_str() );
            this->BaseTexture = nullptr;
        }
        else
        {
            this->BaseTexture = loadedTexture;
        }
        
        Callback();
    } );
    
    // Load Overlay Textures
    for( auto It = Overlays.begin(); It != Overlays.end(); It++ )
    {
        if( !It->second.TextureName.empty() )
        {
            Cache->addImageAsync( It->second.TextureName, [ = ]( cocos2d::Texture2D* loadedTexture )
            {
                if( !loadedTexture )
                {
                    cocos2d::log( "[SpriteEntity] Warning: Failed to load overlay texture for '%s'.. Texture Name: %s", this->GetEntityName().c_str(), It->second.TextureName.c_str() );
                    It->second.Texture = nullptr;
                }
                else
                {
                    It->second.Texture = loadedTexture;
                }
                
                Callback();
            } );
            
            TextureCount++;
        }
    }
    
    return TextureCount;
}

bool SpriteEntity::ShowOverlay( const std::string &inId )
{
    // Check if this overlay exists, and the texture is set
    if( Overlays.count( inId ) <= 0 )
    {
        cocos2d::log( "[SpriteEntity] Failed to add overlay to sprite entity '%s'! Overlay Id: %s", GetEntityName().c_str(), inId.c_str() );
        return false;
    }
    
    auto& Overlay = Overlays[ inId ];
    if( Overlay.TextureName.empty() )
    {
        cocos2d::log( "[SpriteEntity] Failed to add overlay to sprite entity '%s' because the texture isnt set. Overlay Id: %s", GetEntityName().c_str(), inId.c_str() );
        return false;
    }
    
    // If the texture wasnt already loaded, then we will manually load it
    if( !Overlay.Texture )
    {
        auto Cache = cocos2d::Director::getInstance()->getTextureCache();
        if( !Cache )
        {
            cocos2d::log( "[SpriteEntity] Failed to load texture (%s) for sprite entity '%s' because the texture cache was null", Overlay.TextureName.c_str(), GetEntityName().c_str() );
            return false;
        }
        
        Overlay.Texture = Cache->addImage( Overlay.TextureName );
        if( !Overlay.Texture )
        {
            cocos2d::log( "[SpriteEntity] Failed to load texture (%s) for sprite entity '%s' because the texture wasnt found", Overlay.TextureName.c_str(), GetEntityName().c_str() );
            return false;
        }
    }
    
    // If the sprite already exists, then we will remove it
    if( Overlay.Sprite )
    {
        Overlay.Sprite->setOpacity( 255 );
    }
    else
    {
        // Create the new sprite
        Overlay.Sprite = cocos2d::Sprite::createWithTexture( Overlay.Texture );
        Overlay.Sprite->setAnchorPoint( cocos2d::Vec2( 0.5f, 0.5f ) );
        Overlay.Sprite->setPosition( Sprite->getContentSize() * 0.5f );
        Overlay.Sprite->setGlobalZOrder( Sprite->getGlobalZOrder() + Overlay.LocalZ );
        
        Sprite->addChild( Overlay.Sprite );
    }
    
    return true;
}

int SpriteEntity::GetZOrder() const
{
    return ZOrder;
}

void SpriteEntity::SetZOrder( int inZ )
{
    ZOrder = inZ;
    
    // Update Base Sprite Z Order
    if( Sprite )
    {
        Sprite->setGlobalZOrder( inZ );
    }
    
    // Update Overlay Z Orders
    for( auto It = Overlays.begin(); It != Overlays.end(); It++ )
    {
        if( It->second.Sprite )
        {
            It->second.Sprite->setGlobalZOrder( inZ + It->second.LocalZ );
        }
    }
}

void SpriteEntity::PreloadOverlay( const std::string &inId, const std::string &inTexture, int LocalZ )
{
    if( Overlays.count( inId ) > 0 )
    {
        cocos2d::log( "[SpriteEntity] Warning: Duplicate overlay (%s) detected for entity '%s'", inId.c_str(), GetEntityName().c_str() );
        return;
    }
    
    Overlays[ inId ] = SpriteOverlay();
    
    Overlays[ inId ].TextureName    = inTexture;
    Overlays[ inId ].Texture        = nullptr;
    Overlays[ inId ].Sprite         = nullptr;
    Overlays[ inId ].LocalZ         = LocalZ;
}

bool SpriteEntity::HideOverlay( const std::string &inId, bool bDelete )
{
    if( Overlays.count( inId ) <= 0 )
        return false;
    
    auto& Overlay = Overlays[ inId ];
    
    if( Overlay.Sprite )
    {
        if( bDelete )
        {
            Overlay.Sprite->removeFromParent();
            Overlay.Sprite = nullptr;
        }
        else
        {
            Overlay.Sprite->setOpacity( 0 );
        }
    }
    
    return true;
}

bool SpriteEntity::IsOverlayVisible( const std::string &inId )
{
    if( Overlays.count( inId ) <= 0 )
        return false;
    
    auto& Overlay = Overlays[ inId ];
    if( Overlay.Sprite )
    {
        return Overlay.Sprite->getOpacity() > 0;
    }
    
    return false;
}
