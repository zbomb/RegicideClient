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
        // TODO
        
        
        void SetActiveQueue( ActionQueue* In );
        void RunActiveQueue();
        void ClearActiveQueue();
        
    protected:
        
        ActionQueue* ActiveQueue;
        
        virtual bool PreHook() override;
        
    }
}
