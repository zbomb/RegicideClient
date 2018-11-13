//
//  SingleplayerGameMode.hpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/10/18.
//

#pragma once

#include "GameModeBase.hpp"
#include "CardEntity.hpp"

namespace Game
{
    class Player;
    
    class SingleplayerGameMode : public GameModeBase
    {
        
    public:
        
        virtual Player* GetLocalPlayer();
        virtual Player* GetOpponent();
        
        virtual void Cleanup();
        
        virtual void TouchBegan( cocos2d::Touch* inTouch, CardEntity* inCard );
        virtual void TouchEnd( cocos2d::Touch* inTouch, CardEntity* inCard );
        virtual void TouchMoved( cocos2d::Touch* inTouch );
        virtual void TouchCancel( cocos2d::Touch* inTouch );
        
        void OnCardClicked( CardEntity* inCard );
        void OpenGraveyardViewer( GraveyardEntity* Grave );
        void OpenCardViewer( CardEntity* inCard );
        void OpenHandViewer( CardEntity* inCard );
        
    protected:
        
        virtual void Initialize();
        virtual void PostInitialize();
        
        CardEntity* _touchedCard = nullptr;
        
        friend class SingleplayerLauncher;
        
    };
}
