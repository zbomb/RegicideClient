//
//  ICardContainer.hpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/12/18.
//

#pragma once

#include "CardEntity.hpp"

namespace Game
{
    class ICardContainer
    {
        
    public:
        
        virtual void AddToBottom( CardEntity* Input, bool bMoveSprite = true ) = 0;
        virtual void AddToTop( CardEntity* Input, bool bMoveSprite = true ) = 0;
        virtual void AddAtRandom( CardEntity* Input, bool bMoveSprite = true ) = 0;
        virtual void AddAtIndex( CardEntity* Input, uint32 Index, bool bMoveSprite = true ) = 0;
        
        // Discarding Cards
        virtual bool RemoveTop( bool bDestroy = false ) = 0;
        virtual bool RemoveBottom( bool bDestroy = false ) = 0;
        virtual bool RemoveAtIndex( uint32 Index, bool bDestroy = false ) = 0;
        virtual bool RemoveRandom( bool bDestroy = false ) = 0;
        
        // Other Accessors
        virtual bool IndexValid( uint32 Index ) const = 0;
        virtual CardEntity* operator[]( uint32 Index ) = 0;
        virtual CardEntity* At( uint32 Index ) = 0;
        virtual size_t Count() const = 0;
        
    protected:
        
        void SetCardContainer( CardEntity* inCard )
        {
            inCard->Container = this;
        }
        
        void ClearCardContainer( CardEntity* inCard )
        {
            if( inCard->Container == this )
            {
                inCard->Container = nullptr;
                inCard->SetZ( 10 );
            }
        }
        
    };
}
