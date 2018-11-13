//
//  GameModeBase.cpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/10/18.
//

#include "GameModeBase.hpp"

using namespace Game;


GameModeBase::GameModeBase()
: EntityBase( "GameMode" )
{
    
}

GameModeBase::~GameModeBase()
{
    
}

void GameModeBase::Cleanup()
{
    
}

World* GameModeBase::GetWorld() const
{
    auto Ret = World::GetWorld();
    CC_ASSERT( Ret );
    
    return Ret;
}
