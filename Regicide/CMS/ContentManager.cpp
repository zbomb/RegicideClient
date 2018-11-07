//
//  ContentManager.cpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/2/18.
//

#include "ContentManager.hpp"
#include "external/json/document.h"
#include "external/json/prettywriter.h"
#include "network/CCDownloader.h"
#include "IContentSystem.hpp"
#include <functional>
#include "CryptoLibrary.h"
#include "gzip/decompress.hpp"


using namespace Regicide;
using namespace cocos2d;
using namespace rapidjson;
using namespace std::placeholders;


static ContentManager* s_Singleton = nullptr;
ContentManager* ContentManager::GetInstance()
{
    ForceInit();
    return s_Singleton;
}

ContentManager::~ContentManager()
{
    s_Singleton = nullptr;
}

void ContentManager::ForceInit()
{
    if( !s_Singleton )
    {
        s_Singleton = new (std::nothrow) ContentManager();
        s_Singleton->Init();
    }
}

void ContentManager::Init()
{
    Hints.countOfMaxProcessingTasks = 3;
    Hints.tempFileNameSuffix = "dload_";
    Hints.timeoutInSeconds = 10;
    DownloadManager.reset( new network::Downloader( Hints ) );
    
    using namespace std::placeholders;
    DownloadManager->setOnTaskProgress( std::bind( &ContentManager::Internal_OnProgress, this, _1, _2, _3, _4 ) );
}


void ContentManager::CheckForUpdates()
{
    if( bUpdateInProgress )
        return;
    
    // Reset State
    FailureCount = 0;
    bUpdateInProgress = true;
    TotalProgress = 0;
    UpdateSize = 0;
    
    cocos2d::log( "[CMS] Checking for updates..." );
    
    // Download content block manifest
    CurrentDownload = DownloadManager->createDownloadDataTask( "https://cdn.regicidemobile.com/manifest", "manifest" );
    
    DownloadManager->onDataTaskSuccess = [ this ] ( const network::DownloadTask& Task, std::vector<unsigned char>& Data )
    {
        if( Data.size() == 0 )
        {
            if( ++FailureCount >= 5 )
            {
                cocos2d::log( "[CMS] Failed to download content manifest after 5 tries! (Empty)" );
                cocos2d::log( "[CMS] Content update complete (FAILED)" );
                
                this->Internal_OnUpdateChecked( false, true, "Failed to download manifest" );
            }
            else
            {
                cocos2d::log( "[CMS] Downloaded manifest was empty.. retrying (Attempt %d)", FailureCount );
                CurrentDownload = DownloadManager->createDownloadDataTask( "https://cdn.regicidemobile.com/manifest", "manifest" );
            }
            
            return;
        }
        
        // Now, we need to read the manifest entries
        MasterManifest.Entries.clear();
        if( !this->ReadManifest( Data, MasterManifest, UpdateSize ) )
        {
            if( ++FailureCount >= 5 )
            {
                cocos2d::log( "[CMS] Failed to read content manifest after 5 tries! (Parse)" );
                cocos2d::log( "[CMS] Content update complete (FAILED)" );
                
                this->Internal_OnUpdateChecked( false, true, "Failed to read manifest" );
            }
            else
            {
                cocos2d::log( "[CMS] Downloaded manifest was unable to be parsed.. retrying (Attempt %d)", FailureCount );
                CurrentDownload = DownloadManager->createDownloadDataTask( "https://cdn.regicidemobile.com/manifest", "manifest" );
            }
            
            return;
        }
        
        // Now, we need to determine if any of the local content is out of date
        // For every content block we have on disk, there is an entry written in data/blocks
        // So now were going to load all of that information to compare
        auto cs = IContentSystem::GetStorage();
        std::vector< LocalBlock > LocalBlocks;
        
        cs->ReadLocalBlocks( LocalBlocks );

        // Now, we need to build a list of blocks missing/out of date, and a list that can be deleted
        std::vector< LocalBlock > BlocksToDelete;
        std::vector< ManifestEntry > BlocksToUpdate;
        
        this->ProcessManifest( MasterManifest.Entries, LocalBlocks, BlocksToUpdate, BlocksToDelete );
        
        bool bNeedsUpdate = !BlocksToDelete.empty() || !BlocksToUpdate.empty();
        if( bNeedsUpdate )
        {
            cocos2d::log( "[CMS] Updates found! %lu update(s) will be installed, and %lu will be deleted", BlocksToUpdate.size(), BlocksToDelete.size() );
        }
        else
        {
            cocos2d::log( "[CMS] Content is up-to-date!" );
        }
        
        this->Internal_OnUpdateChecked( bNeedsUpdate, false, std::string() );
        
        // Delete old content
        if( BlocksToDelete.size() > 0 )
        {
            this->OldContent = BlocksToDelete;
        }
        
        // Begin downloading new content
        if( BlocksToUpdate.size() > 0 )
        {
            this->NeededContent = BlocksToUpdate;
        }
    };
    
    DownloadManager->onTaskError = [ this ] ( const network::DownloadTask& task, int errorCode, int errorCodeInternal, const std::string& errorStr )
    {
        if( ++FailureCount >= 5 )
        {
            cocos2d::log( "[CMS] Failed to download manifest after 5 tries! Aborting" );
            this->Internal_OnUpdateChecked( false, true, "Failed to download manifest" );
        }
        else
        {
            cocos2d::log( "[CMS] Failed to download manifest from the Regicide CDN!" );
            CurrentDownload = DownloadManager->createDownloadDataTask( "https://cdn.regicidemobile.com/manifest", "manifest" );
        }
    };
}


