//
//	DescriptionText.cpp
//	Regicide Mobile
//
//	Created: 11/27/18
//	Updated: 11/27/18
//
//	© 2018 Zachary Berry, All Rights Reserved
//

#include "DescriptionText.hpp"


DescriptionText* DescriptionText::Create( const std::string& InDesc, float inWide, bool bDrawSep )
{
    auto Output = new (std::nothrow) DescriptionText();
    if( Output && Output->init( InDesc, inWide, bDrawSep ) )
    {
        Output->autorelease();
        return Output;
    }
    else
    {
        CC_SAFE_DELETE( Output );
        Output = nullptr;
        return nullptr;
    }
}

bool DescriptionText::init( const std::string &InDesc, float inWide, bool bDrawSep )
{
    if( !Widget::init() )
        return false;
    
    // Create Label
    Text = cocos2d::Label::createWithTTF( InDesc, "fonts/arial.ttf", 28 );
    Text->enableWrap( true );
    Text->setDimensions( inWide - 12.f, 0.f );
    Text->setAlignment( cocos2d::TextHAlignment::LEFT, cocos2d::TextVAlignment::TOP );
    Text->setTextColor( cocos2d::Color4B( 255, 255, 255, 255 ) );
    Text->setAnchorPoint( cocos2d::Vec2( 0.f, 0.5f ) );
    addChild( Text, 2 );
    
    Draw = cocos2d::DrawNode::create();
    addChild( Draw, 1 );
    
    bDrawSeperator = bDrawSep;
    
    return true;
}

float DescriptionText::GetDesiredHeight()
{
    if( Text )
    {
        return Text->getContentSize().height + 18.f;
    }
    else
    {
        return 18.f;
    }
}

void DescriptionText::onSizeChanged()
{
    if( Text )
    {
        auto Size = getContentSize() * getScale();
        
        Text->setAnchorPoint( cocos2d::Vec2( 0.f, 1.f ) );
        Text->setPosition( cocos2d::Vec2( 6.f, Size.height / 2.f + Text->getContentSize().height / 2.f ) );
        
        if( Draw )
        {
            Draw->clear();
            
            if( bDrawSeperator )
            {
                Draw->drawSegment( cocos2d::Vec2( 6.f, Size.height ), cocos2d::Vec2( Size.width - 6.f, Size.height ), 1.f, cocos2d::Color4F( 0.95f, 0.95f, 0.95f, 1.f ) );
            }
        }
    }
}
