//
//  GameModeBase.hpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/10/18.
//

#pragma once
#include "EntityBase.hpp"
#include "Player.hpp"


namespace Game
{
    
    class GameModeBase : public EntityBase
    {
    public:
        
        GameModeBase();
        ~GameModeBase();
        
        virtual void Cleanup();
        virtual Player& GetLocalPlayer() = 0;
        virtual Player& GetOpponent() = 0;
    
    protected:
        
        virtual void Initialize() = 0;
        virtual void PostInitialize() = 0;
        
        
    };
}
