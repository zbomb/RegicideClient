//
//	Lua_GameBindings.h
//	Regicide Mobile
//
//	Created: 11/22/18
//	Updated: 11/22/18
//
//	© 2018 Zachary Berry, All Rights Reserved
//

#pragma once

#include "LuaEngine.hpp"
#include "EntityBase.hpp"
#include "CardEntity.hpp"
#include "Player.hpp"
#include "DeckEntity.hpp"
#include "HandEntity.hpp"
#include "FieldEntity.hpp"
#include "KingEntity.hpp"
#include "Actions.hpp"
#include "World.hpp"
#include "GameModeBase.hpp"
#include "SingleplayerAuthority.hpp"
#include "Utils.hpp"
#include "GameContext.hpp"
#include "GraveyardEntity.hpp"
#include "SimulatedState.hpp"

#define LUA_POS_HAND 1
#define LUA_POS_FIELD 2
#define LUA_POS_GRAVE 3
#define LUA_POS_DECK 4


namespace Regicide
{
    
    static void LuaBind_Game( lua_State* L )
    {
        
        luabridge::getGlobalNamespace( L )
        .beginClass< Game::EntityBase  >( "Entity" )
            .addFunction( "GetEntityId", &Game::EntityBase::GetEntityId )
            .addFunction( "GetEntityName", &Game::EntityBase::GetEntityName )
        .endClass()
        .deriveClass< Game::CardEntity, Game::EntityBase >( "Card" )
            .addFunction( "InDeck", &Game::CardEntity::InDeck )
            .addFunction( "InHand", &Game::CardEntity::InHand )
            .addFunction( "InGrave", &Game::CardEntity::InGrave )
            .addFunction( "OnField", &Game::CardEntity::OnField )
            .addFunction( "GetPower", &Game::CardEntity::_lua_GetPower )
            .addFunction( "GetStamina", &Game::CardEntity::_lua_GetStamina )
            .addFunction( "GetCardId", &Game::CardEntity::_lua_GetCardId )
            .addFunction( "GetManaCost", &Game::CardEntity::_lua_GetManaCost )
            .addFunction( "GetOwner", &Game::CardEntity::GetOwningPlayer )
            .addFunction( "GetName", &Game::CardEntity::_lua_GetName )
        .endClass()
        .deriveClass< Game::DeckEntity, Game::EntityBase >( "Deck" )
            .addFunction( "GetCount", &Game::DeckEntity::Count )
            .addFunction( "GetDeckId", &Game::DeckEntity::GetDeckId )
            .addFunction( "GetName", &Game::DeckEntity::GetName )
            .addFunction( "IndexValid", &Game::DeckEntity::IndexValid )
            .addFunction( "GetIndex", &Game::DeckEntity::At )
        .endClass()
        .deriveClass< Game::HandEntity, Game::EntityBase >( "Hand" )
            .addFunction( "GetCount", &Game::HandEntity::Count )
            .addFunction( "IndexValid", &Game::HandEntity::IndexValid )
            .addFunction( "GetIndex", &Game::HandEntity::At )
        .endClass()
        .deriveClass< Game::FieldEntity, Game::EntityBase >( "Field" )
            .addFunction( "GetCount", &Game::FieldEntity::Count )
            .addFunction( "IndexValid", &Game::FieldEntity::IndexValid )
            .addFunction( "GetIndex", &Game::FieldEntity::At )
            .addFunction( "GetCards", &Game::FieldEntity::_lua_GetCards )
        .endClass()
        .deriveClass< Game::GraveyardEntity, Game::EntityBase >( "Graveyard" )
            .addFunction( "GetCount", &Game::GraveyardEntity::Count )
            .addFunction( "IndexValid", &Game::GraveyardEntity::IndexValid )
            .addFunction( "GetIndex", &Game::GraveyardEntity::At )
        .endClass()
        .deriveClass< Game::KingEntity, Game::EntityBase >( "King" )
            .addFunction( "GetName", &Game::KingEntity::GetName )
        .endClass()
        .deriveClass< Game::Player, Game::EntityBase >( "Player" )
            .addFunction( "GetDeck", &Game::Player::GetDeck )
            .addFunction( "GetHand", &Game::Player::GetHand )
            .addFunction( "GetField", &Game::Player::GetField )
            .addFunction( "GetGraveyard", &Game::Player::GetGraveyard )
            .addFunction( "GetKing", &Game::Player::GetKing )
            .addFunction( "GetName", &Game::Player::GetName )
            .addFunction( "GetMana", &Game::Player::GetMana )
            .addFunction( "GetHealth", &Game::Player::GetHealth )
            .addFunction( "IsTurn", &Game::Player::IsTurn )
        .endClass()
        .beginClass< Game::CardState >( "CardState" )
            .addData( "Id", &Game::CardState::EntId )
            .addData( "Power", &Game::CardState::Power )
            .addData( "Stamina", &Game::CardState::Stamina )
            .addData( "ManaCost", &Game::CardState::ManaCost )
            .addData( "FaceUp", &Game::CardState::FaceUp )
            .addProperty( "Position", &Game::CardState::_lua_GetPosition )
            .addProperty( "Owner", &Game::CardState::_lua_GetOwner )
        .endClass()
        .beginClass< Game::LuaContext >( "LuaContext" )
            .addFunction( "DrawCard", &Game::LuaContext::DrawCard )
            .addFunction( "DealDamage", &Game::LuaContext::DealDamage )
            .addFunction( "GetState", &Game::LuaContext::GetState )
            .addFunction( "IsTurn", &Game::LuaContext::IsTurn )
            .addFunction( "GetField", &Game::LuaContext::GetField )
            .addFunction( "GetOwner", &Game::LuaContext::GetOwner )
            .addFunction( "GetOpponent", &Game::LuaContext::GetOpponent )
        .endClass()
        .deriveClass< Game::GameContext, Game::LuaContext >( "GameContext" )
        .endClass()
        .deriveClass< Game::SimulatorContext, Game::LuaContext >( "SimulatorContext" )
        .endClass();

    }
}
