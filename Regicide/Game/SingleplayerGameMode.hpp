//
//  SingleplayerGameMode.hpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/10/18.
//

#pragma once

#include "GameModeBase.hpp"


namespace Game
{
    
    class SingleplayerGameMode : GameModeBase
    {
        
    public:
        
        virtual Player& GetLocalPlayer();
        virtual Player& GetOpponent();
        
    protected:
        
        virtual void Initialize();
        virtual void PostInitialize();
        
    };
}
