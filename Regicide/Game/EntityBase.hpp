//
//  EntityBase.hpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/10/18.
//

#pragma once

#include "Numeric.h"
#include <vector>


namespace Game
{
    
    class EntityBase
    {
        
    public:
        
        inline uint32 GetEntityId() const { return EID; }
        inline std::string GetEntityName() const  { return EName; }
        
    protected:
        
        virtual void Cleanup();
        EntityBase( const std::string& Name );
        ~EntityBase();
        
        // TODO: Add any entity hooks here
        
    private:
        
        uint32 EID;
        std::string EName;
        
        inline void SetEntityId( uint32 EntId ) { EID = EntId; }
        friend class IEntityManager;
    };
    
    class IEntityManager
    {
        
    public:
        
        template< typename T >
        T* CreateEntity();
        
        bool EntityExists( uint32 EntityId );
        bool DestroyEntity( uint32 EntityId );
        bool DestroyEntity( EntityBase* EntityRef );
        EntityBase* GetEntity( uint32 EntityId );
        
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
