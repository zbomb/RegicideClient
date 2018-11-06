//
//  ContentSystem.hpp
//  Regicide
//
//  Created by Zachary Berry on 11/5/18.
//

#pragma once
#include "Numeric.h"
#include <string>

// Dependence on RegicideAPI/Account.h
#include "Account.h"

namespace Regicide
{
    
    struct LocalBlock
    {
        std::string Identifier;
        std::string Hash;
        std::vector< std::string > Files;
    };
    
    
    class IContentManager
    {
    public:
        
        virtual void CheckForUpdates() = 0;
        virtual void ProcessUpdates() = 0;
        
        virtual bool InProgress() const = 0;
        virtual void ListenForProgress( std::function< void( uint64, uint64, std::string )> Callback ) = 0;
        virtual void ListenForComplete( std::function< void( bool, uint64 ) > Callback ) = 0;
        virtual void ListenForUpdate( std::function< void( bool, bool, std::string ) > Callback ) = 0;
        virtual void StopListening() = 0;
        
    };
    
    class IContentStorage
    {
    public:
        
        // File Managment
        virtual bool DeleteFile( const std::string& Path ) = 0;
        virtual bool FileExists( const std::string& Path ) = 0;
        
        // Reading
        virtual std::vector< uint8 > ReadFile( const std::string& Path ) = 0;
        virtual std::string ReadFileStr( const std::string& Path ) = 0;
        
        // Writing
        virtual bool WriteFile( const std::string& Path, const std::vector< uint8 >& Data ) = 0;
        virtual bool WriteFile( const std::string& Path, const std::string& Data ) = 0;
        
        // Block Manipulation
        virtual bool BlockExists( const std::string& Identifier ) = 0;
        virtual bool ReadLocalBlock( const std::string& Identifier, LocalBlock& Output ) = 0;
        virtual bool ReadLocalBlocks( std::vector< LocalBlock >& Output ) = 0;
        virtual bool WriteLocalBlock( LocalBlock& Input ) = 0;
    };
    
    class IAccountManager
    {
    public:
        
        virtual std::shared_ptr< UserAccount >& GetLocalAccount() = 0;
        virtual std::string GetAuthToken() = 0;
        virtual bool IsLoginStored() = 0;
        virtual void WriteAccount() = 0;
        
    };
    
    class IContentSystem
    {
    public:
        
        static void Init();
        static IContentStorage* GetStorage();
        static IAccountManager* GetAccounts();
        static IContentManager* GetManager();
    };
    
    
}
