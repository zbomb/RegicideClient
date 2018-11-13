//
//  World.cpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/10/18.
//

#include "World.hpp"

using namespace Game;

World* World::CurrentInstance( nullptr );


World::World()
: EntityBase( "World" )
{
    CC_ASSERT( !CurrentInstance );
    CurrentInstance = this;
}

World::~World()
{
    CC_ASSERT( this == CurrentInstance );
    
    CurrentInstance     = nullptr;
    GM                  = nullptr;
    Auth                = nullptr;
}

void World::Cleanup()
{
}


World* World::GetWorld()
{
    return CurrentInstance;
}
