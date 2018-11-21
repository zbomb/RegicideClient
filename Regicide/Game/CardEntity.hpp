//
//    CardEntity.hpp
//    Regicide Mobile
//
//    Created: 11/10/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#pragma once

// Includes
#include "EntityBase.hpp"   // Base Class
#include "cocos2d.h"        // For Sprite, Texture2D & TextureCache
#include "LuaEngine.hpp"    // For luabridge::LuaRef

// Action Tags
// These are assigned to cocos2d 'actions' to be able to cancel the animation if needed
#define ACTION_TAG_MOVE 1212
#define ACTION_TAG_ROTATE 2323

// Play Errors
// Errors returned by the Game Authority when unable to play a card
#define PLAY_ERROR_INVALID 0
#define PLAY_ERROR_BADCARD 1
#define PLAY_ERROR_BADMANA 2
#define PLAY_ERROR_NOERROR 3


namespace Game
{
    // Enumerators
    enum class CardPos
    {
        DECK,
        HAND,
        FIELD,
        KING,
        GRAVEYARD,
        NONE
    };
    
    // Forward Class Declarations
    class Player;
    class ICardContainer;
    
    // Class Declaration
    class CardEntity : public EntityBase
    {
        
    public:
        
        // Constructor/Destructor
        CardEntity();
        ~CardEntity();
        
        // Public Methods
        bool Load( luabridge::LuaRef& inLua, Player* inOwner, cocos2d::TextureCache* Cache );
        bool ShouldCallHook() const;
        bool ShouldCallHook( const std::string& HookName );
        bool GetHook( const std::string& HookName, luabridge::LuaRef& outFunc );
        void MoveAnimation( const cocos2d::Vec2& To, float Time, std::function< void() > Callback = nullptr, bool bKillFX = true );
        void RotateAnimation( float GlobalRot, float Time );
        void Flip( bool bFaceUp, float Time );
        bool InDeck() const;
        bool InHand() const;
        bool InGrave() const;
        bool OnField() const;
        void SetHighlight( const cocos2d::Color3B& inColor = cocos2d::Color3B( 250, 20, 20 ), uint8_t Alpha = 200 );
        void ClearHighlight();
        void SetOverlay( const std::string& TextureName, uint8_t Alpha = 150 );
        void ClearOverlay();
        
        // Getters/Setters
        inline Player* GetOwningPlayer()        { return OwningPlayer; }
        inline bool IsFaceUp() const            { return bFaceUp; }
        inline int GetZ() const                 { if( Sprite ) return Sprite->getGlobalZOrder(); return 0; }
        inline void SetZ( int In ) const        { if( Sprite ) { Sprite->setGlobalZOrder( In ); Highlight->setGlobalZOrder( In + 1 ); Overlay->setGlobalZOrder( In + 1 ); } }
        inline float GetWidth() const           { if( Sprite ) { return Sprite->getContentSize().width * Sprite->getScaleX(); } else return 0.f; }
        inline void SetIsDragging( bool In )    { _bDragging = In; }
        inline bool GetIsDragging() const       { return _bDragging; }
        inline ICardContainer* GetContainer()   { return Container; }
        
        inline std::string GetFullSizedTextureName() const  { return LargeTextureName; }
        
        // EntityBase Overrides
        virtual void AddToScene( cocos2d::Node* inNode ) override;
        virtual void Invalidate() override;
        
        // Public Members
        std::string DisplayName;
        uint16 CardId;
        
        uint16 Power;
        uint16 Stamina;
        uint16 ManaCost;
        
        bool bAllowDeckHooks;
        bool bAllowHandHooks;
        bool bAllowPlayHooks;
        bool bAllowDeadHooks;
        
        bool bAttacking;
        
        cocos2d::Sprite* Sprite;
        cocos2d::Sprite* Highlight;
        cocos2d::Sprite* Overlay;
        
        cocos2d::Texture2D* FrontTexture;
        cocos2d::Texture2D* BackTexture;
        cocos2d::Texture2D* FullSizedTexture;
        std::shared_ptr< luabridge::LuaRef > Hooks;
        
    protected:
        
        // EntityBase Overrides (Protected)
        virtual void Cleanup() override;
        virtual int LoadResources( const std::function< void() >& Callback ) override;
        
        // Protected Members
        Player* OwningPlayer;
        ICardContainer* Container;
        
        std::string FrontTextureName;
        std::string BackTextureName;
        std::string LargeTextureName;
        
        bool bSceneInit;
        bool bFaceUp;
        bool _bDragging;
        
        // Fiend Class Declarations
        friend class ICardContainer;
        friend class SingleplayerLauncher;
    };
    
    // Card Container Iterator Typedef
    typedef std::deque< CardEntity* >::iterator CardIter;
}
