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


void AuthState::ShuffleDeck( PlayerState* Target )
{
    GameStateBase::ShuffleDeck( Target );
    
    // TODO: Play animation, were not actually going to send the players
    // the new deck order obviously.. although the 'unknown' deck order still
    // needs to be implemented on a fundamental level, so leave this for now
}

void AuthState::DrawCard( PlayerState* Target, uint32_t Count )
{
    if( !ActiveQueue || !Target )
    {
        throw std::exception();
    }
    
    // Were going to fully override this function
    if( Count <= 0 )
        return;
    
    for( int i = 0; i < Count; i++ )
    {
        if( Target->Deck.size() < 1 )
        {
            // TODO: Call out to win function
            return;
        }
        
        auto It = Target->Deck.begin();
        
        It->Position    = CardPos::HAND;
        It->FaceUp      = false;
        
        auto Draw = ActiveQueue->CreateAction< DrawCardAction >();
        Draw->TargetCard = It->EntId;
        Draw->TargetPlayer = Target->EntId;
        
        Target->Hand.push_back( *It );
        Target->Deck.erase( It );
    }
}

void AuthState::TakeMana( PlayerState* Target, CardState* Origin, uint32_t Amount )
{
    if( !ActiveQueue || !Target )
    {
        throw std::exception();
    }
    
    GameStateBase::TakeMana( Target, Origin, Amount );
    
    auto TakeMana = ActiveQueue->CreateAction< UpdateManaAction >();
    TakeMana->TargetPlayer = Target->EntId;
    TakeMana->Amount = Target->Mana;
}

void AuthState::GiveMana( PlayerState* Target, CardState* Origin, uint32_t Amount )
{
    if( !ActiveQueue || !Target )
        throw std::exception();
    
    GameStateBase::GiveMana( Target, Origin, Amount );
    
    auto GiveMana = ActiveQueue->CreateAction< UpdateManaAction >();
    GiveMana->TargetPlayer = Target->EntId;
    GiveMana->Amount = Target->Mana;
}

void AuthState::DamageKing( PlayerState* Target, CardState* Origin, uint32_t Amount )
{
    if( !ActiveQueue || !Target )
        throw std::exception();
    
    GameStateBase::DamageKing( Target, Origin, Amount );
    
    auto Damage = ActiveQueue->CreateAction< DamageAction >();
    Damage->Inflictor = Origin ? Origin->EntId : 0;
    Damage->Target = Target->EntId;
    Damage->UpdatedPower = Target->Health;
    Damage->Damage = (uint16_t) Amount;
}

void AuthState::HealKing( PlayerState* Target, CardState* Origin, uint32_t Amount )
{
    if( !ActiveQueue || !Target )
        throw std::exception();
    
    GameStateBase::HealKing( Target, Origin, Amount );
    
    auto Heal = ActiveQueue->CreateAction< DamageAction >();
    Heal->Inflictor = Origin ? Origin->EntId : 0;
    Heal->Target = Target->EntId;
    Heal->UpdatedPower = Target->Health;
    Heal->Damage = (uint16_t) Amount;
}

void AuthState::DoCombat( CardState* Target, CardState* Origin, uint32_t Amount, int StaminaChange )
{
    // TODO
}

void AuthState::DamageCard( CardState* Target, CardState* Origin, uint32_t Amount )
{
    if( !ActiveQueue || !Target )
        throw std::exception();
    
    GameStateBase::DamageCard( Target, Origin, Amount );
    
    auto Damage = ActiveQueue->CreateAction< DamageAction >();
    Damage->Inflictor = Origin ? Origin->EntId : 0;
    Damage->Target = Target->EntId;
    Damage->UpdatedPower = Target->Power;
    Damage->Damage = (uint16_t) Amount;
}

void AuthState::HealCard( CardState* Target, CardState* Origin, uint32_t Amount )
{
    if( !ActiveQueue || !Target )
        throw std::exception();
    
    GameStateBase::HealCard( Target, Origin, Amount );
    
    auto Health = ActiveQueue->CreateAction< DamageAction >();
    Health->Inflictor = Origin ? Origin->EntId : 0;
    Health->Target = Target->EntId;
    Health->UpdatedPower = Target->Power;
    Health->Damage = (uint16_t) Amount;
}

void AuthState::GiveStamina( CardState* Target, CardState* Origin, uint32_t Amount )
{
    if( !ActiveQueue || !Target )
        throw std::exception();
    
    GameStateBase::GiveStamina( Target, Origin, Amount );
    
    auto Update = ActiveQueue->CreateAction< UpdateStaminaAction >();
    Update->Target = Target->EntId;
    Update->Inflictor = Origin ? Origin->EntId : 0;
    Update->UpdatedAmount = Target->Stamina;
    Update->Amount = Amount;
}

void AuthState::TakeStamina( CardState* Target, CardState* Origin, uint32_t Amount )
{
    if( !ActiveQueue || !Target )
        throw std::exception();
    
    GameStateBase::TakeStamina( Target, Origin, Amount );
    
    auto Update = ActiveQueue->CreateAction< UpdateStaminaAction >();
    Update->Target = Target->EntId;
    Update->Inflictor = Origin ? Origin->EntId : 0;
    Update->UpdatedAmount = Target->Stamina;
    Update->Amount = Amount;
}
