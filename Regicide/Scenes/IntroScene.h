/*==========================================================================================
	Regicide Mobile

	IntroScene.h
	(c) 2018 Zachary Berry
=============================================================================================*/
#pragma once

#include "cocos2d.h"

using namespace cocos2d;

class IntroScene : public cocos2d::Scene
{

public:

	// Cocos2d Implementation
	static cocos2d::Scene* createScene();
	virtual bool init();
	CREATE_FUNC( IntroScene );

};