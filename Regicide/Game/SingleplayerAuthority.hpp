//
//    SingleplayerAuthority.hpp
//    Regicide Mobile
//
//    Created: 11/11/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#include "AuthorityBase.hpp"
#include "CardEntity.hpp"
#include <chrono>
#include "Actions.hpp"

namespace Game
{
    
    class Player;
    
    class SingleplayerAuthority : public AuthorityBase
    {
    public:
        
        virtual void PostInit() override;
        virtual void SceneInit( cocos2d::Scene* inScene ) override;
        
        ~SingleplayerAuthority();
        
    protected:
        
        void Test( float Delta );
        virtual void Cleanup() override;
        
        ////////////////// Game Stuff ////////////////////
        
    public:
        
        virtual void SetReady() override;
        virtual void SetBlitzCards( const std::vector< CardEntity* >& Cards ) override;
        virtual void PlayCard( CardEntity* In, int Index ) override;
        virtual void FinishTurn() override;
        virtual void SetAttackers( const std::vector< CardEntity* >& Cards ) override;
        virtual void SetBlockers( const std::map< CardEntity*, CardEntity* >& Cards ) override;
        virtual void TriggerAbility( CardEntity* Card, uint8_t AbilityId ) override;
        
        void AI_PlayCard( CardEntity* In, std::function< void() > Callback );
        void AI_SetAttackers( const std::vector< CardEntity* >& In );
        void AI_SetBlockers( const std::map< CardEntity*, CardEntity* >& Cards );
        
    protected:
        
        void WaitOnPlayer( std::function< void( float, bool ) > OnReady, float Timeout );
        
        std::vector< CardEntity* > PlayerBlitzSelection;
        std::vector< CardEntity* > OpponentBlitzSelection;
        
        template< typename T >
        T* GetGameMode();
        
        virtual bool DrawCard( Player* In, uint32_t Count = 1 ) override;
        void Tick( float Delta );
        
        void OnGameWon( Player* Winner );
        
    private:
        
        std::chrono::steady_clock::time_point _tWaitStart;
        std::function< void( float, bool ) > _fWaitCallback;
        
        /////////// State System ////////////
        
    protected:
        
        MatchState mState;
        PlayerTurn pState;
        TurnState tState;
        
        void FinishBlitz();
        void StartMatch();
        
        Player* CurrentTurnPlayer();
        void PreTurn( PlayerTurn pTurn );
        void Marshal();
        void Attack();
        void Block();
        void Damage();
        void PostTurn();
        
        std::map< CardEntity*, std::vector< CardEntity* > > BattleMatrix;
        
    public:
        
        inline MatchState GetMatchState() const { return mState; }
        inline PlayerTurn GetPlayerTurn() const { return pState; }
        inline TurnState GetTurnState() const { return tState; }
        
        void StartGame( float Delay, bool bTimeout );
        void CoinFlipFinish();
        
        friend class SingleplayerLauncher;
        
    };
    
    template< typename T >
    T* SingleplayerAuthority::GetGameMode()
    {
        return Game::World::GetWorld()->GetGameMode< T >();
    }
}
