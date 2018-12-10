//
//    DeckEntity.hpp
//    Regicide Mobile
//
//    Created: 11/10/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#pragma once

#include "EntityBase.hpp"
#include "CardEntity.hpp"
#include "ICardContainer.hpp"
#include "cocos2d.h"

namespace Game
{
    class DeckEntity : public EntityBase, public ICardContainer
    {
        
    public:
        
        DeckEntity();
        ~DeckEntity();
        
        // EntityBase Implementation
        virtual void Cleanup() override;
        
        
        inline std::string GetDispayName() const { return DisplayName; }
        
        // Adding/Getting Cards
        CardEntity* DrawCard();
        virtual void AddToBottom( CardEntity* Input, bool bMoveSprite = true, std::function< void() > Callback = nullptr ) override;
        virtual void AddToTop( CardEntity* Input, bool bMoveSprite = true, std::function< void() > Callback = nullptr ) override;
        virtual void AddAtRandom( CardEntity* Input, bool bMoveSprite = true, std::function< void() > Callback = nullptr ) override;
        virtual void AddAtIndex( CardEntity* Input, uint32 Index, bool bMoveSprite = true, std::function< void() > Callback = nullptr ) override;
        
        // Discarding Cards
        virtual bool RemoveTop( bool bDestroy = false ) override;
        virtual bool RemoveBottom( bool bDestroy = false ) override;
        virtual bool RemoveAtIndex( uint32 Index, bool bDestroy = false ) override;
        virtual bool RemoveRandom( bool bDestroy = false ) override;
        virtual bool Remove( CardEntity* inCard, bool bDestroy = false ) override;
        
        // Iterator Access
        inline CardIter Begin()     { return Cards.begin(); }
        inline CardIter End()       { return Cards.end(); }
        
        // Other Accessors
        bool IndexValid( uint32 Index ) const override;
        CardEntity* operator[]( uint32 Index ) override;
        CardEntity* At( uint32 Index ) override;
        inline size_t Count() const override { return Cards.size(); }
        
        inline uint32 GetDeckId() const { return DeckId; }
        
        virtual void Invalidate() override;
        virtual void InvalidateCards( CardEntity* Ignore = nullptr ) override;
        void InvalidateZOrder();
        
        virtual void AddToScene( cocos2d::Node* In ) override;
        
        inline std::string GetName() const { return DisplayName; }
        void Clear();
        
    protected:
        
        std::deque< CardEntity* > Cards;
        std::string DisplayName;
        uint32 DeckId;
        
        void MoveCard( CardEntity* inCard, std::function< void() > Callback );
        
        cocos2d::Label* Counter;
        bool bAddedToScene;
        
        void UpdateCounter();
        
        friend class SingleplayerLauncher;
        
    };
}
