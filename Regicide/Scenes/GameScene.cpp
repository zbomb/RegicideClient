//
//    GameScene.cpp
//    Regicide Mobile
//
//    Created: 11/11/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#include "GameScene.hpp"
#include "Game/World.hpp"
#include "Game/EntityBase.hpp"
#include "UI/ExitOverlay.hpp"
#include "Game/GameModeBase.hpp"

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
    addChild( Test, -1 );
    
    cardLayer = CardLayer::create();
    addChild( cardLayer, 0 );
    
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
    addChild( ExitButton, -1 );
    
    FinishButton = ui::Button::create( "generic_button.png" );
    FinishButton->setCascadeOpacityEnabled( true );
    FinishButton->setAnchorPoint( Vec2( 0.5f, 0.5f ) );
    FinishButton->setPosition( Vec2( Origin.x + Size.width * 0.9f, Origin.y + Size.height * 0.4f ) );
    FinishButton->addTouchEventListener( [&]( cocos2d::Ref* Caller, ui::Widget::TouchEventType Type )
                                      {
                                          if( Type == ui::Widget::TouchEventType::ENDED )
                                          {
                                              auto world = Game::World::GetWorld();
                                              auto GM = world->GetGameMode();
                                              GM->FinishTurn();
                                          }
                                      });
    addChild( FinishButton, -1 );
    
    FinishLabel = cocos2d::Label::createWithTTF( "Finish", "fonts/arial.ttf", 35 );
    FinishLabel->setAnchorPoint( cocos2d::Vec2( 0.5f, 0.5f ) );
    FinishLabel->setPosition( FinishButton->getContentSize() * 0.5f );
    
    FinishButton->addChild( FinishLabel );
    FinishButton->setOpacity( 0.f );
    
    // Add State Labels
    TurnLabel = cocos2d::Label::createWithTTF( "", "fonts/arial.ttf", 60 );
    TurnLabel->setAnchorPoint( cocos2d::Vec2( 0.f, 0.5f ) );
    TurnLabel->setPosition( cocos2d::Vec2( Origin.x + 20.f, Origin.y + Size.height * 0.5f ) );
    addChild( TurnLabel, -1 );
    
    PlayerLabel = cocos2d::Label::createWithTTF( "", "fonts/arial.ttf", 60 );
    PlayerLabel->setAnchorPoint( cocos2d::Vec2( 1.f, 0.5f ) );
    PlayerLabel->setPosition( cocos2d::Vec2( Origin.x + Size.width - 20.f, Origin.y + Size.height * 0.5f ) );
    addChild( PlayerLabel, -1 );
    
    return true;
}

GameScene::~GameScene()
{
    
}

void GameScene::ExitGame()
{
    auto Overlay = ExitOverlay::create();
    Overlay->setGlobalZOrder( 100000 );
    addChild( Overlay, 100 );
    
    Overlay->runAction( cocos2d::Sequence::create( FadeIn::create( 0.5f ), CallFunc::create(
                        [ = ]()
                        {
                            auto* App = AppDelegate::GetInstance();
                            if( App )
                                App->ExitToMenu();
                        }), NULL ) );
}

void GameScene::UpdateTurnState( const std::string& In )
{
    if( TurnLabel )
    {
        TurnLabel->setString( In );
    }
}

void GameScene::UpdatePlayerTurn( const std::string& In )
{
    if( PlayerLabel )
    {
        PlayerLabel->setString( In );
    }
}

void GameScene::ShowFinishButton()
{
    if( FinishButton )
    {
        FinishButton->runAction( cocos2d::FadeIn::create( 0.25f ) );
    }
}

void GameScene::HideFinishButton()
{
    if( FinishButton )
    {
        FinishButton->runAction( cocos2d::FadeOut::create( 0.25f ) );
    }
}

void GameScene::RedrawBlockers()
{
    if( cardLayer )
        cardLayer->RedrawBlockers();
}
