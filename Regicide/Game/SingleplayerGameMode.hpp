//
//    SingleplayerGameMode.hpp
//    Regicide Mobile
//
//    Created: 11/10/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#pragma once

#include "GameModeBase.hpp"
#include "CardEntity.hpp"
#include <chrono>

class CardViewer;

namespace Game
{
    
    class Player;
    
    class SingleplayerGameMode : public GameModeBase
    {
        
    public:
        
        virtual void Cleanup();
        
    protected:
        
        virtual void Initialize();
        
        /*=============================================================================================
            Input Layer
         =============================================================================================*/
        
    public:
        
        virtual void TouchBegan( cocos2d::Touch* inTouch, CardEntity* inCard );
        virtual void TouchEnd( cocos2d::Touch* inTouch, CardEntity* inCard );
        virtual void TouchMoved( cocos2d::Touch* inTouch );
        virtual void TouchCancel( cocos2d::Touch* inTouch );
        
        void OnCardClicked( CardEntity* inCard );
        void OpenGraveyardViewer( GraveyardEntity* Grave );
        void OpenCardViewer( CardEntity* inCard );
        void OpenHandViewer( CardEntity* inCard );
        void CloseGraveyardViewer();
        void CloseCardViewer();
        void CloseHandViewer();
        bool OnCardDragDrop( CardEntity* inCard, cocos2d::Touch* Info );
        
    protected:
        
        CardEntity* _touchedCard = nullptr;
        CardEntity* _viewCard = nullptr;
        bool _bDrag = false;
        cocos2d::Vec2 _DragOffset = cocos2d::Vec2::ZERO;
        
        CardViewer* _Viewer = nullptr;
        void _DoCloseViewer();

        friend class SingleplayerLauncher;
        
    };
}
