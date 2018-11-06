//
//  EventDataTypes.h
//  Regicide
//
//  Created by Zachary Berry on 10/14/18.
//

#ifndef EventDataTypes_h
#define EventDataTypes_h


struct EventData
{
};

struct NullEventData : EventData
{};

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

#endif /* EventDataTypes_h */
