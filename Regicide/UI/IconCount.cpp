//
//	IconCount.cpp
//	Regicide Mobile
//
//	Created: 11/26/18
//	Updated: 11/26/18
//
//	© 2018 Zachary Berry, All Rights Reserved
//

#include "IconCount.hpp"


IconCount* IconCount::Create( const std::string& inTexture, int inCount, int inFont )
{
    auto Output = new (std::nothrow) IconCount();
    if( Output && Output->init( inTexture, inCount, inFont ) )
    {
        Output->autorelease();
    }
    else
    {
        delete Output;
        Output = nullptr;
    }
    
    return Output;
}

IconCount::IconCount()
: Node()
{
    Icon = nullptr;
    Count = nullptr;
}

IconCount::~IconCount()
{
    Icon = nullptr;
    Count = nullptr;
}

bool IconCount::init( const std::string& inTexture, int inCount, int inFont )
{
    if( !Node::init() )
        return false;
    
    Icon = cocos2d::Sprite::create( inTexture );
    Icon->setAnchorPoint( cocos2d::Vec2( 0.f, 0.5f ) );
    addChild( Icon, 1 );
    
    Count = cocos2d::Label::createWithTTF( std::to_string( inCount ), "fonts/arial.ttf", inFont );
    Count->setAnchorPoint( cocos2d::Vec2( 0.f, 0.5f ) );
    Count->setTextColor( cocos2d::Color4B( 255, 255, 255, 255 ) );
    addChild( Count, 2 );
    
    auto IconSize = Icon->getContentSize();
    auto CountSize = Count->getContentSize();
    float FinalHeight = ( IconSize.height > CountSize.height ? IconSize.height : CountSize.height ) + 8.f;
    
    setContentSize( cocos2d::Size( 4.f + IconSize.width + 8.f + CountSize.width + 4.f, FinalHeight ) );

    Icon->setPosition( cocos2d::Vec2( 4.f, FinalHeight / 2.f ) );
    Count->setPosition( cocos2d::Vec2( 4.f + IconSize.width + 8.f, FinalHeight / 2.f ) );
    
    return true;
}

void IconCount::UpdateCount( int inCount )
{
    if( Icon )
    {
        Count->setString( std::to_string( inCount ) );
    }
}

void IconCount::SetTextColor( const cocos2d::Color4B& inColor )
{
    if( Count )
    {
        Count->setTextColor( inColor );
    }
}

