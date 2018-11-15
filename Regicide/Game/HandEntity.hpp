//
//  HandEntity.hpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/11/18.
//

#pragma once

#include "EntityBase.hpp"
#include "ICardContainer.hpp"

namespace Game
{
    class CardEntity;
    typedef std::deque< CardEntity* >::iterator HandIter;
    
    class HandEntity : public EntityBase, public ICardContainer
    {
        
    public:
        
        HandEntity();
        ~HandEntity();
        
        virtual void Cleanup() override;
        
        virtual void AddToBottom( CardEntity* Input, bool bMoveSprite = true ) override;
        virtual void AddToTop( CardEntity* Input, bool bMoveSprite = true ) override;
        virtual void AddAtRandom( CardEntity* Input, bool bMoveSprite = true ) override;
        virtual void AddAtIndex( CardEntity* Input, uint32 Index, bool bMoveSprite = true ) override;
        
        // Discarding Cards
        virtual bool RemoveTop( bool bDestroy = false ) override;
        virtual bool RemoveBottom( bool bDestroy = false ) override;
        virtual bool RemoveAtIndex( uint32 Index, bool bDestroy = false ) override;
        virtual bool RemoveRandom( bool bDestroy = false ) override;
        virtual bool Remove( CardEntity* inCard, bool bDestroy = false ) override;
        
        // Iterator Access
        inline HandIter Begin()     { return Cards.begin(); }
        inline HandIter End()       { return Cards.end(); }
        
        // Other Accessors
        bool IndexValid( uint32 Index ) const override;
        CardEntity* operator[]( uint32 Index ) override;
        CardEntity* At( uint32 Index ) override;
        inline size_t Count() const override { return Cards.size(); }
        
        virtual void Invalidate() override;
        void InvalidateZOrder();
        
        inline bool IsVisibleLocally() const { return bVisibleLocally; }
        inline bool IsExpanded() const { return bExpanded; }
        
        void SetExpanded( bool bExpand );
        virtual void InvalidateCards( CardEntity* Ignore = nullptr, bool bExpanding = false ) override;
        
        bool AttemptDrop( CardEntity* inCard, const cocos2d::Vec2& inPos );
        
    protected:
        
        std::deque< CardEntity* > Cards;
        bool bVisibleLocally;
        bool bExpanded;
        
        void MoveCard( CardEntity* inCard, const cocos2d::Vec2& AbsPos );
        cocos2d::Vec2 CalcPos( int Index, int CardDelta = 0 );
        
        
        friend class SingleplayerLauncher;
    };
}
