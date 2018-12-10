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
        
        uint16_t King;
    };
    
    

};
