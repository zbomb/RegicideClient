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
#include "CardEntity.hpp"

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
    {
        return false;
    }
    
    if( Result->second )
    {
        auto Target = Result->second;
        
        // Cleanup Entity Tree
        CallCleanup( Target.get() );
        
        // Remove from parent
        auto Parent = Result->second->GetOwner();
        if( Parent )
            Parent->RemoveChild( Result->second.get(), false );
        
        // Destroy Entity Tree
        DoDestroy( Result );
    }
    
    return true;
}

/*=================================================================================================
    IEntityManager::DestroyEntity( Entity )
     -> Cleanup and destroy this Entity and all children Entities that this Entity owns.
        Children are always cleaned and always destroyed before the parent. Cleanup will be called
        on ALL entities in the tree before ANY are destroyed!
 =================================================================================================*/
bool IEntityManager::DestroyEntity( EntityBase *EntityRef )
{
    return EntityRef ? DestroyEntity( EntityRef->GetEntityId() ) : false;
}

/*=================================================================================================
     IEntityManager::GetEntity( uint32 EntityId )
     -> Looks up an Entity in the master list. Returns nullptr if not found
 =================================================================================================*/
EntityBase* IEntityManager::GetEntity( uint32 EntityId )
{
    auto Result = EntityStore.find( EntityId );
    if( Result == EntityStore.end() )
    {
        return nullptr;
    }
    
    if( Result->second )
    {
        return Result->second.get();
    }
    
    return nullptr;
}


/*=================================================================================================
    static IEntityManager::GetInstance()
    -> Returns a reference to the EntityManager singleton. Ensure return type is explicitly
       stated as a reference, the copy constructor is deleted, so you will get an error
 =================================================================================================*/
IEntityManager& IEntityManager::GetInstance()
{
    static IEntityManager Singleton;
    return Singleton;
}

/*=================================================================================================
     IEntityManager::~IEntityManager()
     -> Destructor
 =================================================================================================*/
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

std::vector< CardEntity* > IEntityManager::GetAllCards()
{
    std::vector< CardEntity* > Output;
    
    for( auto It = Begin(); It != End(); It++ )
    {
        if( It->second && It->second->IsCard() )
        {
            auto Card = dynamic_cast< CardEntity* >( It->second.get() );
            if( Card )
                Output.push_back( Card );
        }
    }
    
    return Output;
}


/*=====================================================================================================
 ======================================================================================================
    EntityBase Class
 ======================================================================================================
 =====================================================================================================*/

/*=================================================================================================
    EntityBase::Cleanup()
    -> Override this to perform any cleanup before this Entity is destroyed. The parent entity
       is garunteed to still be valid when called.
    -> Do not forget to call this base class method! It unschedules any timers
 =================================================================================================*/
void EntityBase::Cleanup()
{
    cocos2d::Director::getInstance()->getScheduler()->unscheduleAllForTarget( this );
}

/*=================================================================================================
    EntityBase::EntityBase( string )
    -> Constructor for the EntityBase class. Be sure to call this on construction of any derived
       entities. Pass in an 'Entity Name'. Doesnt have to be unique
 =================================================================================================*/
EntityBase::EntityBase( const std::string& Name )
{
    EName = Name;
    
    Position = cocos2d::Vec2::ZERO;
    Rotation = 0.f;
    LinkedScene = nullptr;
}

/*=================================================================================================
    EntityBase::~EntityBase()
    -> Destructor for the EntityBase class
 =================================================================================================*/
EntityBase::~EntityBase()
{
    EName.clear();
}

/*=================================================================================================
    EntityBase::AddChild( Entity )
     -> Adds a child entity to this entity. Child Entities are cleaned and destroyed along with the
        parent Entity. Entities can also access children via 'Children' vector
 =================================================================================================*/
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

/*=================================================================================================
    EntityBase::HasChild( Entity ) -> bool
    -> Checks if the passed in entity is a child entity of this entity.
    -> If nullptr is passed in, false will be returned
 =================================================================================================*/
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

/*=================================================================================================
    EntityBase::RemoveChild( Entity, bool ShouldDestroy = true ) -> bool
    -> Removes a child Entity, optionally destroying it (along with its child entities)
    -> If the entity is not a child, or nullptr, false will be returned, and the entity
       will not be destroyed.
 =================================================================================================*/
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

/*=================================================================================================
    EntityBase::AddToScene( Node* In )
    -> Called when the Entity can optionally be added to the scene. Override this method if
       an entity has a cocos Node, like a sprite or label.
    -> No need to call the BaseClass method
 =================================================================================================*/
void EntityBase::AddToScene( cocos2d::Node* inNode )
{

}

/*=================================================================================================
    EntityBase::Initialize()
    -> Called after the Entity is constructed and set up
    -> Override to perform any additional setup
    -> Do not forget to call BaseClass method!
 =================================================================================================*/
void EntityBase::Initialize()
{
    // Pass hook to children
    for( auto It = Children.begin(); It != Children.end(); It++ )
    {
        if( *It )
            (*It)->Initialize();
    }
    
}

