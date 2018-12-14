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
#include "GameModeBase.hpp"
#include "CardEntity.hpp"
#include "KingEntity.hpp"
#include <chrono>
#include "Actions.hpp"


namespace Game
{
    
    // Forward Declarations
    class Player;
    class AIController;
    
    // Class Definition
    class SingleplayerAuthority : public AuthorityBase
    {
    public:
        
        virtual void PostInit() override;
        virtual void SceneInit( cocos2d::Scene* inScene ) override;
        
        SingleplayerAuthority();
        ~SingleplayerAuthority();
        
        virtual void SetReady() override;
        virtual void SetBlitzCards( const std::vector< uint32_t >& Cards ) override;
        virtual void PlayCard( uint32_t In, int Index ) override;
        virtual void FinishTurn() override;
        virtual void SetAttackers( const std::vector< uint32_t >& Cards ) override;
        virtual void SetBlockers( const std::map< uint32_t, uint32_t >& Cards ) override;
        virtual void TriggerAbility( uint32_t Card, uint8_t AbilityId ) override;
        
        void AI_PlayCard( uint32_t In, std::function< void() > Callback );
        void AI_SetAttackers( const std::vector< uint32_t >& In );
        void AI_SetBlockers( const std::map< uint32_t, uint32_t >& Cards );
        
        std::map< uint32_t, std::vector< uint32_t > > BattleMatrix;
        
        inline AIController* GetAI() { return AI; }
        
        void StartGame( float Delay, bool bTimeout );
        void CoinFlipFinish();
        
    protected:
        
        virtual void Cleanup() override;
        
        void WaitOnPlayer( std::function< void( float, bool ) > OnReady, float Timeout );
        
        std::vector< uint32_t > PlayerBlitzSelection;
        std::vector< uint32_t > OpponentBlitzSelection;
        
        template< typename T >
        T* GetGameMode();
        
        void Tick( float Delta );
        void OnGameWon( uint32_t Winner );
        
        void FinishBlitz();
        void StartMatch();
        
        PlayerState* GetActivePlayer();
        PlayerState* GetInactivePlayer();
        
        void PreTurn( PlayerTurn pTurn );
        void Marshal();
        void Attack();
        void Block();
        void Damage();
        void PostTurn();
        
        AIController* AI;
        
        // State Loading
        bool LoadPlayers( const std::string& LocalPlayerName, uint16_t PlayerHealth, uint16_t PlayerMana, const Regicide::Deck& PlayerDeck, const std::string& OpponentName, uint16_t OpponentHealth, uint16_t OpponentMana, const Regicide::Deck& OpponentDeck );
        
    private:
        
        bool DoLoad( PlayerState* Target, const std::string& Name, uint16_t Mana, uint16_t Stamina, const Regicide::Deck& Deck );
        
        std::chrono::steady_clock::time_point _tWaitStart;
        std::function< void( float, bool ) > _fWaitCallback;
        
        bool _bBlitzComplete;
        
        friend class SingleplayerLauncher;
        
    };
    
    
    /*=====================================================================
        Template Functions
     ====================================================================*/
    template< typename T >
    T* SingleplayerAuthority::GetGameMode()
    {
        return Game::World::GetWorld()->GetGameMode< T >();
    }
    
}
