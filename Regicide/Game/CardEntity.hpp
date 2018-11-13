//
//  CardEntity.hpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/10/18.
//

#pragma once

#include "EntityBase.hpp"
#include "cocos2d.h"
#include "LuaEngine.hpp"

#define ACTION_TAG_MOVE 1212
#define ACTION_TAG_ROTATE 2323

namespace Game
{
    enum class CardPos
    {
        DECK,
        HAND,
        FIELD,
        KING,
        GRAVEYARD,
        NONE
    };
    
    class Player;
    class ICardContainer;
    
    class CardEntity : public EntityBase
    {
        
    public:
        
        CardEntity();
        ~CardEntity();
        
        std::string DisplayName;
        uint16 CardId;
        
        uint16 Power;
        uint16 Stamina;
        
        bool bAllowDeckHooks;
        bool bAllowHandHooks;
        bool bAllowPlayHooks;
        bool bAllowDeadHooks;
        
        cocos2d::Sprite* Sprite;
        
        cocos2d::Texture2D* FrontTexture;
        cocos2d::Texture2D* BackTexture;
        std::shared_ptr< luabridge::LuaRef > Hooks;
        
        bool Load( luabridge::LuaRef& inLua, Player* inOwner, cocos2d::TextureCache* Cache, bool Authority = false );
        bool ShouldCallHook() const;
        bool ShouldCallHook( const std::string& HookName );
        bool GetHook( const std::string& HookName, luabridge::LuaRef& outFunc );
        
        virtual void AddToScene( cocos2d::Node* inNode ) override;
        
        inline Player* GetOwningPlayer() { return OwningPlayer; }
        inline ICardContainer* GetContainer() { return Container; }
        
        virtual void Invalidate() override;
        inline bool IsFaceUp() const { return bFaceUp; }
        
        void MoveAnimation( const cocos2d::Vec2& To, float Time );
        void RotateAnimation( float GlobalRot, float Time );
        void Flip( bool bFaceUp, float Time );
        
        inline int GetZ() const { if( Sprite ) return Sprite->getGlobalZOrder(); return 0; }
        inline void SetZ( int In ) const { if( Sprite ) Sprite->setGlobalZOrder( In ); }
        
        inline float GetWidth() const { if( Sprite ) { return Sprite->getContentSize().width * Sprite->getScaleX(); } else return 0.f; }
        
    protected:
        
        bool bSceneInit = false;
        virtual void Cleanup() override;

        Player* OwningPlayer;
        ICardContainer* Container;
        
        std::string FrontTextureName;
        std::string BackTextureName;
        
        bool bFaceUp;
        
        virtual int LoadResources( const std::function< void() >& Callback ) override;
        
        friend class ICardContainer;
        friend class SingleplayerLauncher;
    };
}
