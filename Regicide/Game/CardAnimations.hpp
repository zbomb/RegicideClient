//
//  CardAnimations.hpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/12/18.
//

#include "cocos2d.h"



class CardFlipX : public cocos2d::RotateTo
{
    
public:
    
    static CardFlipX* Create( float inTime, float inStart, float inEnd, float inDepth = 5.0f );
    bool Init( float inTime, float inStart, float inEnd, float inDepth );
    
    virtual ~CardFlipX() {}
    
protected:
    
    virtual void startWithTarget( cocos2d::Node *inTarget );
    virtual void update( float Time ) ;
    
private:
    
    cocos2d::Sprite* Target;
    
    float Start;
    float End;
    float Diff;
    float Radius;
    float Depth;
    
    float diffCalc( float Delta );
};

class CardFlipY : public cocos2d::RotateTo
{
    
public:
    
    static CardFlipY* Create( float inTime, float inStart, float inEnd, float inDepth = 5.0f );
    bool Init( float inTime, float inStart, float inEnd, float inDepth );
    
    virtual ~CardFlipY() {}
    
protected:
    
    virtual void startWithTarget( cocos2d::Node* inTarget );
    virtual void update( float Time );
    
private:
    
    cocos2d::Sprite* Target;
    
    float Start;
    float End;
    float Diff;
    float Radius;
    float Depth;
    
    float diffCalc( float Delta );
};

class CardFlipTex : public cocos2d::ActionInterval
{
    
public:
    
    static CardFlipTex* Create( float inTime, cocos2d::Texture2D* newTexture );
    bool Init( float inTime, cocos2d::Texture2D* newTexture );
    
    virtual ~CardFlipTex() {}
    
protected:
    
    virtual void startWithTarget( cocos2d::Node* inTarget );
    virtual void update( float Time );
    
    float Time;
    cocos2d::Texture2D* Texture;
    cocos2d::Sprite* Target;
    bool bFlipped;
};
