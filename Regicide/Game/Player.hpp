//
//  Player.hpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/10/18.
//

#pragma once
#include "EntityBase.hpp"


namespace Game
{
    
    class Player : public EntityBase
    {
        
    public:
        
        Player();
        ~Player();
        virtual void Cleanup();
        
        
    };
    
}
