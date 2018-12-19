//
//	ObjectStates.h
//	Regicide Mobile
//
//	Created: 12/7/18
//	Updated: 12/7/18
//
//	© 2018 Zachary Berry, All Rights Reserved
//

#pragma once
#include "LuaEngine.hpp"

namespace Game
{
    // Enumerators
    enum class CardPos
    {
        DECK = 0,
        HAND = 1,
        FIELD = 2,
        KING = 3,
        GRAVEYARD = 4,
        NONE = 5
    };
    
    struct CardState
    {
        uint16_t Id;
        uint32_t EntId;
        int Power;
        int Stamina;
        int ManaCost;
        bool FaceUp;
        CardPos Position;
        uint32_t Owner;
        
        // Pretty Lua Functions
        int _lua_GetPosition() const { return (int) Position; }
        uint32_t _lua_GetOwner() const { return Owner; }
        int _lua_GetPower() const { return Power; }
        int _lua_GetStamina() const { return Stamina; }
        bool _lua_OnField() const { return Position == CardPos::FIELD; }
        bool _lua_InHand() const { return Position == CardPos::HAND; }
        bool _lua_InDeck() const { return Position == CardPos::DECK; }
        bool _lua_InGrave() const { return Position == CardPos::GRAVEYARD; }
        int _lua_GetManaCost() const { return ManaCost; }
        uint32_t _lua_GetEntityId() const { return EntId; }
        uint16_t _lua_GetCardId() const { return Id; }
        bool _lua_IsFaceUp() const { return FaceUp; }
        
        CardState& operator=( const CardState& Other )
        {
            Id          = Other.Id;
            EntId       = Other.EntId;
            Power       = Other.Power;
            Stamina     = Other.Stamina;
            ManaCost    = Other.ManaCost;
            FaceUp      = Other.FaceUp;
            Position    = Other.Position;
            Owner       = Other.Owner;
            
            return *this;
        }
    };
    
    struct KingState
    {
        uint16_t Id;
        uint32_t Owner;
        
        std::string DisplayName;
        std::string PlayerTexture;
        std::string OpponentTexture;
        
        std::shared_ptr< luabridge::LuaRef > Hooks;
        
        KingState& operator=( const KingState& Other )
        {
            Id          = Other.Id;
            Owner       = Other.Owner;
            DisplayName = Other.DisplayName;
            Hooks       = Other.Hooks;
            
            return *this;
        }
    };
    
    struct PlayerState
    {
        std::string DisplayName;
        uint32_t EntId;
        int Mana;
        int Health;
        
        std::vector< CardState > Deck;
        std::vector< CardState > Hand;
        std::vector< CardState > Field;
        std::vector< CardState > Graveyard;
        
        KingState King;
        
    private:
        
        luabridge::LuaRef _lua_BuildTable( std::vector< CardState >::iterator Begin, std::vector< CardState >::iterator End )
        {
            auto Engine = Regicide::LuaEngine::GetInstance();
            auto L = Engine ? Engine->State() : nullptr;
            
            CC_ASSERT( L );
            
            auto Output = luabridge::newTable( L );
            
            int Index = 1;
            for( auto It = Begin; It != End; It++ )
                Output[ Index ] = std::addressof( *It );
            
            return Output;
        }
        
    public:
        
        luabridge::LuaRef _lua_GetDeck()
        {
            return _lua_BuildTable( Deck.begin(), Deck.end() );
        }
        
        luabridge::LuaRef _lua_GetHand()
        {
            return _lua_BuildTable( Hand.begin(), Hand.end() );
        }
        
        luabridge::LuaRef _lua_GetField()
        {
            return _lua_BuildTable( Field.begin(), Field.end() );
        }
        
        luabridge::LuaRef _lua_GetGraveyard()
        {
            return _lua_BuildTable( Graveyard.begin(), Graveyard.end() );
        }
        
        PlayerState& operator=( const PlayerState& Other )
        {
            DisplayName = Other.DisplayName;
            EntId       = Other.EntId;
            Health      = Other.Health;
            Mana        = Other.Mana;
            Deck        = Other.Deck;
            Hand        = Other.Hand;
            Field       = Other.Field;
            Graveyard   = Other.Graveyard;
            King        = Other.King;
            
            return *this;
        }
    };
    
    

};
