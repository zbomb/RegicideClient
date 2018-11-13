//
//  FieldEntity.hpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/11/18.
//

#pragma once

#include "EntityBase.hpp"
#include "CardEntity.hpp"
#include "ICardContainer.hpp"

namespace Game
{
    typedef std::deque< CardEntity* >::iterator FieldIter;
    
    class FieldEntity : public EntityBase, public ICardContainer
    {
        
    public:
        
        FieldEntity();
        ~FieldEntity();
        
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
        
        // Iterator Access
        inline FieldIter Begin()     { return Cards.begin(); }
        inline FieldIter End()       { return Cards.end(); }
        
        // Other Accessors
        bool IndexValid( uint32 Index ) const override;
        CardEntity* operator[]( uint32 Index ) override;
        CardEntity* At( uint32 Index ) override;
        inline size_t Count() const override { return Cards.size(); }
        
        virtual void Invalidate() override;
        void InvalidateZOrder();
        
    protected:
        
        std::deque< CardEntity* > Cards;
        
        void MoveCard( CardEntity* inCard, const cocos2d::Vec2& AbsPos );
        cocos2d::Vec2 CalcPos( int Index, int CardDelta = 0 );
        void Reposition( CardEntity* Ignore, bool bFillIgnoredSpace = false );
        
        friend class SingleplayerLauncher;
    };
}
