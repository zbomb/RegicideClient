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
#include "UI/CardSelector.hpp"
#include "ClientState.hpp"

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
        
        CardSelector* _graveViewer;
        
        /*====================================================
            End of Input Logic
         ====================================================*/
        
    public:
        
        virtual bool CouldPlayCard( CardEntity* In );
        virtual bool CanPlayCard( CardEntity* In );
        
        virtual bool CanCardAttack( CardEntity* In );
        virtual bool CanCardBlock( CardEntity* In, CardEntity* Target = nullptr );
        virtual bool CanTriggerAbility( CardEntity* In );
        virtual bool CanTriggerAbility( CardEntity* In, uint8_t AbilityId );
        
        void RunActionQueue( Game::ActionQueue&& In );
        bool PlayCard( CardEntity* In, int Index = -1 );
        
        inline void InvalidatePossibleActions() { _bCheckPossibleActions = true; }
        
        void FinishTurn();
        
        inline ClientState& GetState() { return State; }
        
        virtual bool TriggerAbility( CardEntity* Target, uint8_t AbilityId );
        
    protected:
        
        template< typename T >
        T* GetAuthority() const;
        
        inline Player* GetPlayer()  { return State.GetPlayer(); }
        inline Player* GetOpponent()   { return State.GetOpponent(); }
        
        void RunAction( Action& Target, std::function< void() > Callback );
        void PopQueue( ActionQueue& Target );
        void AddAction( const std::string& Name, std::function< void( Action*, std::function< void() > ) > Handler );
        
        std::map< uint32_t, ActionQueue > ActiveQueues;
        std::map< std::string, std::function< void( Action*, std::function< void() > ) > > ActionHandlers;
        
        void UpdateMatchState( MatchState In );
        void UpdateTurnState( TurnState In );
        void UpdatePlayerTurn( PlayerTurn In );
        
        void Tick( float Delta );
        bool _bCheckPossibleActions = false;
        bool _bFinishCalled = false;
        uint32_t lastDamageId;
        uint32_t lastDrainId;
        
        ClientState State;

        // Updated Action Callbacks
        virtual void OnCardDraw( Action* In, std::function< void() > Callback );
        virtual void OnCardPlay( Action* In, std::function< void() > Callback );
        virtual void OnManaUpdate( Action* In, std::function< void() > Callback );
        virtual void OnCoinFlip( Action* In, std::function< void() > Callback );
        virtual void OnBlitzStart( Action* In, std::function< void() > Callback );
        virtual void OnBlitzQuery( Action* In, std::function< void() > Callback );
        virtual void OnBlitzError( Action* In, std::function< void() > Callback );
        virtual void OnBlitzSuccess( Action* In, std::function< void() > Callback );
        virtual void OnAttackError( Action* In, std::function< void() > Callback );
        virtual void OnMatchStart( Action* In, std::function< void() > Callback );
        virtual void OnTurnStart( Action* In, std::function< void() > Callback );
        virtual void OnMarshalStart( Action* In, std::function< void() > Callback );
        virtual void OnAttackStart( Action* In, std::function< void() > Callback );
        virtual void OnBlockStart( Action* In, std::function< void() > Callback );
        virtual void OnTurnFinish( Action* In, std::function< void() > Callback );
        virtual void OnDamageStart( Action* In, std::function< void() > Callback );
        virtual void OnDamage( Action* In, std::function< void() > Callback );
        virtual void OnCombat( Action* In, std::function< void() > Callback );
        virtual void OnStaminaUpdate( Action* In, std::function< void() > Callback );
        virtual void OnBoardCleanup( Action* In, std::function< void() > Callback );
        virtual void OnAttackersSet( Action* In, std::function< void() > Callback );
        
        void OnCardDied( CardEntity* Target );
        virtual void OnActionQueue();
        
    private:
        
        void _OnEndOfBranch( uint32_t QueueId );
        bool TurnFinished;
        
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
