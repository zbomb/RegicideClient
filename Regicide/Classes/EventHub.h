//
//  EventHub.h
//  Regicide
//
//  Created by Zachary Berry on 10/13/18.
//
#pragma once

#include "NetHeaders.h"
#include "EventDataTypes.h"
#include "RegCloud.h"

typedef uint32 EventId;
#define EVENT_INVALID 0

#define EVENT_SAFE_UNBIND( inEventId ) if( inEventId != EVENT_INVALID ) { EventHub::UnBind( inEventId ); }

enum CallbackThread
{
    Game,
    Network,
    AsyncPool,
    EventOrigin
};

struct EventHook
{
    EventHook( EventId inId, CallbackThread inThread, std::function< bool( EventData* ) > inCallback )
        : Identifier( inId ), Thread( inThread ), Callback( inCallback )
    {
    }
    
    EventId Identifier;
    CallbackThread Thread;
    std::function< bool( EventData* ) > Callback;
};

struct GlobalListener
{
    EventId Identifier;
    CallbackThread Thread;
    std::function< bool( std::string, EventData* ) > Callback;
};

class EventHub
{
public:
    
    static bool EventExists( EventId inId, std::string EventName = nullptr );
    static bool UnBind( EventId inId, std::string EventName = nullptr );
    
    static EventId Bind( std::string EventName, std::function< bool( EventData* ) > inFunc, CallbackThread inThread = CallbackThread::EventOrigin );
    
    static bool Execute( std::string EventName, EventData& inData );
    
    template< typename T >
    static bool Execute( std::string EventName, T& inData );
    
    static EventId BindGlobal( std::function< bool( std::string, EventData* ) > inFunc, CallbackThread inThread = CallbackThread::EventOrigin );
    static bool GlobalExists( EventId Identifier );
    static bool UnBindGlobal( EventId Identifier );
    
private:
    
    static std::vector< GlobalListener > GlobalListeners;
    static std::map< std::string, std::vector< EventHook > > HookedEvents;
    static EventHook* LookupHook( EventId Identifier, std::string CheckFirst = nullptr );
    
    static EventId LastUsedId;
};

template< typename T >
bool EventHub::Execute( std::string inEvent, T& inData )
{
    using namespace cocos2d;
    Director* dir = Director::getInstance();
    Scheduler* sch = dir ? dir->getScheduler() : nullptr;
    RegCloud* RegSys = RegCloud::Get();
    auto* async = AsyncTaskPool::getInstance();
    bool bRet = false;
    
    if( HookedEvents.count( inEvent ) > 0 )
    {
        for( auto& HookEntry : HookedEvents.at( inEvent ) )
        {
            if( HookEntry.Callback )
            {
                if( HookEntry.Thread == CallbackThread::EventOrigin )
                {
                    HookEntry.Callback( (EventData*) &inData );
                    bRet = true;
                }
                else if( HookEntry.Thread == CallbackThread::Game )
                {
                    if( !sch )
                    {
                        log( "[RegEvent] Failed to execute an event hook on the game thread! Invalid director/scheduler reference" );
                        continue;
                    }
                    
                    sch->performFunctionInCocosThread( [ &, inData ]()
                                                      { HookEntry.Callback( (EventData*) &inData ); }
                                                      );
                    bRet = true;
                }
                else if( HookEntry.Thread == CallbackThread::Network )
                {
                    if( !RegSys )
                    {
                        log( "[RegEvent] Failed to execute an event hook on the network io thread! Invalid RegSys reference!" );
                        continue;
                    }
                    
                    RegSys->GetContext().post( [ &, inData ]()
                                              { HookEntry.Callback( (EventData*) &inData ); }
                                              );
                    bRet = true;
                }
                else if( HookEntry.Thread == CallbackThread::AsyncPool )
                {
                    if( !async )
                    {
                        log( "[RegEvent] Failed to execute an event hook in the async task pool! Invalid async pool reference!" );
                        continue;
                    }
                    
                    async->enqueue(AsyncTaskPool::TaskType::TASK_OTHER,
                                   nullptr, nullptr,
                                   [ &, inData ] ()
                                   {
                                       HookEntry.Callback( (EventData*) &inData );
                                   });
                    bRet = true;
                }
            }
        }
    }
    
    // Call Global Listeners
    for( auto Iter = GlobalListeners.begin(); Iter != GlobalListeners.end(); Iter ++ )
    {
        if( Iter->Thread == CallbackThread::AsyncPool )
        {
            if( !async )
            {
                log( "[RegEvent] Failed to execute an event hook in the async task pool! Invalid async pool reference!" );
                continue;
            }
            async->enqueue( AsyncTaskPool::TaskType::TASK_OTHER,
                           nullptr, nullptr,
                           [ &, inData ]
                           {
                               Iter->Callback( inEvent, (EventData*) &inData );
                           });
            bRet = true;
        }
        else if( Iter->Thread == CallbackThread::Network )
        {
            if( !RegSys )
            {
                log( "[RegEvent] Failed to execute callback for a global event listener on the network IO thread! RegSys reference invalid!" );
                continue;
            }
            
            RegSys->GetContext().post( [ &, inData ]
                                      {
                                          Iter->Callback( inEvent, (EventData*) &inData );
                                      });
            bRet = true;
        }
        else if( Iter->Thread == CallbackThread::Game )
        {
            if( !sch )
            {
                log( "[RegEvent] Failed to execute callback for a global event listener on the game thread! Scheduler reference invalid!" );
                continue;
            }
            
            sch->performFunctionInCocosThread( [ &, inData ]
                                              {
                                                  Iter->Callback( inEvent, (EventData*) &inData );
                                              });
            bRet = true;
        }
        else if( Iter->Thread == CallbackThread::EventOrigin )
        {
            Iter->Callback( inEvent, (EventData*) &inData );
            bRet = true;
        }
    }
    
    return bRet;
}
