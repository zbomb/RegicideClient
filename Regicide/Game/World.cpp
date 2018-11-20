//
//    World.cpp
//    Regicide Mobile
//
//    Created: 11/10/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
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
    EntityBase::Cleanup();
}


World* World::GetWorld()
{
    return CurrentInstance;
}
