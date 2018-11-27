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
#include "GraveyardEntity.hpp"

#define LUA_POS_HAND 1
#define LUA_POS_FIELD 2
#define LUA_POS_GRAVE 3
#define LUA_POS_DECK 4


namespace Regicide
{
    
    class LuaGame
    {
    public:
        
        static bool IsTurn( Game::CardEntity* In )
        {
            if( !In )
                return false;
            
            auto Pl = In->GetOwningPlayer();
            return Pl ? Pl->IsTurn() : false;
        }
        
        static bool IsTurnKing( Game::KingEntity* In )
        {
            if( !In )
                return false;
            
            auto Pl = In->GetOwningPlayer();
            return Pl ? Pl->IsTurn() : false;
        }
        
        static Game::Player* GetOpponent( Game::CardEntity* In )
        {
            if( !In )
                return nullptr;
            
            auto world = Game::World::GetWorld();
            if( !world )
                return nullptr;
            
            auto Owner = In->GetOwningPlayer();
            if( !Owner )
                return nullptr;
            
            return world->GetLocalPlayer() == Owner ? world->GetOpponent() : world->GetLocalPlayer();
        }
    };
    
    class LuaActions
    {
    public:
        
        template< typename T >
        static T* CreateLuaAction( Game::EntityBase* inTarget, int ActionType )
        {
            // Build Action
            return Game::SingleplayerAuthority::CreateLuaAction< T >( inTarget, ActionType );
        }
        
        static bool AddCardDraw( Game::Player* Target, int ActionType )
        {
            if( !Target )
            {
                cocos2d::log( "[Lua] Warning: Failed to create draw card action.. invalid player specified" );
                return false;
            }
            
            // Get top card off deck
            auto Deck = Target->GetDeck();
            if( !Deck )
            {
                cocos2d::log( "[Lua] Warning: Failed to create draw card action.. invalid player specified" );
                return false;
            }
            
            auto Card = Deck->At( Game::SingleplayerAuthority::Lua_PopDeck( Target ) );
            if( !Card )
            {
                cocos2d::log( "[Lua] Warning: Failed to create draw card action.. out of cards!" );
                return false;
            }
            
            Game::DrawCardAction* NewAction = CreateLuaAction< Game::DrawCardAction >( Target, ActionType );
            if( !NewAction )
            {
                cocos2d::log( "[Lua] Warning: Failed to create draw card action" );
                return false;
            }
            
            NewAction->TargetCard = Card->GetEntityId();
            
            return true;
        }
        
        static bool AddDamageCard( Game::CardEntity* Target, Game::CardEntity* Inflictor, int Amount, int ActionType )
        {
            if( !Target || !Inflictor )
            {
                cocos2d::log( "[Lua] Warning: Failed to create damage action.. invalid card specified" );
                return false;
            }
            
            if( Amount <= 0 )
            {
                cocos2d::log( "[Lua] Warning: Failed to create damage action.. damage must at least be 1" );
                return false;
            }
            
            // Target for damage action is GM
            auto world = Game::World::GetWorld();
            auto GM = world ? world->GetGameMode() : nullptr;
            
            if( !GM )
            {
                cocos2d::log( "[Lua] Warning: Failed to create damage action.. gamemode was null" );
                return false;
            }
            
            auto NewAction = CreateLuaAction< Game::DamageAction >( GM, ActionType );
            if( !NewAction )
            {
                cocos2d::log( "[Lua] Warning: Failed to create damage action!" );
                return false;
            }
            
            // Setup Action
            NewAction->ActionName = "CardDamage";
            NewAction->Target = Target;
            NewAction->Inflictor = Inflictor;
            NewAction->Damage = Amount;
            NewAction->StaminaDrain = 0;
            return true;
            
        }
        
        static bool AddDamageKing( Game::Player* Target, Game::CardEntity* Inflictor,  int Amount, int ActionType )
        {
            if( Amount <= 0 )
            {
                cocos2d::log( "[Lua] Warning: Failed to create king damage action.. damage amount must at least be 1" );
                return false;
            }
            
            if( !Target || !Inflictor )
            {
                cocos2d::log( "[Lua] Warning: Failed to create king damage action.. invalid target/origin" );
                return false;
            }
            
            auto NewAction = CreateLuaAction< Game::DamageAction >( Target, ActionType );
            if( !NewAction )
            {
                cocos2d::log( "[Lua] Warning: Failed to create king damage action!" );
                return false;
            }
            
            // Setup Action
            NewAction->ActionName = "KingDamage";
            NewAction->Inflictor = Inflictor;
            NewAction->Damage = Amount;
            NewAction->StaminaDrain = 0;
            return true;
            
        }
        
    };
    
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
            .addFunction( "IsFaceUp", &Game::CardEntity::IsFaceUp )
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
        .beginClass< Regicide::LuaActions >( "Action" )
            .addStaticFunction( "DrawCard", &LuaActions::AddCardDraw )
            .addStaticFunction( "DamageCard", &LuaActions::AddDamageCard )
            .addStaticFunction( "DamageKing", &LuaActions::AddDamageKing )
        .endClass()
        .beginClass< Regicide::LuaGame >( "Game" )
            .addStaticFunction( "IsCardTurn", &LuaGame::IsTurn )
            .addStaticFunction( "IsKingTurn", &LuaGame::IsTurnKing )
            .addStaticFunction( "GetOpponent", &LuaGame::GetOpponent )
        .endClass();

    }
}
