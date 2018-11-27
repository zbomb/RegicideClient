//
//    AuthorityBase.cpp
//    Regicide Mobile
//
//    Created: 11/11/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#include "AuthorityBase.hpp"
#include "Scenes/GameScene.hpp"
#include "GameModeBase.hpp"

using namespace Game;

AuthorityBase::AuthorityBase()
: EntityBase( "Authority" )
{
    
}

AuthorityBase::~AuthorityBase()
{
    
}


void AuthorityBase::Cleanup()
{
    EntityBase::Cleanup();
}
