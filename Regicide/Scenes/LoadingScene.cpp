//
//    LoadingScene.cpp
//    Regicide Mobile
//
//    Created: 11/11/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#include "LoadingScene.hpp"


using namespace cocos2d;

Scene* LoadingScene::createScene()
{
    auto ret = LoadingScene::create();
    if( ret )
    {
        ret->setName( "LoadingScene" );
        ret->setTag( TAG_LOADING );
    }
    
    return ret;
}

bool LoadingScene::init()
{
    if( !Scene::init() )
        return false;
    
    auto dir = Director::getInstance();
    auto Origin = dir->getVisibleOrigin();
    auto Size = dir->getVisibleSize();
    float FontSize = Size.width / 25.f;
    
    // Background Color
    Background = DrawNode::create();
    Background->drawSolidRect( Origin, Origin + Size, Color4F( 0.15f, 0.15f, 0.15f, 1.f ) );
    addChild( Background, 2 );
    
    // Header Text
    Header = Label::createWithTTF( "Loading...", "fonts/arial.ttf", FontSize );
    Header->setAlignment( TextHAlignment::CENTER, TextVAlignment::CENTER );
    Header->setAnchorPoint( Vec2( 0.5f, 0.5f ) );
    Header->setPosition( Vec2( Origin.x + Size.width / 2.f, Origin.y + Size.height * 0.85f ) );
    addChild( Header, 5 );
    
    // Decription Text
    Description = Label::createWithTTF( "", "fonts/arial.ttf", FontSize * 0.7f );
    Description->setAlignment( TextHAlignment::CENTER, TextVAlignment::CENTER );
    Description->setAnchorPoint( Vec2( 0.5f, 0.5f ) );
    Description->setPosition( Vec2( Origin.x + Size.width / 2.f, Origin.y + Size.height * 0.65f ) );
    addChild( Description, 8 );
    
    // Loading Bar
    Progress = ui::LoadingBar::create( "loading_bar.png" );
    Progress->setAnchorPoint( Vec2( 0.5f, 0.5f ) );
    Progress->setPercent( 0.f );
    Progress->setPosition( Vec2( Origin.x + Size.width / 2.f, Origin.y + Size.height * 0.5f ) );
    addChild( Progress, 5 );
    
    
    return true;
}

void LoadingScene::UpdateProgress( float inProgress, const std::string& inDesc /* = std::string() */ )
{
    if( Progress )
        Progress->setPercent( inProgress );
    
    if( Description && !inDesc.empty() )
        Description->setString( inDesc );
}

LoadingScene::~LoadingScene()
{
    
}
