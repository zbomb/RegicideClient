//
//    Account.hpp
//    Regicide Mobile
//
//    Created: 10/31/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#pragma once

#include <vector>
#include "Numeric.hpp"

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
        
        Card( uint16 inId, uint16 inCt )
        : Id( inId ), Ct( inCt )
        {}
    };
    
    struct Deck
    {
        std::string Name;
        uint32 Id;
        uint32 KingId;
        
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
