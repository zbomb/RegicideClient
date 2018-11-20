//
//    ICardContainer.hpp
//    Regicide Mobile
//
//    Created: 11/12/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#pragma once

#include "CardEntity.hpp"

#define TAG_DECK 10
#define TAG_FIELD 11
#define TAG_HAND 12
#define TAG_GRAVE 13
#define TAG_KING 14

namespace Game
{
    class ICardContainer
    {
        
    public:
        
        virtual void AddToBottom( CardEntity* Input, bool bMoveSprite = true, std::function< void() > Callback = nullptr ) = 0;
        virtual void AddToTop( CardEntity* Input, bool bMoveSprite = true, std::function< void() > Callback = nullptr ) = 0;
        virtual void AddAtRandom( CardEntity* Input, bool bMoveSprite = true, std::function< void() > Callback = nullptr ) = 0;
        virtual void AddAtIndex( CardEntity* Input, uint32 Index, bool bMoveSprite = true, std::function< void() > Callback = nullptr ) = 0;
        
        // Discarding Cards
        virtual bool RemoveTop( bool bDestroy = false ) = 0;
        virtual bool RemoveBottom( bool bDestroy = false ) = 0;
        virtual bool RemoveAtIndex( uint32 Index, bool bDestroy = false ) = 0;
        virtual bool RemoveRandom( bool bDestroy = false ) = 0;
        virtual bool Remove( CardEntity* Input, bool bDestroy = false ) = 0;
        
        // Other Accessors
        virtual bool IndexValid( uint32 Index ) const = 0;
        virtual CardEntity* operator[]( uint32 Index ) = 0;
        virtual CardEntity* At( uint32 Index ) = 0;
        virtual size_t Count() const = 0;
        
        inline int GetTag() const { return i_Tag; }
        
        virtual void InvalidateCards( CardEntity* IgnoredCard = nullptr, bool bParam = false ) = 0;
        
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
        
        void SetTag( int inTag )
        {
            i_Tag = inTag;
        }
        
    private:
        
        int i_Tag;
        
    };
}
