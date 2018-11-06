//
//  Account.h
//  Regicide
//
//  Created by Zachary Berry on 10/31/18.
//
#pragma once

#include <vector>
#include "Numeric.h"

namespace Regicide
{
    struct AccountInfo
    {
    public:
        std::string Username;
        std::string DisplayName;
        std::string Email;
        uint64 Coins;
        bool bVerified;
        
        AccountInfo()
        {
            Username = "";
            DisplayName = "";
            Email = "";
            Coins = 0;
            bVerified = false;
        }
    };
    
    struct Card
    {
        uint16 Id;
        uint16 Ct;
        
        Card()
        {
            Id = 0;
            Ct = 0;
        }
    };
    
    struct Deck
    {
        std::string Name;
        uint32 Id;
        
        std::vector< Card > Cards;
        
        Deck()
        {
            Cards = std::vector< Card >();
            Name = "";
            Id = 0;
        }
    };
    
    struct Achievement
    {
        uint32 Id;
        bool Complete;
        int32 State;
        
        Achievement()
        {
            Id = 0;
            Complete = false;
            State = 0;
        }
    };
    
    class UserAccount
    {
    public:
        AccountInfo Info;
        std::vector< Card > Cards;
        std::vector< Deck > Decks;
        std::vector< Achievement > Achievements;
        std::string AuthToken;
        
        UserAccount()
        : Info(), Cards(), Decks(), Achievements()
        {
        }
        
    };
    
}
