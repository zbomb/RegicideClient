//
//    Player.hpp
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
#include "cocos2d.h"

namespace Game
{
    class DeckEntity;
    class HandEntity;
    class FieldEntity;
    class GraveyardEntity;
    class ICardContainer;
    class KingEntity;
    
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
        inline KingEntity* GetKing() { return King; }
        inline std::string GetName() { return DisplayName; }
        
        std::vector< CardEntity* > GetAllCards();
        CardEntity* PerformTouchTrace( const cocos2d::Vec2& inPos );
        
        inline int GetMana() const { return Mana; }
        void SetMana( int In ) { Mana = In; }
        
        inline int GetHealth() const { return Health; }
        void SetHealth( int In ) { Health = In; }
        
        inline bool IsOpponent() const { return bOpponent; }
        void InvalidatePossibleActions();
        
    private:
        
        CardEntity* _Impl_TraceTouch( std::deque< CardEntity* >::iterator Begin, std::deque< CardEntity* >::iterator End, const cocos2d::Vec2& inPos );
        
    protected:
        
        // Card Containers
        DeckEntity* Deck;
        HandEntity* Hand;
        FieldEntity* Field;
        GraveyardEntity* Graveyard;
        KingEntity* King;
        
        // Traits
        std::string DisplayName;
        
        int Mana;
        int Health;
        
        bool bOpponent;
        
        // Actions
        void Action_PlayCard( Action* In, std::function< void() > Callback );
        void Action_UpdateMana( Action* In, std::function< void() > Callback );
        void Action_DrawCard( Action* In, std::function< void() > Callback );
        
        // Friend the launcher
        friend class SingleplayerLauncher;
        
        
    };
    
}