/*=================================================================================================
    EntityBase::SceneInit( Scene )
    -> Another Initialize like function, for after entities are added to a scene
    -> The BaseClass method MUST be called, otherwise LinkedScene will not be set
 =================================================================================================*/
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

/*=================================================================================================
    EntityBase::PostInit()
    -> Final Initialize hook called before GamePlay
    -> Called after all entities are constructed, added to scene, Initialize and SceneInit have
       been called. Eventually, some of these Inits need to be deprecated
    -> Ensure BaseClass method is called!
 =================================================================================================*/
void EntityBase::PostInit()
{
    for( auto It = Children.begin(); It != Children.end(); It++ )
    {
        if( *It )
            (*It)->PostInit();
    }
}

/*=================================================================================================
    EntityBase::SetPosition( Vec2 )
    -> Updates the position of this entity, relative to the parent entity
    -> If an entity has a Cocos Node, update the position of the node in EntityBase::Invalidate
 =================================================================================================*/
void EntityBase::SetPosition( const cocos2d::Vec2& inPos )
{
    Position = inPos;
    Invalidate();
}

/*=================================================================================================
    EntityBase::SetRotation( float )
    -> Updates the rotation of this entity, relative to the parent entity
    -> If an entity has a Cocos Node, update the rotation of the node in EntityBase::Invalidate
 =================================================================================================*/
void EntityBase::SetRotation( float inRot )
{
    Rotation = inRot;
    Invalidate();
}

/*=================================================================================================
    EntityBase::GetAbsolutePosition() -> Vec2
    -> Gets the position of this entity, but unlike GetPosition(), not relative to parent
 =================================================================================================*/
cocos2d::Vec2 EntityBase::GetAbsolutePosition() const
{
    // Local Position + Parent Position
    auto owner = GetOwner();
    if( owner )
        return owner->GetAbsolutePosition() + Position;
    else
        return Position;
}

/*=================================================================================================
    EntityBase::GetAbsoluteRotation() -> float
    -> Gets the rotation of this entity, but unlike GetRotation(), not relative to parent
 =================================================================================================*/
float EntityBase::GetAbsoluteRotation() const
{
    auto owner = GetOwner();
    if( owner )
        return owner->GetAbsoluteRotation() + Rotation;
    else
        return Rotation;
}

/*=================================================================================================
    EntityBase::Invalidate()
    -> Called whenever the position of this Entity is updated
    -> Override to update the position of a Cocos Node along with the entity
    -> Dont forget to call the BaseClass method! Otherwise children wont be updated!
 =================================================================================================*/
void EntityBase::Invalidate()
{
    // Invalidate Children
    for( auto It = Children.begin(); It != Children.end(); It++ )
    {
        if( *It )
            (*It)->Invalidate();
    }
}

/*=================================================================================================
    EntityBase::LoadResources( function( void ) Callback ) -> int
    -> Method is called when resources can be preloaded
    -> IMPORTANT: Call the 'Callback' whenever a resource is done loading (regardless of success)
       and be sure to return the number of times 'Callback' will be called by this Entity
    -> If the returned int, and number of Callbacks dont match, it could cause loading to hang
 =================================================================================================*/
void EntityBase::RequireTexture( const std::string& InTex, std::function< void( cocos2d::Texture2D* ) > Callback )
{
    ResourceList[ InTex ] = Callback;
}

int EntityBase::LoadResources( const std::function<void ()> &Callback )
{
    int Output = 0;
    auto Cache = cocos2d::Director::getInstance()->getTextureCache();
    
    CC_ASSERT( Cache );
    
    for( auto It = ResourceList.begin(); It != ResourceList.end(); It++ )
    {
        auto UserCallback = It->second;
        
        Cache->addImageAsync( It->first, [ = ]( cocos2d::Texture2D* Texture )
        {
            // Run the callback provided by user
            if( UserCallback )
                UserCallback( Texture );
            
            // Run the callback for the loading system
            if( Callback )
                Callback();
            
        } );
        
        Output++;
    }
    
    return Output;
}

/*=================================================================================================
    EntityBase::PerformAction( Action* In, function( void ) OnComplete ) [INTERNAL]
    -> Checks if there is an ActionHandler bound to handle an incoming action
    -> 'OnComplete' will be called once the action is finished executing
 =================================================================================================*/
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

/*======================================================================================================
    EntityBase::SetActionCallback( string ActionId, function( Action*, function( void ) ) Callback )
    -> Binds an ActionHandler to be called when an incoming action matches the specified name
 ======================================================================================================*/
void EntityBase::SetActionCallback( const std::string& ActionId, std::function< void( Game::Action* In, std::function< void() > ) > Callback )
{
    ActionCallbacks[ ActionId ] = Callback;
}

/*=================================================================================================
    EntityBase::GetWorld()
    -> Returns the current Game World entity
 =================================================================================================*/
World* EntityBase::GetWorld() const
{
    auto world = World::GetWorld();
    CC_ASSERT( world );
    
    return world;
}
