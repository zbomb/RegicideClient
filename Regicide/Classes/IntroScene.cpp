/*==========================================================================================
	Regicide Mobile

	MainMenuScene.cpp
	(c) 2018 Zachary Berry
=============================================================================================*/

#include "IntroScene.h"
#include "SimpleAudioEngine.h"


using namespace cocos2d;
using namespace cocos2d::ui;


Scene* IntroScene::createScene()
{
	return IntroScene::create();
}

bool IntroScene::init()
{
	// Initialize parent class
	if( !Scene::init() )
	{
		return false;
	}

	auto sceneOrigin = Director::getInstance()->getVisibleOrigin();
	auto sceneSize = Director::getInstance()->getVisibleSize();

	int HeaderFontSize = ( sceneSize.width / 1920.f ) * 100.f;

	cocos2d::Vector< Node* > Items;

	// Create Intro Text
	auto IntroText = Label::createWithTTF( "Regicide", "fonts/Ringbearer Medium.ttf", HeaderFontSize );
	if( IntroText )
	{
		IntroText->setAnchorPoint( Vec2( 0.5f, 0.5f ) );
		IntroText->setTextColor( Color4B( 240, 240, 240, 255 ) );
		IntroText->setPosition( Vec2( sceneOrigin.x + sceneSize.width / 2.f,
									sceneOrigin.y + sceneSize .height / 2.f  + HeaderFontSize * 0.75 ) );

		Items.pushBack( IntroText );
	}
	else
	{
		log( "[UI ERROR] Failed to create intro text!" );
	}

	auto VersionText = Label::createWithTTF( "v 0.0.1a", "fonts/arial.ttf", HeaderFontSize * 0.22f );
	if( VersionText )
	{
		VersionText->setAnchorPoint( Vec2( 1.f, 0.f ) );
		VersionText->setTextColor( Color4B( 240, 240, 240, 255 ) );
		VersionText->setPosition( Vec2( sceneOrigin.x + sceneSize.width,
										sceneOrigin.y ) );

		Items.pushBack( VersionText );
	}
	else
	{
		log( "[UI ERROR] Failed to create version text" );
	}

	auto WebsiteText = Label::createWithTTF( "Visit www.RegicideMobile.com", "fonts/arial.ttf", HeaderFontSize * 0.3f );
	if( WebsiteText )
	{
		WebsiteText->setAnchorPoint( Vec2( 0.5f, 0.5f ) );
		WebsiteText->setTextColor( Color4B( 240, 240, 240, 255 ) );
		WebsiteText->setPosition( Vec2( sceneOrigin.x + sceneSize.width / 2.f,
										sceneOrigin.y + sceneSize.height / 2.f - HeaderFontSize * 0.25f ) );

		Items.pushBack( WebsiteText );
	}
	else
	{
		log( "[UI ERROR] Failed to create website text" );
	}

	// Run the fade in animation
	auto IntroAnimation = FadeIn::create( 2.f );
	for( Node* ValidNode : Items )
	{
		if( ValidNode )
		{
			ValidNode->runAction( IntroAnimation );
			this->addChild( ValidNode, 3 );
		}
	}

}