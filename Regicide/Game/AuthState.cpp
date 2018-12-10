//
//	AuthState.cpp
//	Regicide Mobile
//
//	Created: 12/6/18
//	Updated: 12/6/18
//
//	© 2018 Zachary Berry, All Rights Reserved
//

#include "AuthState.hpp"
#include "GameModeBase.hpp"

using namespace Game;


AuthState::AuthState()
: GameStateBase(), ActiveQueue( nullptr )
{
    
}

AuthState::~AuthState()
{
    ActiveQueue = nullptr;
}

void AuthState::SetActiveQueue( ActionQueue *In )
{
    ActiveQueue = In;
}

void AuthState::RunActiveQueue()
{
    if( ActiveQueue )
    {
        auto World = World::GetWorld();
        auto GM = World ? World->GetGameMode() : nullptr;
        
        CC_ASSERT( GM );
        
        GM->RunActionQueue( std::move( *ActiveQueue ) );
        ActiveQueue = nullptr;
    }
}

void AuthState::ClearActiveQueue()
{
    ActiveQueue = nullptr;
}

bool AuthState::PreHook()
{
    if( !ActiveQueue )
    {
        cocos2d::log( "[State] Attempt to call hook on active state without the action queue set!" );
        return false;
    }
    
    return true;
}
