//
//  SingleplayerAuthority.hpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/11/18.
//

#include "AuthorityBase.hpp"
#include "CardEntity.hpp"
#include "TurnManager.hpp"
#include <chrono>

namespace Game
{
    
    class Player;
    
    class SingleplayerAuthority : public AuthorityBase
    {
    public:
        
        inline Player* GetPlayer() { return LocalPlayer; }
        inline Player* GetOpponent() { return Opponent; }
        
        virtual void PostInit() override;
        virtual void SceneInit( cocos2d::Scene* inScene ) override;
        
        // Player Actions
        bool CanPlayCard( Player* inPlayer, CardEntity* inCard );
        bool PlayCard( Player* inPlayer, CardEntity* inCard, bool bMoveCard = false );
        
        void FinishTurn( Player* inPlayer );
        
        // Passthrough getters for game state
        PlayerTurn GetPlayersTurn() const   { return turnManager ? turnManager->GetTurn() : PlayerTurn::LocalPlayer; }
        TurnState GetTurnState() const      { return turnManager ? turnManager->GetState() : TurnState::PreTurn;; }
        GameState GetGameState() const      { return turnManager ? turnManager->GetGameState() : GameState::PreGame; }
        
        virtual void update( float Delta );
        
        ~SingleplayerAuthority();
        
    protected:
        
        Player* LocalPlayer;
        Player* Opponent;
        
        void Test( float Delta );
        virtual void Cleanup() override;
        
        // Internal Actions
        void DrawCard( Player* Target );
        
        void StartGame();
        void OnGameStateChanged( GameState inState );
        void OnTurnStateChanged( PlayerTurn inPlayer, TurnState inState );
        
        TurnState _turn;
        PlayerTurn _plturn;
        GameState _gstate;
        
        std::chrono::steady_clock::time_point StateBegin;
        void OnPreGameComplete();
        
        void CheckInitDraw();
        
        friend class SingleplayerLauncher;
        
    };
}
