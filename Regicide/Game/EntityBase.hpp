//
//  EntityBase.hpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/10/18.
//

#pragma once

#include "Numeric.h"
#include <vector>
#include <future>


namespace Game
{
    
    class EntityBase
    {
        
    public:
        
        inline uint32 GetEntityId() const { return EID; }
        inline std::string GetEntityName() const  { return EName; }
        
        void AddChild( EntityBase* inChild );
        bool HasChild( EntityBase* inChild );
        bool RemoveChild( EntityBase* inChild, bool bDestroy = true );
        
        inline EntityBase* GetOwner() { return Owner; }
        inline const EntityBase* GetOwner() const { return Owner; }
        
        virtual void AddToScene( cocos2d::Node* inNode );
        
        // Entity Hooks
        virtual void Initialize();
        virtual void SceneInit( cocos2d::Scene* inScene );
        virtual void PostInit();
        
        inline cocos2d::Scene* GetScene() { return LinkedScene; }
        inline cocos2d::Vec2 GetPosition() const { return Position; }
        inline float GetRotation() const { return Rotation; }
        
        // If subclass has a node, they can override this to move the node with the entitiy
        virtual void SetPosition( const cocos2d::Vec2& inPos );
        virtual void SetRotation( float inRot );
        
        cocos2d::Vec2 GetAbsolutePosition() const;
        float GetAbsoluteRotation() const;
        
        virtual void Invalidate();
        
    protected:
        
        virtual void Cleanup();
        EntityBase( const std::string& Name );
        ~EntityBase();
        
        cocos2d::Scene* LinkedScene;
        
        // Relative to parent
        cocos2d::Vec2 Position;
        float Rotation;
        
        virtual int LoadResources( const std::function< void() >& Callback );
        
        std::vector< EntityBase* >::iterator ChildBegin() { return Children.begin(); }
        std::vector< EntityBase* >::iterator ChildEnd() { return Children.end(); }
        
    private:
        
        uint32 EID;
        std::string EName;
        std::vector< EntityBase* > Children;
        EntityBase* Owner = nullptr;
        
        inline void SetEntityId( uint32 EntId ) { EID = EntId; }
        
        friend class SingleplayerLauncher;
        friend class IEntityManager;
    };
    
    typedef std::map< uint32, std::shared_ptr< EntityBase > >::iterator EntityIter;
    typedef std::map< uint32, std::shared_ptr< EntityBase > >::const_iterator ConstEntityIter;
    
    class IEntityManager
    {
        
    public:
        
        template< typename T >
        T* CreateEntity();
        
        template< typename T >
        T* CreateEntity( cocos2d::Scene* inScene );
        
        bool EntityExists( uint32 EntityId );
        bool DestroyEntity( uint32 EntityId, bool bIncludeChildren = true );
        bool DestroyEntity( EntityBase* EntityRef, bool bIncludeChildren = true );
        EntityBase* GetEntity( uint32 EntityId );
        
        inline EntityIter Begin() { return EntityStore.begin(); }
        inline EntityIter End() { return EntityStore.end(); }
        
        inline ConstEntityIter Begin() const { return EntityStore.begin(); }
        inline ConstEntityIter End() const { return EntityStore.end(); }
        
        template< typename T >
        T* GetEntity( uint32 EntityId );
        
        static IEntityManager& GetInstance();
        
        ~IEntityManager();
        
    private:
        
        std::map< uint32, std::shared_ptr< EntityBase > > EntityStore;
        
        // Explicitly disallow copying
        IEntityManager() {}
        IEntityManager( const IEntityManager& Other ) = delete;
        IEntityManager& operator= ( const IEntityManager& Other ) = delete;
    };
    
    static uint32 NextEntityId = 1;
    template< typename T >
    T* IEntityManager::CreateEntity()
    {
        static_assert( std::is_base_of< EntityBase, T >::value, "Attempt to create Entity with non-entity type" );
        
        // Find next available Id
        while( EntityStore.count( NextEntityId ) > 0 )
            NextEntityId++;
        
        // Create the entity
        uint32 EntId = NextEntityId;
        EntityStore[ EntId ] = std::make_shared< T >();
        EntityStore[ EntId ]->EID = EntId;
        
        // Return pointer to the entity
        return static_cast<T*>( EntityStore[ EntId ].get() );
    }
    
    // Adds entity directly to scene, this should be used at game runtime
    template< typename T >
    T* IEntityManager::CreateEntity( cocos2d::Scene* inScene )
    {
        // Create Entity
        T* NewEntity = CreateEntity< T >();
        
        // Call init hooks
        EntityBase* Base = static_cast< EntityBase* >( NewEntity );
        CC_ASSERT( Base );
        
        Base->Initialize();
        Base->SceneInit( inScene );
        
        return NewEntity;
    }
    
    template< typename T >
    T* IEntityManager::GetEntity( uint32 EntityId )
    {
        static_assert( std::is_base_of< EntityBase, T >::value, "Attempt to get entity casted to a non-entity type!" );
        
        auto Result = EntityStore.find( EntityId );
        if( Result == EntityStore.end() )
            return nullptr;
        
        if( Result->second )
        {
            return static_cast< T* >( Result->second.get() );
        }
        
        return nullptr;
    }
    
}