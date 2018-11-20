//
//    EventHub.cpp
//    Regicide Mobile
//
//    Created: 10/13/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#include "EventHub.hpp"
#include "cocos2d.h"

EventId EventHub::LastUsedId( EVENT_INVALID );
std::map< std::string, std::vector< EventHook > > EventHub::HookedEvents;

EventHook* EventHub::LookupHook( EventId Identifier, const std::string& EventName /* = nullptr */ )
{
    // Check if we were given an EventType
    if( !EventName.empty() )
    {
        if( HookedEvents.count( EventName ) > 0 )
        {
            auto& HookList = HookedEvents.at( EventName );
            for( auto& HookData : HookList )
            {
                if( HookData.Identifier == Identifier )
                {
                    return &HookData;
                }
            }
        }
    }
    
    // Hook wasnt found where caller specified, or caller didnt specify at all
    for( auto Iter = HookedEvents.begin(); Iter != HookedEvents.end(); Iter++ )
    {
        for( auto& HookData : Iter->second )
        {
            if( HookData.Identifier == Identifier )
            {
                return &HookData;
            }
        }
    }
    
    return nullptr;
}

bool EventHub::EventExists( EventId Identifier, const std::string& EventName /* = nullptr */ )
{
    return LookupHook( Identifier, EventName ) != nullptr;
}

EventId EventHub::Bind( const std::string& inEvent, std::function< bool( EventData* ) > inFunc, CallbackThread inThread /* = CallbackThread::EventOrigin */ )
{
    if( inEvent.empty() || inFunc == nullptr )
    {
        return EVENT_INVALID;
    }
    
    EventHook NewHook( ++LastUsedId, inThread, inFunc );

    HookedEvents[ inEvent ].push_back( NewHook );
    return NewHook.Identifier;
}

bool EventHub::UnBind( EventId Identifier, const std::string& CheckFirst /* = nullptr */ )
{
    if( !CheckFirst.empty() )
    {
        if( HookedEvents.count( CheckFirst ) > 0 )
        {
            auto& HookList = HookedEvents.at( CheckFirst );
            
            for( std::vector< EventHook >::iterator Iter = HookList.begin(); Iter != HookList.end(); )
            {
                if( Iter->Identifier == Identifier )
                {
                    HookList.erase( Iter );
                    return true;
                }

                Iter++;
            }
        }
    }
    
    // We didnt find the iter where the caller specified, or it wasnt specified
    for( auto& HookList : HookedEvents )
    {
        for( std::vector< EventHook >::iterator Iter = HookList.second.begin(); Iter != HookList.second.end(); )
        {
            if( Iter->Identifier == Identifier )
            {
                HookList.second.erase( Iter );
                return true;
            }
            
            Iter++;
        }
    }
    
    return false;
}
