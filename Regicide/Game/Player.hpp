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
        inline std::string GetName() { return State.DisplayName; }
        
        std::vector< CardEntity* > GetAllCards();
        CardEntity* PerformTouchTrace( const cocos2d::Vec2& inPos );
        
        inline int GetMana() const { return State.Mana; }
        void SetMana( int In );
        
        inline int GetHealth() const { return State.Health; }
        void SetHealth( int In );
        
        inline bool IsOpponent() const { return bOpponent; }
        
        inline void SetTurn( bool bIn ) { _bIsTurn = bIn; }
        inline bool IsTurn() const { return _bIsTurn; }
        
        // Custom Card Back Textures
        inline std::string GetBackTexture() { return CardBackTexture; }
        inline PlayerState GetState() const { return State; }
        
        // State
        std::string DisplayName;
        int Mana;
        int Health;
        
    protected:
        
        // Card Containers
        DeckEntity* Deck;
        HandEntity* Hand;
        FieldEntity* Field;
        GraveyardEntity* Graveyard;
        KingEntity* King;
        
        std::string CardBackTexture;
        bool bOpponent;
        
        // Actions
        void Action_PlayCard( Action* In, std::function< void() > Callback );
        void Action_UpdateMana( Action* In, std::function< void() > Callback );
        void Action_DrawCard( Action* In, std::function< void() > Callback );
        void Action_KingDamage( Action* In, std::function< void() > Callback );

        uint32_t lastDrawId = 0;
        
        PlayerState State;
        
    private:
        
        CardEntity* _Impl_TraceTouch( std::deque< CardEntity* >::iterator Begin, std::deque< CardEntity* >::iterator End, const cocos2d::Vec2& inPos );
        
        bool _bIsTurn = false;
        
        // Friend the State
        friend class ClientState;
        
    };
    
}