void ContentManager::ProcessUpdates()
{
    if( NeededContent.size() == 0 && OldContent.size() == 0 )
    {
        cocos2d::log( "[CMS] Failed to process updates.. there are none queued." );
        return;
    }
    
    cocos2d::log( "[CMS] Starting update..." );
    
    DownloadManager->onDataTaskSuccess = std::bind( &ContentManager::OnBlockDownload, this, _1, _2 );
    DownloadManager->onTaskError = std::bind( &ContentManager::OnBlockDownloadFail, this, _1, _2, _3, _4 );
    DownloadManager->onTaskProgress = std::bind( &ContentManager::Internal_OnProgress, this, _1, _2, _3, _4 );
    
    // Delete Old Content
    if( OldContent.size() > 0 )
        DeleteContent( OldContent );
    
    // Update content
    if( NeededContent.size() > 0 )
    {
    // Reset Failure Count
        FailureCount = 0;
        auto& FirstDownload = NeededContent.back();
        CurrentDownload = DownloadManager->createDownloadDataTask( "https://cdn.regicidemobile.com/" + FirstDownload.URL, FirstDownload.Identifier );
        Internal_OnProgress( *CurrentDownload.get(), 0, 0, FirstDownload.Size );
    }
}


void ContentManager::OnBlockDownload( const network::DownloadTask &Task, std::vector<unsigned char> &Data )
{
    ManifestEntry ThisDownload;
    bool bVerified = false;
    bool bGZipFailed = false;
    bool bComplete = true;
    uint64 DownloadSize = 0;
    std::vector< uint8 > Decompressed;
    
    if( NeededContent.size() == 0 )
    {
        bComplete = true;
    }
    else
    {
        ThisDownload = NeededContent.back();
        
        // Decompress using GZip
        gzip::Decompressor decomp;
        try
        {
            decomp.decompress( Decompressed, (char*)Data.data(), Data.size() );
        }
        catch( ... )
        {
            bGZipFailed = true;
        }
        
        // Clear original buffer
        DownloadSize = Data.size();
        Data.clear();
        
        if( bGZipFailed || Decompressed.size() == 0 )
        {
            if( ++FailureCount >= 3 )
            {
                cocos2d::log( "[CMS] Error! Download block failed! (GZip) %s", Task.requestURL.c_str() );
                NeededContent.pop_back();
                FailureCount = 0;
            }
            else
            {
                cocos2d::log( "[CMS] Error! Failed to decompress downloaded block! %s", Task.requestURL.c_str() );
            }
        }
        else
        {
            // Verify block integrity
            auto LocalHash = CryptoLibrary::SHA256( Decompressed );
            
            if( LocalHash.size() < 32 )
            {
                if( ++FailureCount >= 3 )
                {
                    cocos2d::log( "[CMS] Error! Download block failed (Hash)! %s", Task.requestURL.c_str() );
                    NeededContent.pop_back();
                    FailureCount = 0;
                }
                else
                {
                    cocos2d::log( "[CMS] Error! Couldnt compute hash of downloaded block! (%d attempts)", FailureCount );
                }
            }
            else
            {
                std::string LocalHashStr = CryptoLibrary::Base64Encode( LocalHash );
                
                if( LocalHashStr.compare( ThisDownload.Hash ) != 0 )
                {
                    if( ++FailureCount >= 3 )
                    {
                        cocos2d::log( "[CMS] Error! Couldnt validate the downloaded block after multiple attempts. %s", Task.requestURL.c_str() );
                        NeededContent.pop_back();
                        FailureCount = 0;
                    }
                    else
                    {
                        cocos2d::log( "[CMS] Warning! Failed to validate hash for downloaded block '%s'", ThisDownload.URL.c_str() );
                    }
                }
                else
                {
                    NeededContent.pop_back();
                    bVerified = true;
                }
            }
        }
    }
    
    // Reset failure counter for next download if this was successful
    if( bVerified )
    {
        cocos2d::log( "[CMS] Update '%s' downloaded", ThisDownload.URL.c_str() );
        FailureCount = 0;
        TotalProgress += DownloadSize;
    }
    
    // Start next download
    if( NeededContent.size() > 0 )
    {
        auto& NextDownload = NeededContent.back();
        CurrentDownload = DownloadManager->createDownloadDataTask( "https://cdn.regicidemobile.com/" + NextDownload.URL, NextDownload.Identifier );
        bComplete = false;
        
        Internal_OnProgress( *CurrentDownload.get(), 0, 0, NextDownload.Size );
    }
    else
    {
        Internal_OnProgress( network::DownloadTask(), 0, 0, 0 );
    }
    
    // If this block wasnt verified, then dont process it
    if( bVerified )
    {
        // Read the block
        ContentBlock newBlock;
        BlockReader Reader;
        
        if( !Reader.ReadHeader( newBlock, Decompressed ) )
        {
            // Hash is verified, so if the header is bad, then redownloading wont solve the issue
            cocos2d::log( "[CMS] Error reading a block header!" );
        }
        else
        {
            // Check if there is already a local content block with this identifier
            auto cs = IContentSystem::GetStorage();
            LocalBlock local;
            
            if( cs->ReadLocalBlock( newBlock.Identifier, local ) )
            {
                // We are just checking for files in the local block, that arent in the master block
                std::vector< std::string > OldFiles;
                for( auto& F : local.Files )
                {
                    for( auto& M : newBlock.Entries )
                    {
                        if( M.File.compare( F ) == 0 )
                        {
                            OldFiles.push_back( F );
                            break;
                        }
                    }
                }
                
                // Delete old files
                for( auto& F : OldFiles )
                {
                    if( !cs->DeleteFile( F ) )
                        cocos2d::log( "[CMS] Failed to delete uneeded content! %s", F.c_str() );
                }
            }
            
            // Now were going to write all the content to disk
            for( auto& Entry : newBlock.Entries )
            {
                std::vector< uint8 > FileData( Entry.Begin, Entry.End );
                if( !cs->WriteFile( Entry.File, FileData ) )
                {
                    cocos2d::log( "[CMS] Failed to write new/updated content! %s", Entry.File.c_str() );
                }
            }
            
            // Create local block to track this content
            LocalBlock newLocal;
            newLocal.Identifier = newBlock.Identifier;
            newLocal.Hash = ThisDownload.Hash;
            
            for( auto& F : newBlock.Entries )
                newLocal.Files.push_back( F.File );
            
            if( !cs->WriteLocalBlock( newLocal ) )
                cocos2d::log( "[ERROR] Failed to write block info for updated content! %s", newBlock.Identifier.c_str() );
            else
                cocos2d::log( "[CMS] Update '%s' installed!", ThisDownload.URL.c_str() );
        }
    }
    
    if( bComplete )
    {
        cocos2d::log( "[CMS] Finished updating!" );
        Internal_OnComplete( true, TotalProgress );
    }
}


