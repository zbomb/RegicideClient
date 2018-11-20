//
//    IntroScene.hpp
//    Regicide Mobile
//
//    Created: 10/9/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

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
