//
//	SpriteEntity.hpp
//	Regicide Mobile
//
//	Created: 11/21/18
//	Updated: 11/21/18
//
//	© 2018 Zachary Berry, All Rights Reserved
//

#pragma once

#include "EntityBase.hpp"
#include "cocos2d.h"


namespace Game
{
    
    struct SpriteOverlay
    {
        std::string TextureName;
        cocos2d::Texture2D* Texture;
        cocos2d::Sprite* Sprite;
        int LocalZ;
    };
    
    class SpriteEntity : public EntityBase
    {
        
    public:
        
        SpriteEntity() = delete;
        SpriteEntity( const std::string& inName, const std::string& baseTexture );
        ~SpriteEntity();
        
        virtual void AddToScene( cocos2d::Node* inNode ) override;
        virtual void Invalidate() override;
        
        bool ShowOverlay( const std::string& inId );
        bool HideOverlay( const std::string& inId, bool bDelete = true );
        bool IsOverlayVisible( const std::string& inId );
        
        int GetZOrder() const;
        void SetZOrder( int inZ );
        
    protected:
        
        virtual void Cleanup() override;
        virtual int LoadResources( const std::function< void() >& Callback ) override;
        
        void PreloadOverlay( const std::string& inId, const std::string& inTexture, int LocalZ );
        
        std::string BaseTextureName;
        cocos2d::Texture2D* BaseTexture;
        
        cocos2d::Sprite* Sprite;
        int ZOrder;
        
    private:
        
        std::map< std::string, SpriteOverlay > Overlays;
        
    };
    
}
