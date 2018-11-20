//
//    ConnectingPopup.hpp
//    Regicide Mobile
//
//    Created: 10/9/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#pragma once

#include "cocos/cocos2d.h"
#include "cocos/2d/CCLayer.h"
#include "cocos/ui/UILoadingBar.h"


using namespace cocos2d;
using namespace cocos2d::ui;


class ConnectingPopup : public cocos2d::LayerColor
{
	
public:

	virtual bool init();
	CREATE_FUNC( ConnectingPopup );

	void Shutdown();
	//void PushResult( EConnectResult Result, std::string AdditionalInfo = nullptr );

	//virtual void update( float delta ) override;

private:

	LoadingBar* Indicator	= nullptr;
	Label* Message			= nullptr;
	DrawNode* Draw			= nullptr;

	EventListenerTouchOneByOne* Listener;

	bool _bResults				= false;
	std::string _resultText		= nullptr;
	//EConnectResult _resultVal	= EConnectResult::ConnectionFailure;

	//std::mutex UpdateMutex;

	//void ShowResult( EConnectResult Result, std::string AdditionalInfo = nullptr );

};
