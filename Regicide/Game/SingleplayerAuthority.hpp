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

#define LUA_ACTION_ROOT 0               // Add action to root queue
#define LUA_ACTION_SERIAL 1             // Add action to run after the last serial/root action
#define LUA_ACTION_PARALLEL 2           // Add action to run in parllel with the last serial/root action
#define LUA_ACTION_SERIAL_PARALLEL 3     // Add action to run after the last parallel action

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
        
        void Tick( float Delta );
        void OnGameWon( Player* Winner );
        
    private:
        
        std::chrono::steady_clock::time_point _tWaitStart;
        std::function< void( float, bool ) > _fWaitCallback;
        
        // We need to keep track of the position in decks, so we can draw cards from lua
        static uint32_t OwnerDeckIndex;
        static uint32_t OpponentDeckIndex;

        
    public:
        
        static uint32_t Lua_PopDeck( Player* In );
        
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
        
        static ActionQueue* Lua_RootQueue;
        static Action* Lua_RootAction;
        static Action* Lua_LastSerial;
        static Action* Lua_LastParallel;
        static void SetLuaActionRoot( ActionQueue* RootQueue, Action* RootAction = nullptr );
        static void ClearLuaActionRoot();
        static void SetLuaDeckIndex( Player* Target, int Index );
        
    public:
        
        template< typename T >
        static T* CreateLuaAction( Game::EntityBase* inTarget, int ActionType );
        
    public:
        
        inline MatchState GetMatchState() const { return mState; }
        inline PlayerTurn GetPlayerTurn() const { return pState; }
        inline TurnState GetTurnState() const { return tState; }
        
        void StartGame( float Delay, bool bTimeout );
        void CoinFlipFinish();
        
        void CallHook( const std::string& In );
        
        template< typename T1 >
        void CallHook( const std::string& In, T1 Arg1 );
        
        template< typename A1, typename A2 >
        void CallHook( const std::string& In, A1 Arg1, A2 Arg2 );
        
        template< typename B1, typename B2, typename B3 >
        void CallHook( const std::string& In, B1 Arg1, B2 Arg2, B3 Arg3 );
        
        template< typename C1, typename C2, typename C3, typename C4 >
        void CallHook( const std::string& In, C1 Arg1, C2 Arg2, C3 Arg3, C4 Arg4 );
        
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
    
    template< typename T >
    T* SingleplayerAuthority::CreateLuaAction( Game::EntityBase* inTarget, int ActionType )
    {
        T* Output = nullptr;
        
        if( ActionType == LUA_ACTION_ROOT )
        {
            if( Lua_RootAction )
            {
                Output = Lua_RootAction->CreateAction< T >( inTarget );
            }
            else
            {
                Output = Lua_RootQueue->CreateAction< T >( inTarget );
            }
        }
        else if( ActionType == LUA_ACTION_SERIAL )
        {
            if( Lua_LastSerial )
            {
                Output = Lua_LastSerial->CreateAction< T >( inTarget );
            }
            else
            {
                if( Lua_RootAction )
                    Output = Lua_RootAction->CreateAction< T >( inTarget );
                else
                    Output = Lua_RootQueue->CreateAction< T >( inTarget );
            }
            
            Lua_LastSerial = Output;
        }
        else if( ActionType == LUA_ACTION_PARALLEL )
        {
            if( Lua_LastSerial )
            {
                // We need to add to the same vector as the last action
                auto* parent = Lua_LastSerial->Owner;
                if( parent )
                {
                    Output = parent->CreateAction< T >( inTarget );
                }
                else
                {
                    if( Lua_RootAction )
                        Output = Lua_RootAction->CreateAction< T >( inTarget );
                    else
                        Output = Lua_RootQueue->CreateAction< T >( inTarget );
                }
            }
            else
            {
                if( Lua_RootAction )
                    Output = Lua_RootAction->CreateAction< T >( inTarget );
                else
                    Output = Lua_RootQueue->CreateAction< T >( inTarget );
            }
            
            Lua_LastParallel = Output;
        }
        else if( ActionType == LUA_ACTION_SERIAL_PARALLEL )
        {
            if( Lua_LastParallel )
            {
                Output = Lua_LastParallel->CreateAction< T >( inTarget );
            }
            else
            {
                if( Lua_RootAction )
                    Output = Lua_RootAction->CreateAction< T >( inTarget );
                else
                    Output = Lua_RootQueue->CreateAction< T >( inTarget );
            }
            
            Lua_LastSerial = Output;
        }
        else
        {
            cocos2d::log( "[Lua] Invalid Action Order Type!" );
        }
        
        return Output;
    }
    
    
    // Card Hook Callers
    template< typename T1 >
    void SingleplayerAuthority::CallHook( const std::string& In, T1 Arg1 )
    {
        auto& Ent = IEntityManager::GetInstance();
        auto lua = Regicide::LuaEngine::GetInstance();
        
        if( !lua )
            return;
        
        // Check if theres an action queue set for lua to use
        // If not, then we will just make our own for local use
        auto Queue = ActionQueue();
        bool bUseLocal = false;
        
        if( !Lua_RootQueue )
        {
            SetLuaActionRoot( &Queue );
            bUseLocal = true;
        }
        
        auto Player = GetPlayer();
        auto Opponent = GetOpponent();
        
        if( Player )
        {
            auto King = Player->GetKing();
            luabridge::LuaRef Func( lua->State() );
            if( King && King->GetHook( In, Func ) )
            {
                try
                {
                    Func( King, Arg1 );
                }
                catch( std::exception& e )
                {
                    cocos2d::log( "[Lua] Error! %s", e.what() );
                }
            }
        }
        
        if( Opponent )
        {
            auto King = Opponent->GetKing();
            luabridge::LuaRef Func( lua->State() );
            if( King && King->GetHook( In, Func ) )
            {
                try
                {
                    Func( King, Arg1 );
                }
                catch( std::exception& e )
                {
                    cocos2d::log( "[Lua] Error! %s", e.what() );
                }
            }
        }
        
        for( auto It = Ent.Begin(); It != Ent.End(); It++ )
        {
            if( It->second && It->second->IsCard() )
            {
                auto Card = dynamic_cast< CardEntity* >( It->second.get() );
                if( Card && Card->ShouldCallHook( In ) )
                {
                    luabridge::LuaRef Func( lua->State() );
                    if( Card->GetHook( In, Func ) )
                    {
                        try
                        {
                            Func( Card, Arg1 );
                        }
                        catch( std::exception& e )
                        {
                            cocos2d::log( "[Lua] Error! %s", e.what() );
                        }
                    }
                }
            }
        }
        
        // If we created our own queue, then run it
        if( bUseLocal && Queue.ActionTree.size() > 0 )
        {
            auto GM = Game::World::GetWorld()->GetGameMode< GameModeBase >();
            CC_ASSERT( GM );
            
            ClearLuaActionRoot();
            GM->RunActionQueue( std::move( Queue ) );
        }
    }
    
    template< typename A1, typename A2 >
    void SingleplayerAuthority::CallHook( const std::string& In, A1 Arg1, A2 Arg2 )
    {
        auto& Ent = IEntityManager::GetInstance();
        auto lua = Regicide::LuaEngine::GetInstance();
        
        if( !lua )
            return;
        
        // Check if theres an action queue set for lua to use
        // If not, then we will just make our own for local use
        auto Queue = ActionQueue();
        bool bUseLocal = false;
        
        if( !Lua_RootQueue )
        {
            SetLuaActionRoot( &Queue );
            bUseLocal = true;
        }
        
        auto Player = GetPlayer();
        auto Opponent = GetOpponent();
        
        if( Player )
        {
            auto King = Player->GetKing();
            luabridge::LuaRef Func( lua->State() );
            if( King && King->GetHook( In, Func ) )
            {
                try
                {
                    Func( King, Arg1, Arg2 );
                }
                catch( std::exception& e )
                {
                    cocos2d::log( "[Lua] Error! %s", e.what() );
                }
            }
        }
        
        if( Opponent )
        {
            auto King = Opponent->GetKing();
            luabridge::LuaRef Func( lua->State() );
            if( King && King->GetHook( In, Func ) )
            {
                try
                {
                    Func( King, Arg1, Arg2 );
                }
                catch( std::exception& e )
                {
                    cocos2d::log( "[Lua] Error! %s", e.what() );
                }
            }
        }
        
        for( auto It = Ent.Begin(); It != Ent.End(); It++ )
        {
            if( It->second && It->second->IsCard() )
            {
                auto Card = dynamic_cast< CardEntity* >( It->second.get() );
                if( Card && Card->ShouldCallHook( In ) )
                {
                    luabridge::LuaRef Func( lua->State() );
                    if( Card->GetHook( In, Func ) )
                    {
                        try
                        {
                            Func( Card, Arg1, Arg2 );
                        }
                        catch( std::exception& e )
                        {
                            cocos2d::log( "[Lua] Error! %s", e.what() );
                        }
                    }
                }
            }
        }
        
        // If we created our own queue, then run it
        if( bUseLocal )
        {
            auto GM = Game::World::GetWorld()->GetGameMode< GameModeBase >();
            CC_ASSERT( GM );
            
            ClearLuaActionRoot();
            GM->RunActionQueue( std::move( Queue ) );
        }
    }
    
    template< typename B1, typename B2, typename B3 >
    void SingleplayerAuthority::CallHook( const std::string& In, B1 Arg1, B2 Arg2, B3 Arg3 )
    {
        auto& Ent = IEntityManager::GetInstance();
        auto lua = Regicide::LuaEngine::GetInstance();
        
        if( !lua )
            return;
        
        // Check if theres an action queue set for lua to use
        // If not, then we will just make our own for local use
        auto Queue = ActionQueue();
        bool bUseLocal = false;
        
        if( !Lua_RootQueue )
        {
            SetLuaActionRoot( &Queue );
            bUseLocal = true;
        }
        
        auto Player = GetPlayer();
        auto Opponent = GetOpponent();
        
        if( Player )
        {
            auto King = Player->GetKing();
            luabridge::LuaRef Func( lua->State() );
            if( King && King->GetHook( In, Func ) )
            {
                try
                {
                    Func( King, Arg1, Arg2, Arg3 );
                }
                catch( std::exception& e )
                {
                    cocos2d::log( "[Lua] Error! %s", e.what() );
                }
            }
        }
        
        if( Opponent )
        {
            auto King = Opponent->GetKing();
            luabridge::LuaRef Func( lua->State() );
            if( King && King->GetHook( In, Func ) )
            {
                try
                {
                    Func( King, Arg1, Arg2, Arg3 );
                }
                catch( std::exception& e )
                {
                    cocos2d::log( "[Lua] Error! %s", e.what() );
                }
            }
        }
        
        for( auto It = Ent.Begin(); It != Ent.End(); It++ )
        {
            if( It->second && It->second->IsCard() )
            {
                auto Card = dynamic_cast< CardEntity* >( It->second.get() );
                if( Card && Card->ShouldCallHook( In ) )
                {
                    luabridge::LuaRef Func( lua->State() );
                    if( Card->GetHook( In, Func ) )
                    {
                        try
                        {
                            Func( Card, Arg1, Arg2, Arg3 );
                        }
                        catch( std::exception& e )
                        {
                            cocos2d::log( "[Lua] Error! %s", e.what() );
                        }
                    }
                }
            }
        }
        
        // If we created our own queue, then run it
        if( bUseLocal )
        {
            auto GM = Game::World::GetWorld()->GetGameMode< GameModeBase >();
            CC_ASSERT( GM );
            
            ClearLuaActionRoot();
            GM->RunActionQueue( std::move( Queue ) );
        }
    }
    
    template< typename C1, typename C2, typename C3, typename C4 >
    void SingleplayerAuthority::CallHook( const std::string& In, C1 Arg1, C2 Arg2, C3 Arg3, C4 Arg4 )
    {
        auto& Ent = IEntityManager::GetInstance();
        auto lua = Regicide::LuaEngine::GetInstance();
        
        if( !lua )
            return;
        
        // Check if theres an action queue set for lua to use
        // If not, then we will just make our own for local use
        auto Queue = ActionQueue();
        bool bUseLocal = false;
        
        if( !Lua_RootQueue )
        {
            SetLuaActionRoot( &Queue );
            bUseLocal = true;
        }
        
        auto Player = GetPlayer();
        auto Opponent = GetOpponent();
        
        if( Player )
        {
            auto King = Player->GetKing();
            luabridge::LuaRef Func( lua->State() );
            if( King && King->GetHook( In, Func ) )
            {
                try
                {
                    Func( King, Arg1, Arg2, Arg3, Arg4 );
                }
                catch( std::exception& e )
                {
                    cocos2d::log( "[Lua] Error! %s", e.what() );
                }
            }
        }
        
        if( Opponent )
        {
            auto King = Opponent->GetKing();
            luabridge::LuaRef Func( lua->State() );
            if( King && King->GetHook( In, Func ) )
            {
                try
                {
                    Func( King, Arg1, Arg2, Arg3, Arg4 );
                }
                catch( std::exception& e )
                {
                    cocos2d::log( "[Lua] Error! %s", e.what() );
                }
            }
        }
        
        for( auto It = Ent.Begin(); It != Ent.End(); It++ )
        {
            if( It->second && It->second->IsCard() )
            {
                auto Card = dynamic_cast< CardEntity* >( It->second.get() );
                if( Card && Card->ShouldCallHook( In ) )
                {
                    luabridge::LuaRef Func( lua->State() );
                    if( Card->GetHook( In, Func ) )
                    {
                        try
                        {
                            Func( Card, Arg1, Arg2, Arg3, Arg4 );
                        }
                        catch( std::exception& e )
                        {
                            cocos2d::log( "[Lua] Error! %s", e.what() );
                        }
                    }
                }
            }
            
            // If we created our own queue, then run it
            if( bUseLocal )
            {
                auto GM = Game::World::GetWorld()->GetGameMode< GameModeBase >();
                CC_ASSERT( GM );
                
                ClearLuaActionRoot();
                GM->RunActionQueue( std::move( Queue ) );
            }
        }
    }
}
