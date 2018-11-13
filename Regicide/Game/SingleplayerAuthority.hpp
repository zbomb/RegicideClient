//
//  SingleplayerAuthority.hpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/11/18.
//

#include "AuthorityBase.hpp"
#include "CardEntity.hpp"


namespace Game
{
    class Player;
    
    class SingleplayerAuthority : public AuthorityBase
    {
    public:
        
        inline Player* GetPlayer() { return LocalPlayer; }
        inline Player* GetOpponent() { return Opponent; }
        
        virtual void PostInit();
        virtual void SceneInit( cocos2d::Scene* inScene );
        
        ~SingleplayerAuthority();
        
    protected:
        
        Player* LocalPlayer;
        Player* Opponent;
        
        void Test( float Delta );
        
        friend class SingleplayerLauncher;
        
    };
}
