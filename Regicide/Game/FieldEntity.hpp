//
//    FieldEntity.hpp
//    Regicide Mobile
//
//    Created: 11/11/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#pragma once

#include "EntityBase.hpp"
#include "CardEntity.hpp"
#include "ICardContainer.hpp"

namespace Game
{
    class FieldEntity : public EntityBase, public ICardContainer
    {
        
    public:
        
        FieldEntity();
        ~FieldEntity();
        
        virtual void Cleanup() override;
        
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
        
        virtual void Invalidate() override;
        
        void InvalidateZOrder();
        virtual void InvalidateCards( CardEntity* Ignore = nullptr, bool bParam = false ) override;
        
        int AttemptDrop( CardEntity* inCard, const cocos2d::Vec2& inPos );
        void PauseInvalidate();
        void ResumeInvalidate();
        
    protected:
        
        std::deque< CardEntity* > Cards;
        
        void MoveCard( CardEntity* inCard, const cocos2d::Vec2& AbsPos, std::function< void() > Callback );
        cocos2d::Vec2 CalcPos( int Index, int CardDelta = 0 );
        
        bool _bInvalidatePaused = false;
        
        friend class SingleplayerLauncher;
    };
}