void ContentManager::OnBlockDownloadFail( const network::DownloadTask &task, int errorCode, int errorCodeInternal, const std::string &errorStr )
{
    cocos2d::log( "[CMS] Download block failed! %s", errorStr.c_str() );
    
    // Check for retry
    if( ++FailureCount >= 3 )
    {
        cocos2d::log( "[CMS] Block '%s' couldnt be downloaded! %d attempts were made.", task.requestURL.c_str(), 3 );
        NeededContent.pop_back();
        FailureCount = 0;
    }
    else
    {
        cocos2d::log( "[CMS] Download error! Attempting again.. (%d failures) %s", FailureCount, task.requestURL.c_str() );
    }
    
    if( NeededContent.size() > 0 )
    {
        auto& NextDownload = NeededContent.back();
        CurrentDownload = DownloadManager->createDownloadDataTask( "https://cdn.regicidemobile.com/" + NextDownload.URL, NextDownload.Identifier );
    }
    else
    {
        cocos2d::log( "[CMS] Content update is complete!" );
        Internal_OnComplete( true );
    }
}


void ContentManager::DeleteContent( std::vector< LocalBlock > &Blocks )
{
    auto cs = IContentSystem::GetStorage();

    for( auto B : Blocks )
    {
        // Delete all files this block owns
        for( auto F : B.Files )
        {
            // TODO: Handle Failures?
            if( cs->FileExists( F ) && !cs->DeleteFile( F ) )
                cocos2d::log( "[CMS] Failed to delete file from outdated block (%s)! %s", B.Identifier.c_str(), F.c_str() );
        }
        
        // Delete the block information
        // TODO: Handle failures?
        if( !cs->DeleteFile( "blocks/" + B.Identifier + ".block" ) )
            cocos2d::log( "[CMS] Failed to delete .block file for outdated block! (%s)", B.Identifier.c_str() );
    }
}


