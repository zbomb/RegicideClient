//
//    CardEntity.cpp
//    Regicide Mobile
//
//    Created: 11/10/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#include "CardEntity.hpp"
#include "Player.hpp"
#include "CardAnimations.hpp"
#include "ICardContainer.hpp"
#include "World.hpp"
#include "GameModeBase.hpp"

using namespace Game;


CardEntity::CardEntity()
    : EntityBase( "card" )
{
    Power       = 0;
    Stamina     = 0;
    ManaCost    = 0;
    lastMoveId  = 0;
    
    _bDragging  = false;
    bSceneInit  = false;
    _bIsCard    = true;
    bAttacking  = false;
    
    Sprite              = nullptr;
    OwningPlayer        = nullptr;
    Container           = nullptr;
    FullSizedTexture    = nullptr;
    FrontTexture        = nullptr;
    BackTexture         = nullptr;
    Highlight           = nullptr;
    Overlay             = nullptr;
    StaminaLabel        = nullptr;
    PowerLabel          = nullptr;
}

CardEntity::~CardEntity()
{
    // Children nodes will be freed automatically
    if( Sprite )
        Sprite->removeFromParent();
    
    OwningPlayer        = nullptr;
    Container           = nullptr;
    FrontTexture        = nullptr;
    BackTexture         = nullptr;
    FullSizedTexture    = nullptr;
    Sprite              = nullptr;
    Overlay             = nullptr;
    Highlight           = nullptr;
    PowerLabel          = nullptr;
    StaminaLabel        = nullptr;
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

bool CardEntity::Load( luabridge::LuaRef& inLua, Player* inOwner )
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
    BackTextureName = inOwner->GetBackTexture();
    
    if( inLua[ "Description" ].isString() )
    {
        Description = inLua[ "Description" ].tostring();
    }
    else
    {
        Description = "";
    }
    
    RequireTexture( FrontTextureName, [ = ]( cocos2d::Texture2D* InTex )
    {
        if( !InTex )
        {
            cocos2d::log( "[Card] ERROR: Failed to load front texture (%s) for card '%s'", FrontTextureName.c_str(), DisplayName.c_str() );
            FrontTexture = nullptr;
        }
        else
        {
            FrontTexture = InTex;
        }
    } );
    
    RequireTexture( LargeTextureName, [ = ]( cocos2d::Texture2D* InTex )
    {
        if( !InTex )
        {
            cocos2d::log( "[Card] ERROR! Failed to load large texture (%s) for card '%s'", LargeTextureName.c_str(), DisplayName.c_str() );
            FullSizedTexture = nullptr;
        }
        else
        {
            FullSizedTexture = InTex;
        }
    } );
    
    RequireTexture( BackTextureName, [ = ]( cocos2d::Texture2D* InTex )
    {
        if( !InTex )
        {
            cocos2d::log( "[Card] ERROR! Failed to load back texture (%s) for card '%s'", BackTextureName.c_str(), DisplayName.c_str() );
            BackTexture = nullptr;
        }
        else
        {
            BackTexture = InTex;
        }
    } );
    
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
    
    // Load Abilities
    if( inLua[ "Abilities" ].isTable() )
    {
        auto AbilityTable = inLua[ "Abilities" ];
        
        int Index = 1;
        while( AbilityTable[ Index ].isTable() )
        {
            // Check if this ability is valid
            auto thisAbil = AbilityTable[ Index ];
            if( !thisAbil[ "Name" ].isString() || !thisAbil[ "Description" ].isString() ||
               !thisAbil[ "OnTrigger" ].isFunction() )
            {
                cocos2d::log( "[Card] Failed to load ability number %d for card %s", Index, DisplayName.c_str() );
                Index++;
                continue;
            }
            
            Abilities[ Index ] = Ability();
            Abilities[ Index ].Name = thisAbil[ "Name" ].tostring();
            Abilities[ Index ].Description = thisAbil[ "Description" ].tostring();
            Abilities[ Index ].MainFunc = std::make_shared< luabridge::LuaRef >( thisAbil[ "OnTrigger" ] );
            
            if( thisAbil[ "ManaCost" ].isNumber() )
                Abilities[ Index ].ManaCost = thisAbil[ "ManaCost" ];
            else
                Abilities[ Index ].ManaCost = 0;
            
            if( thisAbil[ "StaminaCost" ].isNumber() )
                Abilities[ Index ].StaminaCost = thisAbil[ "StaminaCost" ];
            else
                Abilities[ Index ].StaminaCost = 0;
            
            if( thisAbil[ "PreCheck" ].isFunction() )
                Abilities[ Index ].CheckFunc = std::make_shared< luabridge::LuaRef >( thisAbil[ "PreCheck" ] );
            
            Abilities[ Index ].Index = (uint8_t) Index;
            
            Index++;
        }
    }

    return true;
    
}

