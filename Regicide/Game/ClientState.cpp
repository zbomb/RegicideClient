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
