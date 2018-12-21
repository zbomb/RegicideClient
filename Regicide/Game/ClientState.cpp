//
//	ClientState.cpp
//	Regicide Mobile
//
//	Created: 12/6/18
//	Updated: 12/6/18
//
//	© 2018 Zachary Berry, All Rights Reserved
//

#include "ClientState.hpp"
#include "AuthState.hpp"
#include "Player.hpp"
#include "DeckEntity.hpp"
#include "KingEntity.hpp"
#include "HandEntity.hpp"
#include "FieldEntity.hpp"
#include "GraveyardEntity.hpp"

using namespace Game;

ClientState::ClientState()
: LocalPlayer( nullptr ), Opponent( nullptr )
{
}

Player* ClientState::FindPlayer( uint32_t Id )
{
    if( LocalPlayer && LocalPlayer->GetEntityId() == Id )
        return LocalPlayer;
    else if( Opponent && Opponent->GetEntityId() == Id )
        return Opponent;
    
    return nullptr;
}

CardEntity* ClientState::LookupCard( uint32_t Id, Player *Owner, CardPos Position )
{
    if( !Owner )
        return nullptr;
    
    if( Position == CardPos::HAND || Position == CardPos::NONE )
    {
        auto Hand = Owner->GetHand();
        if( Hand )
        {
            for( auto It = Hand->Begin(); It != Hand->End(); It++ )
            {
                if( *It && (*It)->GetEntityId() == Id )
                    return *It;
            }
        }
    }
    if( Position == CardPos::FIELD || Position == CardPos::NONE )
    {
        auto Field = Owner->GetField();
        if( Field )
        {
            for( auto It = Field->Begin(); It != Field->End(); It++ )
            {
                if( *It && (*It)->GetEntityId() == Id )
                    return *It;
            }
        }
    }
    if( Position == CardPos::DECK || Position == CardPos::NONE )
    {
        auto Deck = Owner->GetDeck();
        if( Deck )
        {
            for( auto It = Deck->Begin(); It != Deck->End(); It++ )
            {
                if( *It && (*It)->GetEntityId() == Id )
                    return *It;
            }
        }
    }
    if( Position == CardPos::GRAVEYARD || Position == CardPos::NONE )
    {
        auto Grave = Owner->GetGraveyard();
        if( Grave )
        {
            for( auto It = Grave->Begin(); It != Grave->End(); It++ )
            {
                if( *It && (*It)->GetEntityId() == Id )
                    return *It;
            }
        }
    }
    
    return nullptr;
}

CardEntity* ClientState::FindCard( uint32_t Id, Player* Owner /* = nullptr */, CardPos Position /* = CardPos::NONE */ )
{
    if( Owner )
        return LookupCard( Id, Owner, Position );
    else
    {
        auto Out = LookupCard( Id, LocalPlayer, Position );
        if( Out )
            return Out;
        else
        {
            return LookupCard( Id, Opponent, Position );
        }
    }
}

bool ClientState::IsPlayerTurn( Player* Target )
{
    if( !Target )
        return false;
    
    if( Target == LocalPlayer && pState == PlayerTurn::LocalPlayer )
        return true;
    else if( Target == Opponent && pState == PlayerTurn::Opponent )
        return true;
    
    return false;
}

Player* ClientState::GetOtherPlayer( Player* Owner )
{
    if( !Owner )
        return nullptr;
    
    if( Owner == LocalPlayer )
        return Opponent;
    else if( Owner == Opponent )
        return LocalPlayer;
    
    return nullptr;
}

Player* ClientState::GetActivePlayer()
{
    if( pState == PlayerTurn::LocalPlayer )
        return LocalPlayer;
    else
        return Opponent;
}

Player* ClientState::GetInactivePlayer()
{
    if( pState == PlayerTurn::LocalPlayer )
        return Opponent;
    else
        return LocalPlayer;
}
