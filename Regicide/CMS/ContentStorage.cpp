//
//  ContentStorage.cpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/3/18.
//

#include "ContentStorage.hpp"
#include "Utils.h"
#include "platform/CCFileUtils.h"
#include "external/json/document.h"
#include "external/json/prettywriter.h"

using namespace Regicide;
using namespace cocos2d;
using namespace rapidjson;


static ContentStorage* s_Singleton = nullptr;
ContentStorage* ContentStorage::GetInstance()
{
    ForceInit();
    return s_Singleton;
}

ContentStorage::~ContentStorage()
{
    s_Singleton = nullptr;
}

void ContentStorage::ForceInit()
{
    if( !s_Singleton )
    {
        s_Singleton = new( std::nothrow ) ContentStorage();
        s_Singleton->Init();
    }
}


void ContentStorage::Init()
{
    std::string ContentRoot = Utils::GetContentDir();
    
    // Create needed subdirectories
    auto file = FileUtils::getInstance();
    if( !file->isDirectoryExist( ContentRoot + "/img" ) )
    {
        file->createDirectory( ContentRoot + "/img" );
    }
    
    if( !file->isDirectoryExist( ContentRoot + "/scripts" ) )
    {
        file->createDirectory( ContentRoot + "/scripts" );
    }
    
    if( !file->isDirectoryExist( ContentRoot + "/data" ) )
    {
        file->createDirectory( ContentRoot + "/data" );
    }
    
    if( !file->isDirectoryExist( ContentRoot + "/blocks" ) )
    {
        file->createDirectory( ContentRoot + "/blocks" );
    }
    
    if( !file->isDirectoryExist( ContentRoot + "/download" ) )
    {
        file->createDirectory( ContentRoot + "/download" );
    }
}

bool ContentStorage::ReadLocalBlocks( std::vector< LocalBlock >& Output )
{
    Output.clear();
    
    auto file = FileUtils::getInstance();
    if( !file )
    {
        cocos2d::log( "[CMS] Failed to get file utils instance to read local content block information!" );
        return false;
    }
    
    auto allFiles = file->listFiles( Utils::GetContentDir() + "/blocks" );
    for( auto F : allFiles )
    {
        if( file->getFileExtension( F ) != ".block" )
        {
            continue;
        }
        
        // Load and read file
        auto data = file->getDataFromFile( F );
        if( data.isNull() )
            continue;
        
        LocalBlock newBlock;
        if( !ParseBlock( data.getBytes(), data.getSize(), newBlock ) )
        {
            cocos2d::log( "[CMS] Error while parsing local block!" );
            continue;
        }
        
        Output.push_back( newBlock );
    }
    
    return true;
}


bool ContentStorage::ParseBlock( uint8* Data, ssize_t Size, LocalBlock& Block )
{
    Document Doc;
    if( Doc.Parse( (char*)Data, Size ).HasParseError() )
    {
        cocos2d::log( "[CMS] Error while parsing block!" );
        return false;
    }
    
    if( !Doc.HasMember( "Id" ) || !Doc.HasMember( "Hash" ) || !Doc.HasMember( "Files" ) )
    {
        cocos2d::log( "[CMS] Local block is missing info!" );
        return false;
    }
    
    Block.Identifier = Doc[ "Id" ].GetString();
    Block.Hash = Doc[ "Hash" ].GetString();
    
    auto files = Doc[ "Files" ].GetArray();
    for( auto It = files.begin(); It != files.end(); It++ )
    {
        if( !It->IsString() )
            continue;
        
        Block.Files.push_back( It->GetString() );
    }
    
    return true;
}


bool ContentStorage::BlockExists( const std::string& Identifier )
{
    std::string SearchStr( Identifier );
    std::transform( SearchStr.begin(), SearchStr.end(), SearchStr.begin(), ::tolower );
    
    auto file = FileUtils::getInstance();
    return file->isFileExist( Utils::GetContentDir() + "/blocks/" + Identifier + ".block" );
}


bool ContentStorage::ReadLocalBlock( const std::string& Identifier, LocalBlock &Output )
{
    std::string SearchStr( Identifier );
    std::transform( SearchStr.begin(), SearchStr.end(), SearchStr.begin(), ::tolower );
    
    auto file = FileUtils::getInstance();
    auto data = file->getDataFromFile( Utils::GetContentDir() + "/blocks/" + Identifier + ".block" );
    
    if( data.isNull() )
        return false;
    
    return ParseBlock( data.getBytes(), data.getSize(), Output );
    
}


bool ContentStorage::DeleteFile( const std::string& RelativePath )
{
    auto file = FileUtils::getInstance();
    return file->removeFile( Utils::GetContentDir() + "/" + RelativePath );
}


bool ContentStorage::WriteFile( const std::string& Path, const std::vector< uint8 >& VecData )
{
    Data writeData;
    writeData.fastSet( (uint8*)VecData.data(), VecData.size() );
    bool ret = FileUtils::getInstance()->writeDataToFile( writeData, Utils::GetContentDir() + "/" + Path );
    writeData.fastSet( nullptr, 0 ); // Prevent Data object from deleting vector internal buffer on destruction
    return ret;
}


bool ContentStorage::WriteFile( const std::string& Path, const std::string& Data )
{
    return FileUtils::getInstance()->writeStringToFile( Data, Utils::GetContentDir() + "/" + Path );
}


bool ContentStorage::FileExists( const std::string& RelativePath )
{
    auto file = FileUtils::getInstance();
    return file->isFileExist( Utils::GetContentDir() + "/" + RelativePath );
}


std::vector<uint8> ContentStorage::ReadFile( const std::string& Path )
{
    auto Data = FileUtils::getInstance()->getDataFromFile( Utils::GetContentDir() + "/" + Path );
    
    if( Data.isNull() )
        return std::vector< uint8 >();
    
    return std::vector< uint8 >( Data.getBytes(), Data.getBytes() + Data.getSize() );
}


std::string ContentStorage::ReadFileStr( const std::string& Path )
{
    return FileUtils::getInstance()->getStringFromFile( Utils::GetContentDir() + "/" + Path );
}


bool ContentStorage::WriteLocalBlock( LocalBlock &Input )
{
    std::transform( Input.Identifier.begin(), Input.Identifier.end(), Input.Identifier.begin(), ::tolower );

    // Now we need to save the header for this block, so we can read it in the future
    Document Doc;
    Doc.SetObject();
    Doc.AddMember( "Id", StringRef( Input.Identifier ), Doc.GetAllocator() );
    Doc.AddMember( "Hash", StringRef( Input.Hash ), Doc.GetAllocator() );
    
    rapidjson::Value FileList( kArrayType );
    for( auto& F : Input.Files )
    {
        FileList.PushBack( StringRef( F ), Doc.GetAllocator() );
    }
    
    Doc.AddMember( "Files", FileList, Doc.GetAllocator() );
    
    // Now, we need to write this info file to disk
    StringBuffer Buffer;
    Writer< StringBuffer > writer( Buffer );
    Doc.Accept( writer );
    
    std::string InfoStr = Buffer.GetString();
    auto file = FileUtils::getInstance();
    return file->writeStringToFile( InfoStr, Utils::GetContentDir() + "/blocks/" + Input.Identifier + ".block" );
}
