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
        
        int _lua_GetPosition() const { return (int) Position; }
    };
    
    struct KingState
    {
        uint16_t Id;
        uint32_t Owner;
        
        std::string DisplayName;
        std::string PlayerTexture;
        std::string OpponentTexture;
        
        std::shared_ptr< luabridge::LuaRef > Hooks;
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
    };
    
    

};
