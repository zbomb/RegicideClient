#include "ConnectingPopup.h"
#include "cocos/base/CCEventListenerTouch.h"



using namespace cocos2d;
using namespace cocos2d::ui;


bool ConnectingPopup::init()
{
	log( "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" );
	// Initialize Parent
	if( !LayerColor::initWithColor( Color4B( 0, 0, 0, 180 ) ) )
	{
		return false;
	}

	auto screenSize = Director::getInstance()->getVisibleSize();
	int FontScale = screenSize.height / 1080.f * 60.f;

	auto thisSize = getContentSize();

	Listener = EventListenerTouchOneByOne::create();
	if( Listener && _eventDispatcher )
	{
		Listener->setSwallowTouches( true );
		Listener->onTouchBegan = [=] ( cocos2d::Touch* Args, cocos2d::Event* TheEvent ) {
			
			return true;
		};
		_eventDispatcher->addEventListenerWithFixedPriority( Listener, -100 );
	}

	// Create Child Nodes
	Message = Label::createWithTTF( "Connecting...", "fonts/arial.ttf", FontScale );
	if( Message )
	{
		Message->setTextColor( Color4B( 240, 240, 250, 255 ) );
		Message->setAnchorPoint( Vec2( 0.5f, 0.5f ) );
		Message->setPosition( thisSize.width / 2.f, thisSize.height / 2.f );
		Message->setMaxLineWidth( thisSize.width * 0.5f );
		Message->setOverflow( Label::Overflow::RESIZE_HEIGHT );

		this->addChild( Message, 8, "Message" );
	}
	else
	{
		log( "[UI ERROR] Failed to create loading screen text!" );
	}

	Draw = DrawNode::create();
	if( Draw )
	{
		Draw->drawSolidRect( Vec2( thisSize.width * 0.2f, thisSize.height * 0.4f ), Vec2( thisSize.width * 0.8f, thisSize.height * 0.6f ), Color4F( 0.2f, 0.2f, 0.2f, 1.f ) );
		Draw->drawSolidRect( Vec2( thisSize.width * 0.2f + 3, thisSize.height * 0.4 + 3 ), Vec2( thisSize.width * 0.8f - 3, thisSize.height * 0.6f - 3 ), Color4F( 0.1f, 0.1f, 0.1f, 1.f ) );
		
		this->addChild( Draw, 7, "Draw" );
	}
	else
	{
		log( "[UI ERROR] Failed to create draw node for loading screen popup!" );
	}

	// Start receiving updates
	//scheduleUpdate();

	return true;
}
/*
void ConnectingPopup::update( float Delta )
{
	// Check if we received results since last frame
	if( _bResults )
	{
		//UpdateMutex.lock();
		ShowResult( _resultVal, _resultText );
		unscheduleUpdate();

		_bResults = false;
		//UpdateMutex.unlock();
	}
}
*/
/*
void ConnectingPopup::PushResult( EConnectResult Result, std::string AdditionalInfo )
{
	//UpdateMutex.lock();
	_bResults = true;
	_resultText = AdditionalInfo;
	_resultVal = Result;
	//UpdateMutex.unlock();
}
 */

/*
void ConnectingPopup::ShowResult( EConnectResult Result, std::string AdditionalInfo = nullptr )
{
	if( Message )
	{
		this->removeChild( Message, true );
		Message = nullptr;
	}

	Color4B textColor;
	std::string textContent;

	auto screenSize = Director::getInstance()->getVisibleSize();
	int FontScale = screenSize.height / 1080.f * 60.f;

	auto thisSize = this->getContentSize();

	if( Result == EConnectResult::Success )
	{
		textColor = Color4B( 240, 240, 240, 255 );
		textContent = "Connected!";
	}
	else if( Result == EConnectResult::ConnectionFailure )
	{
		textColor = Color4B( 210, 40, 40, 255 );
		textContent = "Failed to connect! ";
		FontScale *= 0.6f;
		if( AdditionalInfo.size() == 0 )
		{
			textContent.append( "Check internet connection and retry" );
		}
		else
		{
			textContent.append( AdditionalInfo );
		}
	}
	else
	{
		textColor = Color4B( 210, 40, 40, 255 );
		textContent = "An error occurred! ";
		FontScale *= 0.6f;
		if( AdditionalInfo.size() == 0 )
		{
			textContent.append( "Restart the game and try again." );
		}
		else
		{
			textContent.append( AdditionalInfo );
		}
	}

	Message = Label::createWithTTF( textContent, "fonts/arial.ttf", FontScale );
	if( Message )
	{
		Message->setAnchorPoint( Vec2( 0.5f, 0.5f ) );
		//Message->setTextColor( textColor );
		Message->setPosition( Vec2( screenSize.width / 2.f, screenSize.height / 2.f ) );
		Message->setMaxLineWidth( this->getContentSize().width * 0.5f );
		Message->setOverflow( Label::Overflow::RESIZE_HEIGHT );

		this->addChild( Message, 8, "Message" );
	}
	else
	{
		log( "[UI ERROR] Failed to update popup text!" );
	}
}
*/

void ConnectingPopup::Shutdown()
{
	if( Listener )
	{
		Listener->setSwallowTouches( false );
	}
}
