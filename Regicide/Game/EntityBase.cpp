//
//    EntityBase.cpp
//    Regicide Mobile
//
//    Created: 11/10/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#include "EntityBase.hpp"
#include "Scenes/GameScene.hpp"
#include "World.hpp"
#include "Actions.hpp"

using namespace Game;


bool IEntityManager::EntityExists( uint32 EntityId )
{
    return EntityStore.count( EntityId ) > 0;
}

/*=================================================================================================
    IEntityManager::CallCleanup( EntityBase* ) [INTERNAL]
     -> Calls the 'Cleanup' method on this entity and all children entities that this
        entity owns. The children are always cleaned up before the parent. INTERNAL METHOD
=================================================================================================*/
void IEntityManager::CallCleanup( EntityBase* In )
{
    if( !In )
        return;
    
    for( auto It = In->ChildBegin(); It != In->ChildEnd(); It++ )
    {
        if( *It )
            CallCleanup( *It );
    }
    
    In->Cleanup();
}

/*=================================================================================================
     IEntityManager::DoDestroy( unit32_t (ENT ID) ) [INTERNAL]
     -> Destroys an entity and all of its children. Children are always destroyed before
        the parent gets destroyed. INTERNAL METHOD. DO NOT CALL
 =================================================================================================*/
void IEntityManager::DoDestroy( std::map< uint32, std::shared_ptr< EntityBase > >::iterator Iter ) 
{
    if( Iter == EntityStore.end() )
        return;
    
    if( Iter->second )
    {
        // Recursivley destroy child entities
        for( auto It = Iter->second->ChildBegin(); It != Iter->second->ChildEnd(); It++ )
        {
            if( *It )
            {
                DoDestroy( EntityStore.find( (*It)->EID ) );
            }
        }
        
        // Destroy this entity and clear entry in master list
        Iter->second.reset();
    }
    
    EntityStore.erase( Iter );
}

/*=================================================================================================
     IEntityManager::DestroyEntity( uint32_t EntityId )
     -> Cleanup and destroy this Entity and all children Entities that this Entity owns.
        Children are always cleaned and always destroyed before the parent. Cleanup will be called
        on ALL entities in the tree before ANY are destroyed!
 =================================================================================================*/
bool IEntityManager::DestroyEntity( uint32 EntityId )
{
    // Lookup Entity in master list
    auto Result = EntityStore.find( EntityId );
    if( Result == EntityStore.end() )
        return false;
    
    if( Result->second )
    {
        auto Target = Result->second;
        
        // Cleanup Entity Tree
        CallCleanup( Result->second.get() );
        
        // Destroy Entity Tree
        DoDestroy( Result );
    }
    
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
    cocos2d::Director::getInstance()->getScheduler()->unscheduleAllForTarget( this );
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

void EntityBase::PerformAction( Game::Action* In, std::function< void() > OnComplete )
{
    if( !In )
    {
        cocos2d::log( "[ENT] Attempt to run null action!" );
        if( OnComplete )
            OnComplete();
    }
    
    // Check if theres a callback tied to this action
    if( ActionCallbacks.count( In->ActionName ) > 0 && ActionCallbacks[ In->ActionName ] )
    {
        ActionCallbacks[ In->ActionName ]( In, OnComplete );
    }
    else if( OnComplete )
    {
        // Print Warning
        cocos2d::log( "[Ent] Warning: Unbound Action! Name: %s", In->ActionName.c_str() );
        OnComplete();
    }
}

void EntityBase::SetActionCallback( const std::string& ActionId, std::function< void( Game::Action* In, std::function< void() > ) > Callback )
{
    ActionCallbacks[ ActionId ] = Callback;
}

World* EntityBase::GetWorld() const
{
    auto world = World::GetWorld();
    CC_ASSERT( world );
    
    return world;
}
