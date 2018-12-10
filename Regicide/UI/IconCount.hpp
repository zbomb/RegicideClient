//
//	IconCount.hpp
//	Regicide Mobile
//
//	Created: 11/26/18
//	Updated: 11/26/18
//
//	© 2018 Zachary Berry, All Rights Reserved
//

#include "cocos2d.h"


class IconCount : public cocos2d::Node
{
public:
    
    static IconCount* Create( const std::string& inTexture, int inCount, int inFont = 28 );
    
    IconCount();
    ~IconCount();
    
    virtual bool init( const std::string& inTexture, int inCount, int inFont );
    void UpdateCount( int inCount );
    void SetTextColor( const cocos2d::Color4B& inColor );
    
protected:
    
    cocos2d::Sprite* Icon;
    cocos2d::Label* Count;
};
