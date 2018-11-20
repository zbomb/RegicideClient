//
//    AccountManager.hpp
//    Regicide Mobile
//
//    Created: 11/2/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#pragma once

#include "Account.hpp"
#include "IContentSystem.hpp"

namespace Regicide
{
    enum ValidationState
    {
        NotValidated,
        InProgress,
        Valid
    };
    
    class AccountManager: public IAccountManager
    {
    public:
        
        //////////////// IAccountManager Implementation /////////////////
        inline std::shared_ptr< UserAccount >& GetLocalAccount() { return LocalAccount; }
        std::string GetAuthToken();
        bool IsLoginStored();
        void WriteAccount();
        
        ~AccountManager();
        
    private:
        
        void Init();
        std::shared_ptr< UserAccount > LocalAccount;
        
        static AccountManager* GetInstance();
        static void ForceInit();
        
        friend class IContentSystem;
        
    };
    
    
}
