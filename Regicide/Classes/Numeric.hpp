//
//    Numeric.hpp
//    Regicide Mobile
//
//    Created: 11/1/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#pragma once

#include <stdint.h>

typedef uint8_t     uint8;
typedef int8_t      int8;
typedef uint16_t    uint16;
typedef int16_t     int16;
typedef uint32_t    uint32;
typedef int32_t     int32;
typedef uint64_t    uint64;
typedef int64_t     int64;


namespace Math
{
    template< typename T >
    inline T Clamp( T Value, T Min, T Max )
    {
        return Value > Min ? ( Value < Max ? Value : Max ) : Min;
    }
    
    template< typename T >
    inline T Max( T A, T B )
    {
        return A >= B ? A : B;
    }
    
    template< typename T >
    inline T Max( T A, T B, T C )
    {
        if( A >= B && A >= C )
            return A;
        else if( B >= A && B >= C )
            return B;
        else
            return C;
    }
    
    template< typename T >
    inline T Min( T A, T B )
    {
        return A <= B ? A : B;
    }
    
    template< typename T >
    inline T Min( T A, T B, T C )
    {
        if( A <= B && A <= C )
            return A;
        else if( B <= A && B <= C )
            return B;
        else
            return C;
    }
    
    template< typename T >
    inline T SDiv( T Num, T Den )
    {
        return Den != (T)0 ? Num / Den : (T)0;
    }
    
    template< typename T >
    inline T Abs( T In )
    {
        return In < (T)0 ? -In : In;
    }
    
    
}
