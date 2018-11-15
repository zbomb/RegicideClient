//
//  AuthorityBase.hpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/11/18.
//

#pragma once

#include "EntityBase.hpp"

namespace Game
{
    class TurnManager;
    
    class AuthorityBase : public EntityBase
    {
        
    public:
        
        AuthorityBase();
        ~AuthorityBase();
        
        virtual void Cleanup();
        
    protected:
        
        TurnManager* turnManager;
        
    };
}
