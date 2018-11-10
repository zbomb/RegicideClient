//
//  EntityBase.cpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/10/18.
//

#include "EntityBase.hpp"

using namespace Game;



bool IEntityManager::EntityExists( uint32 EntityId )
{
    return EntityStore.count( EntityId ) > 0;
}


bool IEntityManager::DestroyEntity( uint32 EntityId )
{
    auto Result = EntityStore.find( EntityId );
    if( Result == EntityStore.end() )
    {
        return false;
    }
    
    if( Result->second )
    {
        Result->second->Cleanup();
        Result->second.reset();
    }
    
    EntityStore.erase( Result );
    return true;
}


bool IEntityManager::DestroyEntity( EntityBase *EntityRef )
{
    return EntityRef ? DestroyEntity( EntityRef->GetEntityId() ) : false;
}


EntityBase* IEntityManager::GetEntity( uint32 EntityId )
{
    auto Result = EntityStore.find( EntityId );
    if( Result == EntityStore.end() )
        return nullptr;
    
    if( Result->second )
        return Result->second.get();
    
    return nullptr;
}


// Singleton Access Pattern (Avoiding dynamic allocation)
IEntityManager& IEntityManager::GetInstance()
{
    static IEntityManager Singleton;
    return Singleton;
}


IEntityManager::~IEntityManager()
{
    // Cleanup all entities and delete them
    for( auto It = EntityStore.begin(); It != EntityStore.end(); It++ )
    {
        if( It->second )
        {
            It->second->Cleanup();
            It->second.reset();
        }
    }
    
    EntityStore.clear();
}


// Cleanup
// Called right before destruction, giving any entities the chance to do some cleanup
// work before the object is explicitly destroyed
void EntityBase::Cleanup()
{
    
}

// Constructor
// An entity name must be passed in on construction. Entity names dont need to be unique
EntityBase::EntityBase( const std::string& Name )
{
    EName = Name;
}

// Destructor
EntityBase::~EntityBase()
{
    EName.clear();
    EID = 0;
}
