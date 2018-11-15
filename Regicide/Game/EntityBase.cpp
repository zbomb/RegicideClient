//
//  EntityBase.cpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/10/18.
//

#include "EntityBase.hpp"
#include "Scenes/GameScene.hpp"

using namespace Game;



bool IEntityManager::EntityExists( uint32 EntityId )
{
    return EntityStore.count( EntityId ) > 0;
}


bool IEntityManager::DestroyEntity( uint32 EntityId, bool bIncludeChildren /* = true */ )
{
    auto Result = EntityStore.find( EntityId );
    if( Result == EntityStore.end() )
    {
        return false;
    }
    
    if( Result->second )
    {
        auto Target = Result->second;
        
        // Recursivley destroy children entities
        for( auto It = Target->Children.begin(); It != Target->Children.end(); It++ )
        {
            if( *It )
                DestroyEntity( (*It)->EID );
        }
        
        // Cleanup and delete this entitiy
        Result->second->Cleanup();
        Result->second.reset();
    }
    
    EntityStore.erase( Result );
    return true;
}


bool IEntityManager::DestroyEntity( EntityBase *EntityRef, bool bIncludeChildren /* = true */ )
{
    return EntityRef ? DestroyEntity( EntityRef->GetEntityId(), bIncludeChildren ) : false;
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
    
    Position = cocos2d::Vec2::ZERO;
    Rotation = 0.f;
}

// Destructor
EntityBase::~EntityBase()
{
    EName.clear();
}

void EntityBase::AddChild( EntityBase* inChild )
{
    if( inChild && !HasChild( inChild ) )
    {
        // Check if this entity is already owned somewhere else, and remove
        if( inChild->Owner )
        {
            inChild->Owner->RemoveChild( inChild, false );
        }
        
        inChild->Owner = this;
        Children.push_back( inChild );
    }
}

bool EntityBase::HasChild( EntityBase* inChild )
{
    if( inChild )
    {
        for( auto It = Children.begin(); It != Children.end(); It++ )
        {
            if( *It == inChild )
            {
                // While were here, ensure Owner is set properly
                if( (*It)->Owner != this )
                {
                    if( (*It)->Owner )
                        (*It)->Owner->RemoveChild( *It, false );
                    
                    (*It)->Owner = this;
                }
                
                return true;
            }
        }
    }
    
    return false;
}

// Returns false if this entity isnt actually a child, or invalid
bool EntityBase::RemoveChild( EntityBase* inChild, bool bDestroy /* = true */ )
{
    if( inChild )
    {
        for( auto It = Children.begin(); It != Children.end(); It++ )
        {
            if( (*It) == inChild )
            {
                inChild->Owner = nullptr;
                Children.erase( It );
                
                if( bDestroy )
                {
                    IEntityManager::GetInstance().DestroyEntity( inChild );
                }
                
                return true;
            }
        }
    }
    
    return false;
}

void EntityBase::AddToScene( cocos2d::Node* inNode )
{

}

void EntityBase::Initialize()
{
    // Pass hook to children
    for( auto It = Children.begin(); It != Children.end(); It++ )
    {
        if( *It )
            (*It)->Initialize();
    }
    
}

void EntityBase::SceneInit( cocos2d::Scene* inScene )
{
    CC_ASSERT( inScene );

    // Pass hook to children
    for( auto It = Children.begin(); It != Children.end(); It++ )
    {
        if( *It )
            (*It)->SceneInit( inScene );
    }
    
    // Link scene
    LinkedScene = inScene;
}

void EntityBase::PostInit()
{
    for( auto It = Children.begin(); It != Children.end(); It++ )
    {
        if( *It )
            (*It)->PostInit();
    }
}

void EntityBase::SetPosition( const cocos2d::Vec2& inPos )
{
    Position = inPos;
    Invalidate();
}

void EntityBase::SetRotation( float inRot )
{
    Rotation = inRot;
    Invalidate();
}

cocos2d::Vec2 EntityBase::GetAbsolutePosition() const
{
    // Local Position + Parent Position
    auto owner = GetOwner();
    if( owner )
        return owner->GetAbsolutePosition() + Position;
    else
        return Position;
}

float EntityBase::GetAbsoluteRotation() const
{
    auto owner = GetOwner();
    if( owner )
        return owner->GetAbsoluteRotation() + Rotation;
    else
        return Rotation;
}

void EntityBase::Invalidate()
{
    // Invalidate Children
    for( auto It = Children.begin(); It != Children.end(); It++ )
    {
        if( *It )
            (*It)->Invalidate();
    }
}

int EntityBase::LoadResources( const std::function<void ()> &Callback )
{
    // Return the number of times the callback will be executed
    return 0;
}
