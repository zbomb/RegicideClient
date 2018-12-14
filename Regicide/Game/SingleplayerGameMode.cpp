//
//    SingleplayerGameMode.cpp
//    Regicide Mobile
//
//    Created: 11/10/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#include "SingleplayerGameMode.hpp"
#include "World.hpp"
#include "SingleplayerAuthority.hpp"
#include "CardEntity.hpp"
#include "HandEntity.hpp"
#include "GraveyardEntity.hpp"
#include "FieldEntity.hpp"
#include "UI/CardViewer.hpp"
#include "Scenes/GameScene.hpp"

using namespace Game;


SingleplayerGameMode::SingleplayerGameMode()
: GameModeBase()
{
}

void SingleplayerGameMode::Initialize()
{

}

void SingleplayerGameMode::Cleanup()
{
    GameModeBase::Cleanup();
}
