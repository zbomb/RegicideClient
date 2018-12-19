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
#include "Actions.hpp"
#include "ObjectStates.hpp"

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

// Default Move Time
// Time it takes for a card to move between two points, the most elegant looking method is
// to have a fixed time, as opposed to a fixed speed
#define CARD_DEFAULT_MOVE_TIME 0.35f


namespace Game
{
    
    // Forward Class Declarations
    class Player;
    class ICardContainer;
    
    // Ability Declaration
    struct Ability
    {
        std::shared_ptr< luabridge::LuaRef > MainFunc;
        std::shared_ptr< luabridge::LuaRef > CheckFunc;
        std::shared_ptr< luabridge::LuaRef > SimulateFunc;
        
        std::string Name;
        std::string Description;
        
        uint16_t ManaCost;
        uint16_t StaminaCost;
        
        uint8_t Index;
    };
    
    // Dynamic State, all static attributes are stored in a static table, and lookups are performed when needed

    
    struct CardInfo
    {
        // Name/Description
        std::string DisplayName;
        std::string Description;
        
        uint16_t Id;
        
        // Passive and active abilities
        std::shared_ptr< luabridge::LuaRef > Hooks;
        std::map< uint32_t, Ability > Abilities;
        
        // Texture Info
        std::string FrontTexture;
        std::string FullTexture;
        
        // Starting State
        int Power;
        int Stamina;
        int ManaCost;
        
    };
    
    class CardEntity;
    
    class CardManager
    {
    public:
        
        static CardManager& GetInstance();
        
        bool GetInfo( uint16_t inId, CardInfo& Out );
        bool InfoLoaded( uint16_t inId );
        CardInfo* GetInfoAddress( uint16_t );
        
        CardEntity* CreateCard( uint16_t inId, Player* inOwner, bool bPreloadTextures = false );
        CardEntity* CreateCard( CardState& State, Player* inOwner, bool bPreloadTextures = false );
        
        ~CardManager();
        
    protected:
        
        std::map< uint16_t, CardInfo > CachedCards;
        
    private:
        
        CardManager() {}
        CardManager( const CardManager& Other ) = delete;
        CardManager& operator= ( const CardManager& Other ) = delete;
    };
    
    // Class Declaration
    class CardEntity : public EntityBase
    {
        
    public:
        
        // Constructor/Destructor
        CardEntity();
        ~CardEntity();
        
        // Public Methods
        void MoveAnimation( const cocos2d::Vec2& To, float Time );
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
        inline int GetZ() const                 { if( Sprite ) return Sprite->getGlobalZOrder(); return 0; }
        void SetZ( int In ); 
        inline float GetWidth() const           { if( Sprite ) { return Sprite->getContentSize().width * Sprite->getScaleX(); } else return 0.f; }
        inline void SetIsDragging( bool In )    { _bDragging = In; }
        inline bool GetIsDragging() const       { return _bDragging; }
        inline ICardContainer* GetContainer()   { return Container; }
        
        // EntityBase Overrides
        virtual void AddToScene( cocos2d::Node* inNode ) override;
        virtual void Invalidate() override;
        
        void CreateOverlays();
        void DestroyOverlays();
        
        void UpdatePower( int inPower );
        void UpdateStamina( int inStamina );
        
        void ShowPowerStamina();
        void HidePowerStamina();

        // Getters for Lua
        int _lua_GetCardId() const { return Id; }
        int _lua_GetPower() const { return Power; }
        int _lua_GetStamina() const { return Stamina; }
        int _lua_GetManaCost() const { return ManaCost; }
        std::string _lua_GetName() const { return ""; }
        bool _lua_IsFaceUp() const { return FaceUp; }
        
        bool bAttacking;
        
        cocos2d::Sprite* Sprite;
        cocos2d::Sprite* Highlight;
        cocos2d::Sprite* Overlay;
        cocos2d::Label* PowerLabel;
        cocos2d::Label* StaminaLabel;
        
        cocos2d::Texture2D* FrontTexture;
        cocos2d::Texture2D* BackTexture;
        cocos2d::Texture2D* FullSizedTexture;
        
        inline CardInfo* GetInfo() { return Info; }
        
        uint16_t Id;
        int Power;
        int Stamina;
        int ManaCost;
        bool FaceUp;
        CardPos Pos;
        uint32_t Owner;

    protected:
        
        // EntityBase Overrides (Protected)
        virtual void Cleanup() override;
        
        // Protected Members
        Player* OwningPlayer;
        ICardContainer* Container;
        
        bool bSceneInit;
        bool _bDragging;
        
        uint32_t lastMoveId;
        
        CardInfo* Info;
        
        // Fiend Class Declarations
        friend class ICardContainer;
        friend class CardManager;
    };
    
    // Card Container Iterator Typedef
    typedef std::deque< CardEntity* >::iterator CardIter;
    

    
}
