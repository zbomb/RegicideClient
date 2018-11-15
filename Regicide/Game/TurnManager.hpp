//
//  TurnManager.hpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/14/18.
//

#pragma once
#include "EntityBase.hpp"

namespace Game
{
    enum class GameState
    {
        PreGame,    // Loading, in multiplayer, connecting
        CoinFlip,
        InitialDraw,
        Main,
        PostMatch
    };
    
    enum class PlayerTurn
    {
        Opponent,
        LocalPlayer
    };
    
    enum class TurnState
    {
        PreTurn, // Reset state, draw card, etc..
        BuildForces, // Play cards
        Planning, // Setup attacks
        Engage, // Opponent choosing attack response
        Battle, // Attack results
        PostBattle // Final cleanup?
    };
    
    class TurnManager : public EntityBase
    {
        
    public:
        
        TurnManager()
        : EntityBase( "TurnManager" )
        {
            _pTurn = PlayerTurn::LocalPlayer;
            _tState = TurnState::PreTurn;
            _gState = GameState::PreGame;
            
            TurnCount = 0;
        }
        
        ~TurnManager()
        {
            
        }
        
        virtual void Cleanup() override
        {
            EntityBase::Cleanup();
        }
        
        virtual void Initialize() override
        {
            EntityBase::Initialize();
            
            _pTurn = PlayerTurn::LocalPlayer;
            _tState = TurnState::PreTurn;
            _gState = GameState::PreGame;
            
            TurnCount = 0;
        }
        
        void StartGame()
        {
            if( _gState == GameState::PreGame )
            {
                // Moves the game into coin toss
                _gState = GameState::CoinFlip;
                
                if( OnGameStateChanged )
                    OnGameStateChanged( _gState );
            }
        }
        
        void SetupMatch( PlayerTurn whosFirst )
        {
            if( _gState == GameState::CoinFlip )
            {
                // Setup state
                _gState     = GameState::InitialDraw;
                _pTurn      = whosFirst;
                _tState     = TurnState::PreTurn;
                TurnCount   = 0;
                
                if( OnGameStateChanged )
                    OnGameStateChanged( _gState );
            }
        }
        
        void Advance()
        {
            if( _gState == GameState::InitialDraw )
            {
                // Both players get to play simult on initdraw
                // so once its complete, we go right into the player whos first
                // because of this, we actually dont need to advance anything
                _gState = GameState::Main;
                
                // Call both hooks, the game state moved into main
                // and the first player turn has begun
                if( OnGameStateChanged )
                    OnGameStateChanged( _gState );
                
                if( OnTurnStateChanged )
                    OnTurnStateChanged( _pTurn, _tState );
            }
            else if( _gState == GameState::Main )
            {
                // Check for turn complete
                if( _tState == TurnState::PostBattle )
                {
                    if( _pTurn == PlayerTurn::Opponent )
                        _pTurn = PlayerTurn::LocalPlayer;
                    else
                        _pTurn = PlayerTurn::Opponent;
                    
                    _tState = TurnState::PreTurn;
                }
                else if( _tState == TurnState::Battle )
                    _tState = TurnState::PostBattle;
                else if( _tState == TurnState::Engage )
                    _tState = TurnState::Battle;
                else if( _tState == TurnState::Planning )
                    _tState = TurnState::Engage;
                else if( _tState == TurnState::BuildForces )
                    _tState = TurnState::Planning;
                else
                    _tState = TurnState::BuildForces;
                
                if( OnTurnStateChanged )
                    OnTurnStateChanged( _pTurn, _tState );
            }
        }
        
        void OnPlayerWin()
        {
            _gState = GameState::PostMatch;
            
            if( OnGameStateChanged )
                OnGameStateChanged( _gState );
        }
        
        inline TurnState GetState() const       { return _tState; }
        inline PlayerTurn GetTurn() const       { return _pTurn; }
        inline GameState GetGameState() const   { return _gState; }
        inline int GetTurnCount() const         { return TurnCount; }
        
        void SetGameStateCallback( std::function< void( GameState ) > inCallback )              { OnGameStateChanged = inCallback; }
        void SetTurnStateCallback( std::function< void( PlayerTurn, TurnState ) > inCallback )  { OnTurnStateChanged = inCallback; }
        
    private:
        
        GameState _gState;
        TurnState _tState;
        PlayerTurn _pTurn;
        int TurnCount;
        
        std::function< void( GameState ) > OnGameStateChanged;
        std::function< void( PlayerTurn, TurnState ) > OnTurnStateChanged;
    };
}
