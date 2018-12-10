//
//    SingleplayerGameMode.cpp
//    Regicide Mobile
//
//    Created: 11/10/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#include "SingleplayerGameMode.hpp"
#include "World.hpp"
#include "SingleplayerAuthority.hpp"
#include "CardEntity.hpp"
#include "HandEntity.hpp"
#include "GraveyardEntity.hpp"
#include "FieldEntity.hpp"
#include "UI/CardViewer.hpp"
#include "Scenes/GameScene.hpp"

using namespace Game;


SingleplayerGameMode::SingleplayerGameMode()
: GameModeBase()
{
    using namespace std::placeholders;
    SetActionCallback( "Win", std::bind( &SingleplayerGameMode::Action_GameWon, this, _1, _2 ) );
}

void SingleplayerGameMode::Initialize()
{

}

void SingleplayerGameMode::Cleanup()
{
    GameModeBase::Cleanup();
}


void SingleplayerGameMode::Action_GameWon( Action* In, std::function< void() > Callback )
{
    // Cast to WinEvent
    auto Win = dynamic_cast< WinAction* >( In );
    if( !Win )
    {
        cocos2d::log( "[GM] Win event occured, but unable to cast to WinAction!" );
        
        FinishAction( Callback );
        return;
    }
    
    cocos2d::Sprite* Sprite;
    if( Win->bDidWin )
        Sprite = cocos2d::Sprite::create( "win_banner.png" );
    else
        Sprite = cocos2d::Sprite::create( "loose_banner.png" );
    
    auto Dir = cocos2d::Director::getInstance();
    auto Size = Dir->getVisibleSize();
    auto Origin = Dir->getVisibleOrigin();
    
    Sprite->setGlobalZOrder( 99999 );
    Sprite->setAnchorPoint( cocos2d::Vec2( 0.5f, 0.5f ) );
    Sprite->setPosition( Origin + Size * 0.5f );
    
    GetScene()->addChild( Sprite );
    DisableSelection();
    
    Sprite->runAction( cocos2d::FadeIn::create( 0.3f ) );
    Dir->getScheduler()->schedule( [=]( float f ) { if( Callback ) Callback(); }, this, 0.35f, 0, 0.f, false, "WinCallback" );
    
    // Clear Card Overlays/Glows
    auto Cards = IEntityManager::GetInstance().GetAllCards();
    for( auto It = Cards.begin(); It != Cards.end(); It++ )
    {
        if( *It )
        {
            (*It)->ClearOverlay();
            (*It)->ClearHighlight();
        }
    }
    
    Dir->getScheduler()->schedule( [=]( float f )
    {
        auto parentScene = GetScene();
        if( parentScene )
        {
            auto Scene = dynamic_cast< GameScene* >( parentScene );
            if( Scene )
            {
                Scene->ExitGame();
            }
        }
        
    }, this, 5.f, 0, 0.f, false, "ExitTimer" );
    
    auto parentScene = GetScene();
    if( parentScene )
    {
        auto Scene = dynamic_cast< GameScene* >( parentScene );
        if( Scene )
        {
            Scene->HideFinishButton();
        }
    }
    
    FinishAction( Callback );
}
