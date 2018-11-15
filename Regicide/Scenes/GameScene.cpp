//
//  GameScene.cpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/11/18.
//

#include "GameScene.hpp"
#include "Game/World.hpp"
#include "Game/EntityBase.hpp"
#include "UI/ExitOverlay.hpp"


using namespace cocos2d;

Scene* GameScene::createScene()
{
    auto ret = GameScene::create();
    if( ret )
    {
        ret->setName( "GameScene" );
        ret->setTag( TAG_GAME );
    }
    
    return ret;
}

bool GameScene::init()
{
    if( !Scene::init() )
        return false;
    
    
    auto dir = Director::getInstance();
    auto Origin = dir->getVisibleOrigin();
    auto Size = dir->getVisibleSize();
    
    auto Test = Label::createWithTTF( "GAME SCENE TEST", "fonts/arial.ttf", Size.width / 20.f );
    Test->setAlignment( TextHAlignment::CENTER, TextVAlignment::CENTER );
    Test->setAnchorPoint( Vec2( 0.5f, 0.5f ) );
    Test->setPosition( Vec2( Origin.x + Size.width / 2.f, Origin.y + Size.height / 2.f ) );
    addChild( Test, 5 );
    
    cardLayer = CardLayer::create();
    addChild( cardLayer, 7 );
    
    ExitButton = ui::Button::create( "launch_button_normal.png" );
    ExitButton->setAnchorPoint( Vec2( 0.f, 1.f ) );
    ExitButton->setPosition( Vec2( Origin.x + 20.f, Origin.y + Size.height - 20.f ) );
    ExitButton->addTouchEventListener( [&]( cocos2d::Ref* Caller, ui::Widget::TouchEventType Type )
    {
        
        if( Type == ui::Widget::TouchEventType::ENDED )
        {
            this->ExitGame();
        }
        
    } );
    addChild( ExitButton, 50 );
    
    
    
    return true;
}

GameScene::~GameScene()
{
    
}

void GameScene::ExitGame()
{
    auto Overlay = ExitOverlay::create();
    Overlay->setGlobalZOrder( 1000 );
    addChild( Overlay, 100 );
    
    Overlay->runAction( cocos2d::Sequence::create( FadeIn::create( 0.5f ), CallFunc::create(
                        [ = ]()
                        {
                            auto* App = AppDelegate::GetInstance();
                            if( App )
                                App->ExitToMenu();
                        }), NULL ) );
}
