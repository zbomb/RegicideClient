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

    class ClientState
    {
    public:
        
        ClientState();
        
        Player* GetPlayer() { return LocalPlayer; }
        Player* GetOpponent() { return Opponent; }
        
        MatchState mState;
        PlayerTurn pState;
        TurnState tState;
        
        Player* FindPlayer( uint32_t Id );
        CardEntity* FindCard( uint32_t Id, Player* Owner = nullptr, CardPos Position = CardPos::NONE );
        
        bool IsPlayerTurn( Player* Target );
        Player* GetOtherPlayer( Player* Owner );
        
    protected:
        
        Player* LocalPlayer;
        Player* Opponent;
        
        CardEntity* LookupCard( uint32_t Id, Player* Owner, CardPos Position );

        friend class SingleplayerLauncher;
        
    };

}