bool CardEntity::ShouldCallHook() const
{
    if( InHand() && bAllowHandHooks )
        return true;
    
    if( InDeck() && bAllowDeckHooks )
        return true;
    
    if( OnField() && bAllowPlayHooks )
        return true;
    
    if( InGrave() && bAllowDeadHooks )
        return true;
    
    return false;
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
    
    if( !BackTexture || !FrontTexture || !FullSizedTexture )
    {
        cocos2d::log( "[Card] CRITICAL! Failed to add to scene! Missing needed textures" );
        return;
    }
    
    // Create sprite
    // Defaults to back side visible
    Sprite = cocos2d::Sprite::createWithTexture( BackTexture );
    Sprite->setAnchorPoint( cocos2d::Vec2( 0.5f, 0.5f ) );
    Sprite->setScale( 0.7f );
    Sprite->setName( "Card" );
    Sprite->setCascadeOpacityEnabled( true );
    
    bFaceUp = false;
    
    inNode->addChild( Sprite );
}

void CardEntity::CreateOverlays()
{
    if( !Sprite )
    {
        cocos2d::log( "[Card] Failed to create overlays.. base sprite is null" );
        return;
    }
    
    if( Highlight )
    {
        Highlight->removeFromParent();
        Highlight = nullptr;
    }
    
    if( Overlay )
    {
        Overlay->removeFromParent();
        Overlay = nullptr;
    }
    
    if( PowerLabel )
    {
        PowerLabel->removeFromParent();
        PowerLabel = nullptr;
    }
    
    if( StaminaLabel )
    {
        StaminaLabel->removeFromParent();
        StaminaLabel = nullptr;
    }
    
    Highlight = cocos2d::Sprite::create( "CardHighlight.png" );
    Highlight->setAnchorPoint( cocos2d::Vec2( 0.5f, 0.5f ) );
    Highlight->setName( "Highlight" );
    Highlight->setPosition( Sprite->getContentSize() * 0.5f );
    Highlight->setOpacity( 0 );
    Highlight->setColor( cocos2d::Color3B( 245, 25, 25 ) );
    Highlight->setGlobalZOrder( Sprite->getGlobalZOrder() + 1 );
    
    Overlay = cocos2d::Sprite::create();
    Overlay->setAnchorPoint( cocos2d::Vec2( 0.5f, 0.5f ) );
    Overlay->setName( "Overlay" );
    Overlay->setPosition( Sprite->getContentSize() * 0.5f );
    Overlay->setOpacity( 0 );
    Overlay->setGlobalZOrder( Sprite->getGlobalZOrder() + 1 );
    
    auto Size = Sprite->getContentSize();
    
    PowerLabel = cocos2d::Label::createWithTTF( "", "fonts/arial.ttf", 65 );
    PowerLabel->setAnchorPoint( cocos2d::Vec2( 0.5f, 0.5f ) );
    PowerLabel->setTextColor( cocos2d::Color4B( 250, 250, 250, 255 ) );
    PowerLabel->setPosition( cocos2d::Vec2( Size.width - 30.f, 30.f ) );
    PowerLabel->setGlobalZOrder( Sprite->getGlobalZOrder() + 2 );
    
    StaminaLabel = cocos2d::Label::createWithTTF( "", "fonts/arial.ttf", 65 );
    StaminaLabel->setAnchorPoint( cocos2d::Vec2( 0.5f, 0.5f ) );
    StaminaLabel->setTextColor( cocos2d::Color4B( 250, 250, 250, 255 ) );
    StaminaLabel->setPosition( cocos2d::Vec2( 30.f, 30.f ) );
    StaminaLabel->setGlobalZOrder( Sprite->getGlobalZOrder() + 2 );
    
    Sprite->addChild( StaminaLabel );
    Sprite->addChild( PowerLabel );
    Sprite->addChild( Highlight );
    Sprite->addChild( Overlay );
}

