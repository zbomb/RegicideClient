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

using namespace Game;

ClientState::ClientState()
: EntityBase( "ClientState" ), LocalPlayer( nullptr ), Opponent( nullptr )
{
}

bool ClientState::StreamPlayer( Player*& Target, PlayerState& Source, bool bOpponent )
{
    auto& Ent = IEntityManager::GetInstance();
    auto& CM = CardManager::GetInstance();
    
    // Create the entity
    Target = Ent.CreateEntity< Player >( Source.EntId );
    
    // Load the king
    // Load King
    auto Engine = Regicide::LuaEngine::GetInstance();
    auto L = Engine ? Engine->State() : nullptr;
    auto King = Target->GetKing();
    
    if( !L || !King )
    {
        cocos2d::log( "[Client] Failed to get lua state!" );
        return false;
    }
    
    auto KingTable = luabridge::newTable( L );
    luabridge::setGlobal( L, KingTable, "KING" );
    
    if( !Engine->RunScript( "kings/" + std::to_string( Source.King ) + ".lua" ) )
    {
        cocos2d::log( "[Client] Failed to load king with id: %d", Source.King );
        
        // Reset KING to nil
        luabridge::setGlobal( L, luabridge::LuaRef( L ), "KING" );
        return false;
    }
    
    if( !King->Load( KingTable, Target, bOpponent ) )
    {
        luabridge::setGlobal( L, luabridge::LuaRef( L ), "KING" );
        return false;
    }
    
    King->OwningPlayer = Target;
    King->UpdateMana( Source.Mana );
    King->UpdateHealth( Source.Health );
    
    // Setup State
    Target->SetMana( Source.Mana );
    Target->SetHealth( Source.Health );
    
    Target->DisplayName     = Source.DisplayName;
    Target->bOpponent       = bOpponent;
    Target->CardBackTexture = "CardBack.png";
    
    auto Deck = Target->GetDeck();
    auto Hand = Target->GetHand();
    
    if( !Hand )
    {
        cocos2d::log( "[Client] Couldnt create hand!" );
        return false;
    }
    
    Hand->bVisibleLocally = !bOpponent;
    
    // Create Cards!
    for( auto It = Source.Deck.begin(); It != Source.Deck.end(); It++ )
    {
        auto Info = CM.GetInfoAddress( It->Id );
        
        if( !Info )
        {
            cocos2d::log( "[Client] Failed to get card info for id '%d'", (int) It->Id );
            continue;
        }
        
        auto NewCard    = CM.CreateCard( *It, Target, true );
        if( !NewCard )
        {
            cocos2d::log( "[Client] Failed to create new card!" );
            continue;
        }

        Deck->AddToTop( NewCard, false );
    }
    
    return true;
}

bool ClientState::StreamFrom( AuthState *Target )
{
    if( !Target )
        return false;
    
    // Game State
    mState = Target->mState;
    pState = Target->pState;
    tState = Target->tState;

    // Load Players
    if( !StreamPlayer( LocalPlayer, Target->GetPlayer(), false ) )
        return false;
    
    if( !StreamPlayer( Opponent, Target->GetOpponent(), true ) )
        return false;
    
    
    return true;
    
}
