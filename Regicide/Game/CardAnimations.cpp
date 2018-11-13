//
//  CardAnimations.cpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/12/18.
//

#include "CardAnimations.hpp"


CardFlipX* CardFlipX::Create( float inTime, float inStart, float inEnd, float inDepth )
{
    // Create new animation
    CardFlipX* newAnim = new (std::nothrow) CardFlipX();
    newAnim->autorelease();
    
    // Init and return it
    return newAnim->Init( inTime, inStart, inEnd, inDepth ) ? newAnim : nullptr;
}

bool CardFlipX::Init( float inTime, float inStart, float inEnd, float inDepth )
{
    if( RotateTo::initWithDuration( inTime, inEnd, inEnd ) )
    {
        Depth = inDepth;
        Start = inStart;
        End = inEnd;
        
        Target = nullptr;
        
        return true;
    }
    
    return false;
}

void CardFlipX::startWithTarget( cocos2d::Node *inTarget )
{
    RotateTo::startWithTarget( inTarget) ;
    Radius = inTarget->getContentSize().height / 2.0f;
    
    calculateAngles( Start, Diff, End );
    
    Target = dynamic_cast< cocos2d::Sprite* >( inTarget );
    CC_ASSERT( Target );
}

float CardFlipX::diffCalc( float Delta )
{
    return( ( Start + Diff * Delta ) * 0.0174532925f );
}

void CardFlipX::update( float Time )
{
    cocos2d::PolygonInfo pi = Target->getPolygonInfo();
    
    float rad = diffCalc( Time );
    float width = Target->getContentSize().width;
    
    // Update sprite verts
    pi.triangles.verts[0].vertices.y = cosf( -rad ) * Radius + Radius;
    pi.triangles.verts[0].vertices.x = ( sinf( -rad ) * Radius ) / Depth;
    
    pi.triangles.verts[1].vertices.y = cosf( M_PI - rad ) * Radius + Radius;
    pi.triangles.verts[1].vertices.x = ( sinf( M_PI - rad ) * Radius ) / Depth;
    
    pi.triangles.verts[2].vertices.y = cosf( rad ) * Radius + Depth;
    pi.triangles.verts[2].vertices.x = ( sinf( rad ) * Radius ) / Depth + width;
    
    pi.triangles.verts[3].vertices.y = cosf( M_PI + rad ) * Radius + Radius;
    pi.triangles.verts[3].vertices.x = ( sinf( M_PI + rad ) * Radius ) / Depth + width;
    
    Target->setPolygonInfo( pi );
}

// FakeRotateY

CardFlipY* CardFlipY::Create( float inTime, float inStart, float inEnd, float inDepth )
{
    auto Output = new (std::nothrow) CardFlipY();
    Output->autorelease();
    
    return Output->Init( inTime, inStart, inEnd, inDepth ) ? Output : nullptr;
}

bool CardFlipY::Init( float inTime, float inStart, float inEnd, float inDepth )
{
    if( RotateTo::initWithDuration( inTime, inEnd, inEnd ) )
    {
        Depth = inDepth;
        Start = inStart;
        End = inEnd;
        
        Target = nullptr;
        
        return true;
    }
    
    return false;
}

void CardFlipY::startWithTarget( cocos2d::Node* inTarget )
{
    RotateTo::startWithTarget( inTarget );
    Radius = inTarget->getContentSize().width / 2.0;
    
    calculateAngles( Start, Diff, End );
    
    Target = dynamic_cast< cocos2d::Sprite* >( inTarget );
    CC_ASSERT( Target );
}

float CardFlipY::diffCalc( float Delta )
{
    return( ( Start + Diff * Delta ) * 0.0174532925f );
}

void CardFlipY::update( float Time )
{
    cocos2d::PolygonInfo pi = Target->getPolygonInfo();
    
    float rad = diffCalc( Time );
    float height = Target->getContentSize().height;
    
    // Update verts
    pi.triangles.verts[0].vertices.x = cosf( M_PI + rad ) * Radius + Radius;
    pi.triangles.verts[0].vertices.y = ( sinf( M_PI + rad ) * Radius ) / Depth + height;
    
    pi.triangles.verts[1].vertices.x = cosf( M_PI - rad ) * Radius + Radius;
    pi.triangles.verts[1].vertices.y = ( sinf( M_PI - rad ) * Radius ) / Depth;
    
    pi.triangles.verts[2].vertices.x = cosf( rad ) * Radius + Radius;
    pi.triangles.verts[2].vertices.y = ( sinf( rad ) * Radius ) / Depth + height;
    
    pi.triangles.verts[3].vertices.x = cosf( -rad ) * Radius + Radius;
    pi.triangles.verts[3].vertices.y = ( sinf( -rad ) * Radius ) / Depth;
    
    Target->setPolygonInfo( pi );
}



CardFlipTex* CardFlipTex::Create( float inTime, cocos2d::Texture2D *newTexture )
{
    auto Output = new (std::nothrow) CardFlipTex();
    Output->autorelease();
    
    return Output->Init( inTime, newTexture ) ? Output : nullptr;
}

bool CardFlipTex::Init( float inTime, cocos2d::Texture2D* newTexture )
{
    if( ActionInterval::initWithDuration( inTime ) )
    {
        Time = inTime;
        Texture = newTexture;
        bFlipped = false;
        Target = nullptr;
        
        return true;
    }
    
    return false;
}

void CardFlipTex::startWithTarget( cocos2d::Node* inTarget )
{
    ActionInterval::startWithTarget( inTarget );
    
    Target = dynamic_cast< cocos2d::Sprite* >( inTarget );
    CC_ASSERT( Target );
}

void CardFlipTex::update( float Time )
{
    // TODO: Fade between two textures
    if( !bFlipped )
    {
        Target->setTexture( Texture );
        bFlipped = true;
    }
}
