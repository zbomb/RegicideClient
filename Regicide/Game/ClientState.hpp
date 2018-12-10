//
//	ClientState.hpp
//	Regicide Mobile
//
//	Created: 12/6/18
//	Updated: 12/6/18
//
//	© 2018 Zachary Berry, All Rights Reserved
//

#pragma once

#include "EntityBase.hpp"
#include "Actions.hpp"
#include "ObjectStates.hpp"

namespace Game
{
    class Player;
    class AuthState;

    class ClientState : public EntityBase
    {
    public:
        
        ClientState();
        
        Player* GetPlayer() { return LocalPlayer; }
        Player* GetOpponent() { return Opponent; }
        
        MatchState mState;
        PlayerTurn pState;
        TurnState tState;
        
    protected:
        
        Player* LocalPlayer;
        Player* Opponent;
        
        bool StreamPlayer( Player*& Target, PlayerState& Source, bool bOpponent );
        bool StreamFrom( AuthState* Target );
        
        friend class SingleplayerLauncher;
        
    };

}