void ContentManager::ProcessManifest( std::vector< ManifestEntry >& Entries, std::vector< LocalBlock >& LocalBlocks, std::vector< ManifestEntry >& UpdateBlocks, std::vector< LocalBlock >& ObsoleteBlocks )
{
    // First, were going to check for blocks that we have locally, but arent in the master manifest
    for( auto B : LocalBlocks )
    {
        // Check if the master manifest contains this identifier
        bool bFound = false;
        for( auto M : Entries )
        {
            if( M.Identifier == B.Identifier )
            {
                bFound = true;
                break;
            }
        }
        
        if( !bFound )
        {
            ObsoleteBlocks.push_back( B );
        }
    }
    
    // Now, we need to build a list of blocks that need to be updated
    for( auto M : Entries )
    {
        // Check if we have this block, and if the hashes are equal
        bool bUpToDate = false;
        for( auto B : LocalBlocks )
        {
            if( M.Identifier == B.Identifier && M.Hash == B.Hash )
            {
                bUpToDate = true;
                break;
            }
        }
        
        if( !bUpToDate )
            UpdateBlocks.push_back( M );
    }
}


bool ContentManager::ReadManifest( std::vector< uint8 >& Data, Manifest& Output, uint64& outSize )
{
    // Parse the data as json
    Document Doc;
    outSize = 0;
    
    if( Doc.Parse( (char*) Data.data(), Data.size() ).HasParseError() )
    {
        cocos2d::log( "[ContentManager] Failed to parse content manifest!" );
        return false;
    }
    
    if( !Doc.HasMember( "Entries" ) || !Doc[ "Entries" ].IsArray() )
    {
        cocos2d::log( "[ContentManager] Manifest is invalid.. missing 'blocks' array" );
        return false;
    }
    
    for( auto It = Doc[ "Entries" ].Begin(); It != Doc[ "Entries" ].End(); It++ )
    {
        // Build block info
        if( !It->HasMember( "URL" ) || !It->HasMember( "Id" ) || !It->HasMember( "Hash" ) || !It->HasMember( "Size" ) )
        {
            cocos2d::log( "[ContentManager] Manifest contains invalid block info!" );
            continue;
        }
        
        ManifestEntry newEntry;
        newEntry.URL = (*It)[ "URL" ].GetString();
        newEntry.Identifier = (*It)[ "Id" ].GetString();
        newEntry.Hash = (*It)[ "Hash" ].GetString();
        newEntry.Size = (*It)[ "Size" ].GetUint64();
        outSize += newEntry.Size;
        // Ensure all identifiers are converted to lowercase
        std::transform( newEntry.Identifier.begin(), newEntry.Identifier.end(), newEntry.Identifier.begin(), ::tolower );
        
        Output.Entries.push_back( newEntry );
    }
    
    return true;
}

