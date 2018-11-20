//
//    EventHub.hpp
//    Regicide Mobile
//
//    Created: 10/13/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#pragma once

#include "EventDataTypes.hpp"
#include "LuaEngine.hpp"

typedef uint32_t EventId;
#define EVENT_INVALID 0

#define EVENT_SAFE_UNBIND( inEventId ) if( inEventId != EVENT_INVALID ) { EventHub::UnBind( inEventId ); }

enum class CallbackThread
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

class EventHub
{
public:
    
    static bool EventExists( EventId inId, const std::string& EventName = nullptr );
    static bool UnBind( EventId inId, const std::string& EventName = nullptr );
    
    static EventId Bind( const std::string& EventName, std::function< bool( EventData* ) > inFunc, CallbackThread inThread = CallbackThread::EventOrigin );
    
    template< typename T = NullEventData >
    static bool Execute( const std::string& EventName, const T& inData = NullEventData() );
    
    
private:
    
    static std::map< std::string, std::vector< EventHook > > HookedEvents;
    static EventHook* LookupHook( EventId Identifier, const std::string& CheckFirst = nullptr );
    
    static EventId LastUsedId;
};

template< typename T >
bool EventHub::Execute( const std::string& inEvent, const T& inData )
{
    using namespace cocos2d;
    Director* dir = Director::getInstance();
    Scheduler* sch = dir ? dir->getScheduler() : nullptr;
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

    // Call Lua
    // First, we need to convert the class to a LuaRef
    auto Engine = Regicide::LuaEngine::GetInstance();
    luabridge::LuaRef LuaData( Engine->State() );
    inData.ToLua( LuaData );
    
    if( Engine->ExecuteHook( inEvent, LuaData ) )
        bRet = true;
    
    return bRet;

}