void CardEntity::DestroyOverlays()
{
    if( Highlight )
    {
        Highlight->removeFromParent();
        Highlight = nullptr;
    }
    
    if( Overlay )
    {
        Overlay->removeFromParent();
        Overlay = nullptr;
    }
    
    if( PowerLabel )
    {
        PowerLabel->removeFromParent();
        PowerLabel = nullptr;
    }
    
    if( StaminaLabel )
    {
        StaminaLabel->removeFromParent();
        StaminaLabel = nullptr;
    }
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


void CardEntity::SetHighlight( const cocos2d::Color3B& inColor, uint8_t inAlpha )
{
    if( Highlight )
    {
        Highlight->setColor( inColor );
        Highlight->setOpacity( inAlpha );
    }
}

void CardEntity::ClearHighlight()
{
    if( Highlight )
    {
        Highlight->setOpacity( 0 );
    }
}

void CardEntity::SetOverlay(const std::string &TextureName, uint8_t Opacity )
{
    if( Sprite && Overlay )
    {
        Overlay->setTexture( TextureName );
        Overlay->setAnchorPoint( cocos2d::Vec2( 0.5f, 0.5f ) );
        Overlay->setPosition( Sprite->getContentSize() * 0.5f );
        Overlay->setOpacity( Opacity );
    }
}

void CardEntity::ClearOverlay()
{
    if( Overlay )
    {
        Overlay->setOpacity( 0 );
    }
}


void CardEntity::MoveAnimation( const cocos2d::Vec2 &To, float Time, std::function< void() > Callback )
{
    if( Sprite )
    {
        cocos2d::Vec2 FinalPosition = To;
        if( GetOwner() )
            FinalPosition += GetOwner()->GetAbsolutePosition();
        
        auto moveAction = cocos2d::MoveTo::create( Time, FinalPosition );
        moveAction->setTag( ACTION_TAG_MOVE );
        
        // We cant just stop the move animation, incase theres a callback associated with it
        // instead, were going to call the callback after cancelling the sequence
        Sprite->stopActionByTag( ACTION_TAG_MOVE );
        Sprite->runAction( moveAction );
        
        // Schedule Callback
        if( Callback )
            cocos2d::Director::getInstance()->getScheduler()->schedule( [=]( float Delay ) { if( Callback ) Callback(); }, this, Time + 0.1f, 0, 0.f, false, "MoveCallback" + std::to_string( ++lastMoveId ) );
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

void CardEntity::Flip( bool bInFaceUp, float Time )
{
    if( bInFaceUp == bFaceUp )
        return;
    
    // Load correct texture
    auto desired = bInFaceUp ? FrontTexture : BackTexture;
    if( !desired )
        cocos2d::log( "[Card] ERROR: Failed to flip card properly.. couldnt load texture" );
    
    ClearHighlight();
    ClearOverlay();
    
    if( Sprite && desired )
    {
        if( bInFaceUp )
        {
            Sprite->runAction( cocos2d::Sequence::create( CardFlipY::Create( Time / 2.f, 0.f, 90.f ), CardFlipTex::Create( 0.f, desired ), CardFlipY::Create( Time / 2.f, -90.f, 0.f ), cocos2d::CallFunc::create( std::bind( &CardEntity::ShowPowerStamina, this ) ), NULL ) );
        }
        else
        {
            Sprite->runAction( cocos2d::Sequence::create( CardFlipY::Create( Time / 2.f, 0.f, 90.f ), CardFlipTex::Create( 0.f, desired ), CardFlipY::Create( Time / 2.f, -90.f, 0.f ), NULL ) );
        }
    }
    
    bFaceUp = bInFaceUp;
    
}

void CardEntity::SetZ( int In )
{
    if( Sprite )
    {
        Sprite->setGlobalZOrder( In );
    }
    
    if( Overlay )
    {
        Overlay->setGlobalZOrder( In + 1 );
    }
    
    if( Highlight )
    {
        Highlight->setGlobalZOrder( In + 1 );
    }
    
    if( PowerLabel )
    {
        PowerLabel->setGlobalZOrder( In + 2 );
    }
    
    if( StaminaLabel )
    {
        StaminaLabel->setGlobalZOrder( In + 2 );
    }
}

void CardEntity::UpdatePower( int inPower )
{
    if( PowerLabel )
        PowerLabel->setString( std::to_string( inPower ) );
    
    Power = inPower;
}

void CardEntity::UpdateStamina( int inStamina )
{
    if( StaminaLabel )
        StaminaLabel->setString( std::to_string( inStamina ) );
    
    Stamina = inStamina;
}

void CardEntity::ShowPowerStamina()
{
    if( StaminaLabel )
        StaminaLabel->setString( std::to_string( Stamina ) );
    
    if( PowerLabel )
        PowerLabel->setString( std::to_string( Power ) );
}

void CardEntity::HidePowerStamina()
{
    if( StaminaLabel )
        StaminaLabel->setString( "" );
    
    if( PowerLabel )
        PowerLabel->setString( "" );
}

bool CardEntity::CanTriggerAbility( int Index )
{
    // Check Bounds
    if( Index < 0 || Abilities.count( Index ) == 0 )
        return false;
    
    // Get Ability
    auto& Abil = Abilities.at( Index );
    
    // Check Mana & Stamina
    auto Owner = GetOwningPlayer();
    if( !Owner )
        return false;
    
    if( Abil.ManaCost > Owner->GetMana() || Abil.StaminaCost > Stamina )
        return false;
    
    // Run manual check if exists
    if( Abil.CheckFunc && Abil.CheckFunc->isFunction() )
        return ( *Abil.CheckFunc )( this );
    
    return true;
}
