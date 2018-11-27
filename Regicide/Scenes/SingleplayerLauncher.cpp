//
//    SingleplayerLauncher.cpp
//    Regicide Mobile
//
//    Created: 11/9/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#include "SingleplayerLauncher.hpp"
#include "PopTransition.hpp"
#include "AppDelegate.hpp"
#include "CMS/IContentSystem.hpp"

USING_NS_CC;


Scene* SingleplayerLauncherScene::createScene()
{
    auto ret = SingleplayerLauncherScene::create();
    ret->setTag( REG_TAG_LAUNCHER );
    ret->setName( "SingleplayerLauncher" );
    return ret;
}

bool SingleplayerLauncherScene::init()
{
    if( !Scene::init() )
        return false;
    
    auto sceneSize = Director::getInstance()->getVisibleSize();
    auto sceneOrigin = Director::getInstance()->getVisibleOrigin();
    int FontScale = sceneSize.width / 20;
    float Padding = sceneSize.width / 50.f;
    
    Draw = DrawNode::create();
    Draw->drawSolidRect( sceneOrigin, sceneOrigin + sceneSize, Color4F( 0.15f, 0.15f, 0.15f, 1.f ) );
    this->addChild( Draw, 1 );
    
    Header = Label::createWithTTF( "Singleplayer", "fonts/arial.ttf", FontScale, Size::ZERO, TextHAlignment::CENTER, TextVAlignment::CENTER );
    Header->setPosition( Vec2( sceneOrigin.x + sceneSize.width / 2, sceneOrigin.y + sceneSize.height * 0.85f ) );
    this->addChild( Header, 10 );
    
    BackButton = ui::Button::create( "backbutton_arrow_right.png", "backbutton_arrow_right.png", "backbutton_arrow_right.png" );
    BackButton->setAnchorPoint( Vec2( 1.f, 0.f ) );
    BackButton->setPosition( Vec2( sceneOrigin.x + sceneSize.width - Padding, sceneOrigin.y + Padding ) );
    BackButton->addTouchEventListener([&]( Ref* sender, ui::Widget::TouchEventType type ){
        switch (type)
        {
            case ui::Widget::TouchEventType::BEGAN:
                break;
            case ui::Widget::TouchEventType::ENDED:
                this->OnBackClicked( sender );
                break;
            default:
                break;
        }
    });
    
    this->addChild( BackButton, 20 );
    
    LaunchButton = ui::Button::create( "launch_button_normal.png", "launch_button_pressed.png", "launch_button_disabled.png" );
    LaunchButton->setAnchorPoint( Vec2( 1.f, 0.f ) );
    LaunchButton->setPosition( Vec2( sceneOrigin.x + sceneSize.width / 2.f, sceneOrigin.y + Padding ) );
    LaunchButton->addTouchEventListener( [&]( Ref* Sender, ui::Widget::TouchEventType Type )
        {
            
            if( Type == ui::Widget::TouchEventType::ENDED )
            {
                this->OnLaunchClicked( Sender );
            }
            
        } );
    addChild( LaunchButton, 20 );
    
    
    return true;
}

void SingleplayerLauncherScene::OnBackClicked( Ref* Caller )
{
    auto dir = Director::getInstance();
    dir->pushScene( pop_scene_with< cocos2d::TransitionSlideInR >::create( 0.5f ) );
}

void SingleplayerLauncherScene::OnLaunchClicked( cocos2d::Ref *Caller )
{
    auto app = AppDelegate::GetInstance();
    PracticeArguments Args;
    
    auto act = Regicide::IContentSystem::GetAccounts();
    auto Pl = act->GetLocalAccount();
    
    Args.PlayerName = Pl->Info.DisplayName;
    Args.OpponentName = "Test Bot";
    
    Regicide::Deck TestDeck;
    TestDeck.Name = "Test Deck";
    TestDeck.KingId = 1;
    TestDeck.Id = 1;
    
    for( int i = 0; i < 1; i++ )
    {
        Regicide::Card newCard;
        newCard.Id = 5;
        newCard.Ct = 20;
        
        TestDeck.Cards.push_back( newCard );
    }
    
    Regicide::Card otherCard;
    otherCard.Id = 6;
    otherCard.Ct = 10;
    TestDeck.Cards.push_back( otherCard );
    
    Args.PlayerDeck = TestDeck;
    Args.OpponentDeck = TestDeck;
    
    Args.Difficulty = AIDifficulty::Normal;
    Args.LevelBackground = 5;
    
    app->LaunchPractice( Args );
}
