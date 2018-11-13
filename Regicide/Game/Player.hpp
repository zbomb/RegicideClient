//
//  Player.hpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/10/18.
//

#pragma once
#include "EntityBase.hpp"
#include "CardEntity.hpp"
#include "cocos2d.h"

namespace Game
{
    class DeckEntity;
    class HandEntity;
    class FieldEntity;
    class GraveyardEntity;
    class ICardContainer;
    
    class Player : public EntityBase
    {
        
    public:
        
        Player();
        ~Player();
        virtual void Cleanup();
        
        inline DeckEntity* GetDeck() { return Deck; }
        inline HandEntity* GetHand() { return Hand; }
        inline FieldEntity* GetField() { return Field; }
        inline GraveyardEntity* GetGraveyard() { return Graveyard; }
        inline std::string GetName() { return DisplayName; }
        
        std::vector< CardEntity* > GetAllCards();
        CardEntity* PerformTouchTrace( const cocos2d::Vec2& inPos );
        
    private:
        
        CardEntity* _Impl_TraceTouch( std::deque< CardEntity* >::iterator Begin, std::deque< CardEntity* >::iterator End, const cocos2d::Vec2& inPos );
        
    protected:
        
        // Card Containers
        DeckEntity* Deck;
        HandEntity* Hand;
        FieldEntity* Field;
        GraveyardEntity* Graveyard;
        
        // Traits
        std::string DisplayName;
        
        // Friend the launcher
        friend class SingleplayerLauncher;
        
        
    };
    
}
