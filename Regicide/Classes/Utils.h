//
//  Utils.h
//  Regicide
//
//  Created by Zachary Berry on 10/16/18.
//

#pragma once
#include "external/json/document.h"
#include "RegicideAPI/Account.h"
#include <memory>


// Json Types
typedef rapidjson::GenericObject<false,rapidjson::GenericValue<rapidjson::UTF8<>>> JsonObject;
typedef rapidjson::GenericArray<false, rapidjson::GenericValue<rapidjson::UTF8<>>> JsonArray;

namespace Regicide
{

    class Utils
    {
        
    public:
        
        static bool ReadAccount( JsonObject Input, std::shared_ptr< UserAccount >& Output );
        static bool ReadAccountInfo( JsonObject& Input, AccountInfo& Output );
        static bool ReadCards( JsonArray& Input, std::vector< Card >& Output );
        static bool ReadDecks( JsonArray& Input, std::vector< Deck >& Output );
        static bool ReadAchievements( JsonArray& Input, std::vector< Achievement >& Output );
        
        static bool WriteAccount( std::shared_ptr< UserAccount >& Input, std::string& Output );
        static bool WriteAccountInfo( AccountInfo& Input, rapidjson::Value& Output, rapidjson::MemoryPoolAllocator<>& Allocator );
        static bool WriteCards( std::vector< Card >& Input, rapidjson::Value& Output, rapidjson::MemoryPoolAllocator<>& Allocator );
        static bool WriteDecks( std::vector< Deck >& Input, rapidjson::Value& Output, rapidjson::MemoryPoolAllocator<>& Allocator );
        static bool WriteAchievements( std::vector< Achievement >& Input, rapidjson::Value& Output, rapidjson::MemoryPoolAllocator<>& Allocator );
        
#if CC_TARGET_PLATFORM == CC_PLATFORM_IOS
        static std::string GetSandboxDirectory();
#endif
        
        static std::string GetPlatformDir();
        static std::string GetContentDir();
        static bool IsFirstTimeLaunch( int Version = 0 );
        
        template< typename T >
        static inline T Clamp( T In, T Min, T Max ) { return In < Min ? Min : In > Max ? Max : In; }
        
        template<typename ... Args>
        static std::string FormatString( const std::string& format, Args ... args )
        {
            size_t size = snprintf( nullptr, 0, format.c_str(), args ... ) + 1;
            std::unique_ptr<char[]> buf( new char[ size ] );
            snprintf( buf.get(), size, format.c_str(), args ... );
            return std::string( buf.get(), buf.get() + size - 1 );
        }
        
        static std::string ByteString( uint64 Bytes );
        
    };
}