void ContentManager::Internal_OnProgress( const network::DownloadTask &Task, int64_t Received, int64_t TotalReceived, int64_t TotalExpected )
{
    // Were receiving progress on a single block download, but our callback needs to be provided
    // with progress on the content update as a whole, so we have to determine the total download size
    // while processing the manifest, and keep track of how many bytes we have downloaded so far
    if( ProgressCallback )
    {
        ProgressCallback( TotalProgress + TotalReceived, UpdateSize, Task.identifier );
    }
}


void ContentManager::Internal_OnComplete( bool bSuccess, uint64 TotalBytes /* = 0 */ )
{
    // Reset State
    FailureCount        = 0;
    UpdateSize          = 0;
    TotalProgress       = 0;
    bUpdateInProgress   = false;
    
    // Call bound callback
    if( CompleteCallback )
    {
        CompleteCallback( bSuccess, TotalBytes );
    }
}

void ContentManager::Internal_OnUpdateChecked( bool bUpdating, bool bError, std::string ErrMessage )
{
    if( !bUpdating )
    {
        if( bError )
            cocos2d::log( "[CMS] Failed to update.. an error occured!" );
        
        FailureCount        = 0;
        UpdateSize          = 0;
        TotalProgress       = 0;
        bUpdateInProgress   = false;
    }
    
    if( UpdateCallback )
    {
        UpdateCallback( bUpdating, bError, ErrMessage );
    }
}


/*=====================================================================================================
        Block Reader
=====================================================================================================*/
bool BlockReader::ReadHeader( ContentBlock& Output, std::vector< uint8 >& Data )
{
    // First, we need to get the first 4 bytes, which lets us know the size of the header
    if( Data.size() < 72 )
    {
        cocos2d::log( "[BlockReader] Failed to read block! Not enough data" );
        return false;
    }
    
    uint32_t HeaderSize = *( (uint32_t*) Data.data() );
    if( Data.size() < HeaderSize + 4 )
    {
        cocos2d::log( "[BlockReader] Failed to read block! Not enough data for the header" );
        return false;
    }
    auto BlobStart = Data.begin() + 4 + HeaderSize;
    std::string HeaderData( Data.begin() + 4, BlobStart );
    Document Doc;
    
    
    if( Doc.Parse( HeaderData ).HasParseError() || !Doc.HasMember( "Files" ) || !Doc.HasMember( "Id" ) )
    {
        cocos2d::log( "[BlockReader] Failed to read block! Parse error in header json" );
        return false;
    }
    
    Output.Identifier = Doc[ "Id" ].GetString();
    auto Files = Doc[ "Files" ].GetArray();
    
    for( auto It = Files.Begin(); It != Files.End(); It++ )
    {
        if( !It->HasMember( "File" ) || !It->HasMember( "Begin" ) || !It->HasMember( "End" ) )
        {
            cocos2d::log( "[BlockReader] Block header contained invalid file info!" );
            continue;
        }
        
        ContentEntry newFile;
        newFile.File = (*It)[ "File" ].GetString();
        
        uint64 StartOffset = (*It)[ "Begin" ].GetUint64();
        uint64 EndOffset = (*It)[ "End" ].GetUint64();
        
        // These offsets are referring to the start of the data blob, which is right after the header
        newFile.Begin = BlobStart + StartOffset;
        newFile.End = BlobStart + EndOffset;
        Output.Entries.push_back( newFile );
    }
    
    std::transform( Output.Identifier.begin(), Output.Identifier.end(), Output.Identifier.begin(), ::tolower );
    
    return true;
}
