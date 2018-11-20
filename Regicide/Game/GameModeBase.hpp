//
//    GameModeBase.hpp
//    Regicide Mobile
//
//    Created: 11/10/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#pragma once
#include "EntityBase.hpp"
#include "Player.hpp"
#include "World.hpp"
#include "GameModeBase.hpp"
#include "AuthorityBase.hpp"
#include "Actions.hpp"


namespace Game
{
    
    class GameModeBase : public EntityBase
    {
    public:
        
        GameModeBase();
        ~GameModeBase();
        
        virtual void Cleanup() override;
        virtual void PostInit() override;
        
        virtual void TouchBegan( cocos2d::Touch* inTouch, CardEntity* inCard ) = 0;
        virtual void TouchEnd( cocos2d::Touch* inTouch, CardEntity* inCard ) = 0;
        virtual void TouchMoved( cocos2d::Touch* inTouch ) = 0;
        virtual void TouchCancel( cocos2d::Touch* inTouch ) = 0;
        
        virtual bool CouldPlayCard( CardEntity* In );
        virtual bool CanPlayCard( CardEntity* In );
        
        virtual bool CanCardAttack( CardEntity* In );
        virtual bool CanCardBlock( CardEntity* In, CardEntity* Target = nullptr );
        virtual bool CanTriggerAbility( CardEntity* In );

    protected:
        
        template< typename T >
        T* GetAuthority() const;
        
        inline Player* GetPlayer()  { return GetWorld()->GetLocalPlayer(); }
        inline Player* GetOpponent()   { return GetWorld()->GetOpponent(); }
    
    protected:
        
        virtual void Initialize() override = 0;
        
    public:
        
        void RunActionQueue( Game::ActionQueue&& In );
        bool PlayCard( CardEntity* In, int Index = -1 );

        inline void InvalidatePossibleActions() { _bCheckPossibleActions = true; }
        
    protected:
        
        MatchState mState;
        TurnState tState;
        PlayerTurn pState;
        
        void UpdateMatchState( MatchState In );
        void UpdateTurnState( TurnState In );
        void UpdatePlayerTurn( PlayerTurn In );
        
        std::map< uint32_t, ActionQueue > ActiveQueues;
        virtual void ExecuteActions( const std::vector< std::unique_ptr< Game::Action > >& In, uint32_t QueueId, bool bRootAction = true );
        
        void Tick( float Delta );
        bool _bCheckPossibleActions = false;
        
    private:
        
        void _OnEndOfBranch( uint32_t QueueId );
        
        // Actions
        
    protected:
        
        virtual void Action_CoinFlip( Action* In, std::function< void() > Callback );
        virtual void Action_BlitzStart( Action* In, std::function< void() > Callback );
        virtual void Action_BlitzQuery( Action* In, std::function< void() > Callback );
        virtual void Action_MainStart( Action* In, std::function< void() > Callback );
        virtual void Action_BlitzError( Action* In, std::function< void() > Callback );
        virtual void Action_TurnStart( Action* In, std::function< void() > Callback );
        virtual void Action_MarshalStart( Action* In, std::function< void() > Callback );
        virtual void Action_AttackStart( Action* In, std::function< void() > Callback );
        virtual void Action_BlockStart( Action* In, std::function< void() > Callback );
        virtual void Action_PostTurnStart( Action* In, std::function< void() > Callback );
        
    public:
        
        void FinishTurn();
        
    };
    
    template< typename T >
    T* GameModeBase::GetAuthority() const
    {
        static_assert( std::is_base_of< AuthorityBase, T >::value, "Cast type must be a subclass of AuthorityBase" );
        auto world = GetWorld();
        auto auth = world->GetAuthority< T >();
        
        // Should we assert?
        if( !auth )
            cocos2d::log( "[FATAL] Failed to get reference to authority from within GameMode!" );

        return auth;
    }
}
