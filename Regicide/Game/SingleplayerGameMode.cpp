//
//  SingleplayerGameMode.cpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/10/18.
//

#include "SingleplayerGameMode.hpp"


using namespace Game;



Player& SingleplayerGameMode::GetLocalPlayer()
{
    static Player Debug;
    return Debug;
}

Player& SingleplayerGameMode::GetOpponent()
{
    static Player Debug;
    return Debug;
}

void SingleplayerGameMode::Initialize()
{
    
}

void SingleplayerGameMode::PostInitialize()
{
    
}
