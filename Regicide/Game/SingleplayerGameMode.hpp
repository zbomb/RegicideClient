//
//    SingleplayerGameMode.hpp
//    Regicide Mobile
//
//    Created: 11/10/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#pragma once

#include "GameModeBase.hpp"
#include "CardEntity.hpp"
#include <chrono>

class CardViewer;

namespace Game
{
    
    class Player;
    
    class SingleplayerGameMode : public GameModeBase
    {
        
    public:
        
        SingleplayerGameMode();
        virtual void Cleanup() override;
        
    protected:
        
        virtual void Initialize() override;
        
        void Action_GameWon( Action* In, std::function< void() > Callback );

        friend class SingleplayerLauncher;
        
    };
}
