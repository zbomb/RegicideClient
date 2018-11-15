//
//  CardEntity.cpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/10/18.
//

#include "CardEntity.hpp"
#include "Player.hpp"
#include "CardAnimations.hpp"
#include "ICardContainer.hpp"

using namespace Game;



CardEntity::CardEntity()
    : EntityBase( "card" ), Sprite( nullptr ), OwningPlayer( nullptr ), Container( nullptr ),
    FullSizedTexture( nullptr ), FrontTexture( nullptr ), BackTexture( nullptr )
{
    Power       = 0;
    Stamina     = 0;
    ManaCost    = 0;
    
    _bDragging = false;
}

CardEntity::~CardEntity()
{
    if( Sprite && Sprite != NULL )
    {
        /*
        auto dir = cocos2d::Director::getInstance();
        auto sch = dir ? dir->getScheduler() : nullptr;
        
        if( sch )
        {
            sch->performFunctionInCocosThread( [ = ]()
                  {
                      if( Sprite )
                          Sprite->removeFromParent();
                  } );
        }
         */
        
        Sprite->removeFromParent();
    }
    
    OwningPlayer    = nullptr;
    Container       = nullptr;
    
    FrontTexture        = nullptr;
    BackTexture         = nullptr;
    FullSizedTexture    = nullptr;
    Sprite              = nullptr;
}

void CardEntity::Cleanup()
{
    EntityBase::Cleanup();
}

bool CardEntity::InDeck() const
{
    return Container && Container->GetTag() == TAG_DECK;
}

bool CardEntity::InHand() const
{
    return Container && Container->GetTag() == TAG_HAND;
}

bool CardEntity::InGrave() const
{
    return Container && Container->GetTag() == TAG_GRAVE;
}

bool CardEntity::OnField() const
{
    return Container && Container->GetTag() == TAG_FIELD;
}

bool CardEntity::Load( luabridge::LuaRef& inLua, Player* inOwner, cocos2d::TextureCache* Cache, bool Authority /* = false */ )
{
    if( !inLua.isTable() )
    {
        cocos2d::log( "[Card] ERROR: Failed to load, invalid card table passed into Load!" );
        return false;
    }

    // Check validity of the hook table
    if( !inLua[ "Power" ].isNumber() || !inLua[ "Stamina" ].isNumber() || !inLua[ "Name" ].isString() || !inLua[ "Texture" ].isString() || !inLua[ "LargeTexture" ].isString() || !inLua[ "Mana" ].isNumber() )
    {
        cocos2d::log( "[Card] ERROR: Failed to load, card table passed into load was missing values" );
        return false;
    }
    
    // Assign Members
    DisplayName     = inLua[ "Name" ].tostring();
    Power           = (uint16) int( inLua[ "Power" ] );
    Stamina         = (uint16) int( inLua[ "Stamina" ] );
    ManaCost        = (uint16) int( inLua[ "Mana" ] );

    FrontTextureName = inLua[ "Texture" ].tostring();
    LargeTextureName = inLua[ "LargeTexture" ].tostring();
    
    if( inLua[ "BackTexture" ].isString() )
    {
        BackTextureName = inLua[ "BackTexture" ].tostring();
    }
    
    bAllowDeckHooks = inLua[ "EnableDeckHooks" ];
    bAllowHandHooks = inLua[ "EnableHandHooks" ];
    bAllowPlayHooks = inLua[ "EnablePlayHooks" ];
    bAllowDeadHooks = inLua[ "EnableDeadHooks" ];
    
    OwningPlayer = inOwner;
    
    // Store Hooks
    if( inLua[ "Hooks" ].isTable() )
        Hooks = std::make_shared< luabridge::LuaRef >( inLua[ "Hooks" ] );
    else
        Hooks = std::make_shared< luabridge::LuaRef >( luabridge::newTable( inLua.state() ) );
    
    // Set Authority Flag
    (*Hooks)[ "Authority" ] = Authority;
    

    return true;
    
}

