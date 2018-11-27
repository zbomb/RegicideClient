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
#include "UI/CardViewer.hpp"


namespace Game
{
    
    class GameModeBase : public EntityBase
    {
    public:
        
        GameModeBase();
        ~GameModeBase();
        
        virtual void Initialize() override;
        virtual void Cleanup() override;
        virtual void PostInit() override;
        
        /*====================================================
            Input Logic
        ====================================================*/
    public:
        
        virtual void TouchBegan( cocos2d::Touch* inTouch, CardEntity* inCard );
        virtual void TouchEnd( cocos2d::Touch* inTouch, CardEntity* inCard, cocos2d::DrawNode* inDraw );
        virtual void TouchMoved( cocos2d::Touch* inTouch, cocos2d::DrawNode* inDraw );
        virtual void TouchCancel( cocos2d::Touch* inTouch, cocos2d::DrawNode* inDraw );
        
        virtual void OnCardClicked( CardEntity* inCard );
        virtual void OpenGraveyardViewer( GraveyardEntity* Grave );
        virtual void OpenCardViewer( CardEntity* inCard );
        virtual void OpenHandViewer( CardEntity* inCard );
        virtual void CloseGraveyardViewer();
        virtual void CloseCardViewer();
        virtual void CloseHandViewer();
        virtual bool OnCardDragDrop( CardEntity* inCard, cocos2d::Touch* Info );
        virtual void OnCardSwipedUp( CardEntity* inCard );
        virtual bool OnBlockerSelect( CardEntity* inBlocker, CardEntity* inAttacker );
        
        void RedrawBlockers();
        inline std::map< CardEntity*, CardEntity* > GetBlockMatrix() { return BlockMatrix; }
        inline void SetBlocker( CardEntity* InBlocker, CardEntity* InAttacker ) { if( InBlocker && InAttacker ) BlockMatrix[ InBlocker ] = InAttacker; }
        
    protected:
        
        std::map< CardEntity*, CardEntity* > BlockMatrix;
        
        CardEntity* _touchedCard;
        CardEntity* _viewCard;
        CardViewer* _Viewer;
        
        bool _bDrag = false;
        bool _bBlockerDrag = false;
        cocos2d::Vec2 _DragOffset = cocos2d::Vec2::ZERO;
        cocos2d::Vec2 _TouchStart = cocos2d::Vec2::ZERO;
        
        
        void _DoCloseViewer();
        
        virtual void DisableSelection();
        virtual void EnableSelection();
        
        bool _bSelectionEnabled;
        
        /*====================================================
            End of Input Logic
         ====================================================*/
        
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
        uint32_t lastDamageId;
        uint32_t lastDrainId;
        
    private:
        
        void _OnEndOfBranch( uint32_t QueueId );
        
        // Actions
        
    protected:
        
        virtual void Action_CoinFlip( Action* In, std::function< void() > Callback );
        virtual void Action_BlitzStart( Action* In, std::function< void() > Callback );
        virtual void Action_BlitzQuery( Action* In, std::function< void() > Callback );
        virtual void Action_MainStart( Action* In, std::function< void() > Callback );
        virtual void Action_BlitzError( Action* In, std::function< void() > Callback );
        virtual void Action_AttackError( Action* In, std::function< void() > Callback );
        virtual void Action_TurnStart( Action* In, std::function< void() > Callback );
        virtual void Action_MarshalStart( Action* In, std::function< void() > Callback );
        virtual void Action_AttackStart( Action* In, std::function< void() > Callback );
        virtual void Action_BlockStart( Action* In, std::function< void() > Callback );
        virtual void Action_PostTurnStart( Action* In, std::function< void() > Callback );
        virtual void Action_CardDamage( Action* In, std::function< void() > Callback );
        virtual void Action_DamageStart( Action* In, std::function< void() > Callback );
        virtual void Action_StaminaDrain( Action* In, std::function< void() > Callback );
        
        virtual void OnActionQueue();
        
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
