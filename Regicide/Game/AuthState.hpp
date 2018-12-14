//
//	AuthState.hpp
//	Regicide Mobile
//
//	Created: 12/6/18
//	Updated: 12/6/18
//
//	© 2018 Zachary Berry, All Rights Reserved
//

#pragma once

#include "GameStateBase.hpp"

namespace Game
{
    
    class AuthState : public GameStateBase
    {
    public:
        
        AuthState();
        ~AuthState();
        
        // Override Lua Actions
        virtual void ShuffleDeck( PlayerState* Target ) override;
        virtual void DrawCard( PlayerState* Target, uint32_t Count ) override;
        virtual void TakeMana( PlayerState* Target, CardState* Origin, uint32_t Amount ) override;
        virtual void GiveMana( PlayerState* Target, CardState* Origin, uint32_t Amount ) override;
        virtual void DamageKing( PlayerState* Target, CardState* Origin, uint32_t Amount ) override;
        virtual void HealKing( PlayerState* Target, CardState* Origin, uint32_t Amount ) override;
        
        // Card Targeted Actions
        virtual void DoCombat( CardState* Target, CardState* Origin, uint32_t Amount, int StaminaChange ) override;
        virtual void DamageCard( CardState* Target, CardState* Origin, uint32_t Amount ) override;
        virtual void HealCard( CardState* Target, CardState* Origin, uint32_t Amount ) override;
        virtual void GiveStamina( CardState* Target, CardState* Origin, uint32_t Amount ) override;
        virtual void TakeStamina( CardState* Target, CardState* Origin, uint32_t Amount ) override;
        
        void SetActiveQueue( ActionQueue* In );
        void RunActiveQueue();
        void ClearActiveQueue();
        
    protected:
        
        ActionQueue* ActiveQueue;
        virtual bool PreHook();
        
    };
}
