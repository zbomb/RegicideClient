//
//  EventDataTypes.h
//  Regicide
//
//  Created by Zachary Berry on 10/14/18.
//

#ifndef EventDataTypes_h
#define EventDataTypes_h

#include "NetHeaders.h"

struct EventData
{
};

struct NullEventData : EventData
{};

struct ConnectEventData : EventData
{
    ConnectResult Result;
    
    ConnectEventData( ConnectResult inRes )
    : Result( inRes )
    {}
};

struct LoginEventData : EventData
{
    LoginResult Result;
    
    LoginEventData( LoginResult inRes )
        : Result( inRes )
    {}
};

struct NumericEventData : EventData
{
    int Data;
    
    NumericEventData( int inData )
        : Data( inData )
    {}
};

struct StringEventData : EventData
{
    std::string Data;
    
    StringEventData( std::string& inData )
        : Data( inData )
    {}
};

struct PacketEventData : EventData
{
    FIncomingPacket Packet;
    
    PacketEventData( FIncomingPacket& inPack )
    : Packet( inPack )
    {
    }
};

#endif /* EventDataTypes_h */
