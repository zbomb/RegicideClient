//
//  ContentManager.hpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/2/18.
//

#ifndef ContentManager_hpp
#define ContentManager_hpp

#include "network/CCDownloader.h"
#include "Numeric.h"
#include "IContentSystem.hpp"

using namespace cocos2d;

namespace Regicide
{
    /*====================================================
        Manifest Structure
     ====================================================*/
    struct ManifestEntry
    {
        std::string URL;
        std::string Identifier;
        std::string Hash;
        uint64 Size;
    };
    
    struct Manifest
    {
        std::string Hash;
        std::vector< ManifestEntry > Entries;
    };
    
    /*====================================================
        Block Structure
     ====================================================*/
    struct ContentEntry
    {
        std::string File;
        std::vector< uint8 >::iterator Begin;
        std::vector< uint8 >::iterator End;
    };
    
    struct ContentBlock
    {
        std::string Identifier;
        //std::string Hash;
        std::vector< ContentEntry > Entries;
    };
    
    
    // Reads downloaded blocks from the server
    class BlockReader
    {
    public:
        
        bool ReadHeader( ContentBlock& Output, std::vector< uint8 >& );
        
    };
    
    
    class ContentManager: public IContentManager
    {
    public:
        
        ////////////// IContentManager Implementation ////////////////////
        void CheckForUpdates();
        void ProcessUpdates();
        
        inline bool InProgress() const { return bUpdateInProgress; }
        inline void ListenForProgress( std::function< void( uint64, uint64, std::string )> Callback ) { ProgressCallback = Callback; }
        inline void ListenForComplete( std::function< void( bool, uint64 ) > Callback ) { CompleteCallback = Callback; }
        inline void ListenForUpdate( std::function< void( bool, bool, std::string ) > Callback ) { UpdateCallback = Callback; }
        inline void StopListening() { ProgressCallback = nullptr; CompleteCallback = nullptr; UpdateCallback = nullptr; }
        
    private:
        
        void Init();
        
        std::shared_ptr< network::Downloader > DownloadManager;
        std::shared_ptr< const network::DownloadTask > CurrentDownload;
        std::vector< ManifestEntry > NeededContent;
        std::vector< LocalBlock > OldContent;
        
        bool ReadManifest( std::vector< uint8 >& Data, Manifest& Output, uint64& outSize );
        Manifest MasterManifest;
        
        void ProcessManifest( std::vector< ManifestEntry >& Entry, std::vector< LocalBlock >& LocalBlocks, std::vector< ManifestEntry >& NeededBlocks, std::vector< LocalBlock >& ObsoleteBlocks );
        
        void DeleteContent( std::vector< LocalBlock >& Blocks );
        void OnBlockDownload( const network::DownloadTask& Task, std::vector< unsigned char >& Data );
        void OnBlockDownloadFail( const network::DownloadTask& task, int errorCode, int errorCodeInternal, const std::string& errorStr );
        
        unsigned int FailureCount = 0;
        network::DownloaderHints Hints;
        
        bool bUpdateInProgress = false;
        
        std::function< void( uint64, uint64, std::string ) > ProgressCallback;
        std::function< void( bool, uint64 ) > CompleteCallback;
        std::function< void( bool, bool, std::string ) > UpdateCallback;
        
        uint64 TotalProgress;
        uint64 UpdateSize;
        
        void Internal_OnProgress( const network::DownloadTask& Task, int64_t Received, int64_t TotalReceived, int64_t TotalExpected );
        void Internal_OnComplete( bool bSuccess, uint64 BytesTotal = 0 );
        void Internal_OnUpdateChecked( bool bStartingUpdate, bool bError, std::string ErrorMessage );
        
        static ContentManager* GetInstance();
        static void ForceInit();
        
        friend class IContentSystem;
        
    };
    
    
}

#endif /* ContentManager_hpp */
