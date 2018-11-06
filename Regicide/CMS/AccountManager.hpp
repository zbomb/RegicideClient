//
//  AccountManager.hpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/2/18.
//

#ifndef AccountManager_hpp
#define AccountManager_hpp

#include "Account.h"
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
        
        
    private:
        
        void Init();
        std::shared_ptr< UserAccount > LocalAccount;
        
        static AccountManager* GetInstance();
        static void ForceInit();
        
        friend class IContentSystem;
        
    };
    
    
}

#endif /* AccountManager_hpp */
