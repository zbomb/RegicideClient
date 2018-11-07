//
//  EventHub.cpp
//  Regicide
//
//  Created by Zachary Berry on 10/13/18.
//
#include "EventHub.h"
#include "cocos2d.h"

EventId EventHub::LastUsedId( EVENT_INVALID );
std::vector< GlobalListener > EventHub::GlobalListeners;
std::map< std::string, std::vector< EventHook > > EventHub::HookedEvents;

EventHook* EventHub::LookupHook( EventId Identifier, std::string EventName /* = nullptr */ )
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

bool EventHub::EventExists( EventId Identifier, std::string EventName /* = nullptr */ )
{
    return LookupHook( Identifier, EventName ) != nullptr;
}

EventId EventHub::Bind( std::string inEvent, std::function< bool( EventData* ) > inFunc, CallbackThread inThread /* = CallbackThread::EventOrigin */ )
{
    if( inEvent.empty() || inFunc == nullptr )
    {
        return EVENT_INVALID;
    }
    
    EventHook NewHook( ++LastUsedId, inThread, inFunc );

    HookedEvents[ inEvent ].push_back( NewHook );
    return NewHook.Identifier;
}

bool EventHub::UnBind( EventId Identifier, std::string CheckFirst /* = nullptr */ )
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

EventId EventHub::BindGlobal( std::function< bool( std::string, EventData* ) > inFunc, CallbackThread inThread /* = CallbackThread::EventOrigin */ )
{
    if( !inFunc )
    {
        return EVENT_INVALID;
    }
    
    GlobalListener newGlobal;
    newGlobal.Callback = inFunc;
    newGlobal.Identifier = ++LastUsedId;
    newGlobal.Thread = inThread;
    GlobalListeners.push_back( newGlobal );
    
    return newGlobal.Identifier;
}

bool EventHub::UnBindGlobal( EventId Identifier )
{
    for( std::vector< GlobalListener >::iterator Iter = GlobalListeners.begin(); Iter != GlobalListeners.end(); )
    {
        if( Iter->Identifier == Identifier )
        {
            GlobalListeners.erase( Iter );
            return true;
        }
        
        Iter++;
    }
    
    return false;
}

bool EventHub::GlobalExists( EventId Identifier )
{
    for( auto Iter = GlobalListeners.begin(); Iter != GlobalListeners.end(); Iter++ )
    {
        if( Iter->Identifier == Identifier )
        {
            return true;
        }
    }
    
    return false;
}

