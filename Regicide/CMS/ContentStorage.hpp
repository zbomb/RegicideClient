//
//  ContentStorage.hpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/3/18.
//

#pragma once

#include "IContentSystem.hpp"

namespace Regicide
{
    
    
    class ContentStorage: public IContentStorage
    {
        
    public:
        
        ///////// IContentStorage Implementation /////////////
        virtual std::vector< uint8 > ReadFile( const std::string& Path );
        virtual std::string ReadFileStr( const std::string& Path );
        virtual bool WriteFile( const std::string& Path, const std::vector< uint8 >& Data );
        virtual bool WriteFile( const std::string& Path, const std::string& Data );
        virtual bool FileExists( const std::string& Path );
        virtual bool DeleteFile( const std::string& Path );
        
        virtual bool BlockExists( const std::string& Identifier );
        virtual bool ReadLocalBlock( const std::string& Identifier, LocalBlock& Output );
        virtual bool ReadLocalBlocks( std::vector< LocalBlock >& Output );
        virtual bool WriteLocalBlock( LocalBlock& Input );
        
        ~ContentStorage();
        
    private:
        
        void Init();
        bool ParseBlock( uint8* Data, ssize_t DataSize, LocalBlock& Output );
        static ContentStorage* GetInstance();
        static void ForceInit();
        
        friend class IContentSystem;
        
    };
    
    
}