int CardEntity::LoadResources( const std::function< void() >& Callback )
{
    auto Cache = cocos2d::Director::getInstance()->getTextureCache();
    int Output = 1;
    
    if( FrontTextureName.empty() )
    {
        // DECIDE: Should be fatal loading error?
        cocos2d::log( "[Card] ERROR! Failed to load front-texture!" );
        FrontTexture = nullptr;
    }
    else
    {
        Cache->addImageAsync( FrontTextureName, [ = ]( cocos2d::Texture2D* t )
        {
            if( !t )
            {
                // DECIDE: Should be fatal?
                cocos2d::log( "[Card] ERROR: Failed to load front-side texture! %s", FrontTextureName.c_str() );
                this->FrontTexture = nullptr;
            }
            else
            {
                this->FrontTexture = t;
            }
            
            Callback();
        } );
        
        Output++;
    }
    
    if( LargeTextureName.empty() )
    {
        // DECIDE: Should be fatal loading error?
        cocos2d::log( "[Card] ERROR! Failed to load full sized texture" );
        FullSizedTexture = nullptr;
    }
    else
    {
        Cache->addImageAsync( LargeTextureName, [ = ]( cocos2d::Texture2D* t )
         {
             if( !t )
             {
                 // DECIDE: Should be fatal?
                 cocos2d::log( "[Card] ERROR: Failed to load full-sized texture! %s", LargeTextureName.c_str() );
                 this->FullSizedTexture = nullptr;
             }
             else
             {
                 this->FullSizedTexture = t;
             }
             
             // Let loader know this texture is done loading
             Callback();
         } );
        
        // Increment output to let the loader know how many textures need to be loaded
        Output++;
    }
    
    if( BackTextureName.empty() )
    {
        BackTextureName = "CardBack.png";
        Cache->addImageAsync( BackTextureName, [ = ] ( cocos2d::Texture2D* t )
        {
            if( !t )
            {
                // DECIDE: Should be fatal error?
                cocos2d::log( "[Card] ERROR! Failed to load default back-side texture! %s", BackTextureName.c_str() );
            }
            else
            {
                this->BackTexture = t;
            }
            
            Callback();
            
        } );
    }
    else
    {
        Cache->addImageAsync( BackTextureName, [ = ] ( cocos2d::Texture2D* t )
        {
            if( !t )
            {
                cocos2d::log( "[Card] ERROR! Failed to load specified back-side texture (%s) falling back to default", BackTextureName.c_str() );
                auto c = cocos2d::Director::getInstance()->getTextureCache();
                
                this->BackTextureName = "CardBack.png";
                c->addImageAsync( this->BackTextureName, [ = ] ( cocos2d::Texture2D* t )
                {
                    if( !t )
                    {
                        // DECIDE: Should be fatal?
                        cocos2d::log( "[Card] ERROR! Failed to load default back-side texture! %s", BackTextureName.c_str() );
                    }
                    else
                    {
                        this->BackTexture = t;
                    }
                    
                    Callback();
                } );
                
                return;
            }
            else
            {
                this->BackTexture = t;
            }
            
            Callback();
            
        } );
    }
    
    return Output;
}

bool CardEntity::ShouldCallHook() const
{
    return true;
}

bool CardEntity::ShouldCallHook( const std::string& HookName )
{
    if( !ShouldCallHook() )
        return false;
    
    // Check if this hook exists
    return ( Hooks && (*Hooks)[ HookName ].isFunction() );
}


bool CardEntity::GetHook( const std::string &HookName, luabridge::LuaRef& outFunc )
{
    if( Hooks )
    {
        if( (*Hooks)[ HookName ].isFunction() )
        {
            outFunc = (*Hooks)[ HookName ];
            return true;
        }
    }
    
    return false;
}

void CardEntity::AddToScene( cocos2d::Node* inNode )
{
    if( bSceneInit )
        return;
    
    bSceneInit = true;
    
    CC_ASSERT( inNode );
    
    // Create sprite
    // Defaults to back side visible
    Sprite = cocos2d::Sprite::createWithTexture( BackTexture );
    Sprite->setAnchorPoint( cocos2d::Vec2( 0.5f, 0.5f ) );
    Sprite->setScale( 0.7f );
    Sprite->setName( "Card" );
    
    bFaceUp = false;
    
    inNode->addChild( Sprite );
}


void CardEntity::Invalidate()
{
    EntityBase::Invalidate();
    
    if( Sprite )
    {
        Sprite->setPosition( GetAbsolutePosition() );
        Sprite->setRotation( GetAbsoluteRotation() );
    }
}


void CardEntity::Flip( bool bInFaceUp, float Time )
{
    if( bInFaceUp == bFaceUp )
        return;
    
    // Load correct texture
    auto desired = bInFaceUp ? FrontTexture : BackTexture;
    if( !desired )
        cocos2d::log( "[Card] ERROR: Failed to flip card properly.. couldnt load texture" );
    
    if( Sprite && desired )
        Sprite->runAction( cocos2d::Sequence::create( CardFlipY::Create( Time / 2.f, 0.f, 90.f ), CardFlipTex::Create( 0.f, desired ), CardFlipY::Create( Time / 2.f, -90.f, 0.f ), NULL ) );
    
    bFaceUp = bInFaceUp;
}

void CardEntity::MoveAnimation( const cocos2d::Vec2 &To, float Time )
{
    if( Sprite )
    {
        cocos2d::Vec2 FinalPosition = To;
        if( GetOwner() )
            FinalPosition += GetOwner()->GetAbsolutePosition();
        
        auto moveAction = cocos2d::MoveTo::create( Time, FinalPosition );
        moveAction->setTag( ACTION_TAG_MOVE );
        
        Sprite->stopActionByTag( ACTION_TAG_MOVE );
        Sprite->runAction( moveAction );
    }
    
    // Update entity position
    Position = To;
}

void CardEntity::RotateAnimation( float GlobalRot, float Time )
{
    if( Sprite )
    {
        auto rotAction = cocos2d::RotateTo::create( Time, GlobalRot );
        rotAction->setTag( ACTION_TAG_ROTATE );
        
        Sprite->stopActionByTag( ACTION_TAG_ROTATE );
        Sprite->runAction( rotAction );
    }
    
    // Update entity rotation
    if( GetOwner() )
        GlobalRot -= GetOwner()->GetAbsoluteRotation();
    
    Rotation = GlobalRot;
}
