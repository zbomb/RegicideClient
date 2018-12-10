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


CardManager& CardManager::GetInstance()
{
    static CardManager Singleton;
    return Singleton;
}

CardManager::~CardManager()
{
    CachedCards.clear();
}

CardInfo* CardManager::GetInfoAddress( uint16_t inId )
{
    CardInfo Temp;
    if( !GetInfo( inId, Temp ) )
        return nullptr;
    
    return std::addressof( CachedCards[ inId ] );
}

bool CardManager::GetInfo( uint16_t inId, CardInfo& Out )
{
    // Check if cached
    if( CachedCards.count( inId ) > 0 )
    {
        Out = CachedCards[ inId ];
        return true;
    }
    
    // Attempt to load the lua file
    cocos2d::log( "[CardManager] Loading card info for id '%d'!", inId );
    
    auto Engine = Regicide::LuaEngine::GetInstance();
    auto L = Engine ? Engine->State() : nullptr;
    CC_ASSERT( L );
    
    auto CardTable = luabridge::newTable( L );
    luabridge::setGlobal( L, CardTable, "CARD" );
    
    if( Engine->RunScript( "cards/" + std::to_string( inId ) + ".lua" ) )
    {
        // Check Lua Table
        if( CardTable.isTable() && CardTable[ "Power" ].isNumber() && CardTable[ "Stamina" ].isNumber() && CardTable[ "Name" ].isString() && CardTable[ "Texture" ].isString() && CardTable[ "FullTexture" ].isString() && CardTable[ "Mana" ].isNumber() )
        {
            auto& newCard = CachedCards[ inId ] = CardInfo();
            newCard.DisplayName = CardTable[ "Name" ].tostring();
            newCard.Power = int( CardTable[ "Power" ] );
            newCard.Stamina = int( CardTable[ "Stamina" ] );
            newCard.ManaCost = int( CardTable[ "Mana" ] );
            newCard.FrontTexture = CardTable[ "Texture" ].tostring();
            newCard.FullTexture = CardTable[ "FullTexture" ].tostring();
            
            if( CardTable[ "Description" ].isString() )
            {
                newCard.Description = CardTable[ "Description" ].tostring();
            }
            else
            {
                newCard.Description = "";
            }
            
            if( CardTable[ "Hooks" ].isTable() )
            {
                newCard.Hooks = std::make_shared< luabridge::LuaRef >( CardTable[ "Hooks" ] );
            }
            else
            {
                newCard.Hooks.reset();
            }
            
            // Load Card Abilities
            if( CardTable[ "Abilities" ].isTable() )
            {
                int Index = 1;
                while( CardTable[ "Abilities" ][ Index ].isTable() )
                {
                    auto AbilTbl = CardTable[ "Abilities" ][ Index ];
                    
                    // Check ability validity
                    if( !AbilTbl[ "Name" ].isString() || !AbilTbl[ "Description" ].isString() || !AbilTbl[ "OnTrigger" ].isFunction() )
                    {
                        cocos2d::log( "[CardManager] Failed to load ability (%d) for card '%s' (%d)", Index, newCard.DisplayName.c_str(), newCard.Id );
                        Index++;
                        continue;
                    }
                    
                    auto& newAbility = newCard.Abilities[ (uint32_t) Index ] = Ability();
                    newAbility.Name = AbilTbl[ "Name" ].tostring();
                    newAbility.Description = AbilTbl[ "Description" ].tostring();
                    newAbility.MainFunc = std::make_shared< luabridge::LuaRef >( AbilTbl[ "OnTrigger" ] );
                    newAbility.Index = (uint8_t) Index;
                    
                    if( AbilTbl[ "ManaCost" ].isNumber() )
                        newAbility.ManaCost = uint16_t( AbilTbl[ "ManaCost" ] );
                    else
                        newAbility.ManaCost = 0;
                    
                    if( AbilTbl[ "StaminaCost" ].isNumber() )
                        newAbility.StaminaCost = uint16_t( AbilTbl[ "StaminaCost" ] );
                    else
                        newAbility.StaminaCost = 0;
                    
                    if( AbilTbl[ "PreCheck" ].isFunction() )
                        newAbility.CheckFunc = std::make_shared< luabridge::LuaRef >( AbilTbl[ "PreCheck" ] );
                    else
                        newAbility.CheckFunc.reset();
                    
                    Index++;
                }
            }
            
            // Finished loading card
            luabridge::setGlobal( L, luabridge::LuaRef( L ), "CARD" );
            
            Out = newCard;
            return true;
        }
    }
    
    luabridge::setGlobal( L, luabridge::LuaRef( L ), "CARD" );
    return false;
}

CardEntity* CardManager::CreateCard( CardState& State, Player* inOwner, bool bPreloadTextures ) 
{
    if( !inOwner )
    {
        cocos2d::log( "[CardManager] Failed to create new card.. specified player is invalid" );
        return nullptr;
    }
    
    auto Info = GetInfoAddress( State.Id );
    if( !Info )
    {
        cocos2d::log( "[CardManager] Failed to create new card.. couldnt load info" );
        return nullptr;
    }
    
    auto Output = IEntityManager::GetInstance().CreateEntity< CardEntity >( State.EntId );
    if( !Output )
    {
        cocos2d::log( "[CardManager] Failed to create new card.. entity couldnt be created" );
        return nullptr;
    }
    
    inOwner->AddChild( Output );
    
    Output->FaceUp          = false;
    Output->Id              = State.Id;
    Output->ManaCost        = State.ManaCost;
    Output->Owner           = State.Owner;
    Output->Position        = State.Position;
    Output->Power           = State.Power;
    Output->Stamina         = State.Stamina;
    Output->Info            = Info;
    Output->OwningPlayer    = inOwner;
    
    if( bPreloadTextures )
    {
        Output->RequireTexture( Info->FrontTexture, [ = ]( cocos2d::Texture2D* t )
                               {
                                   if( !t )
                                   {
                                       cocos2d::log( "[CardManager] Failed to load front texture for card '%s'", Info->DisplayName.c_str() );
                                       Output->FrontTexture = nullptr;
                                   }
                                   else
                                   {
                                       Output->FrontTexture = t;
                                   }
                               } );
        
        Output->RequireTexture( Info->FullTexture, [ = ]( cocos2d::Texture2D* t )
                               {
                                   if( !t )
                                   {
                                       cocos2d::log( "[CardManager] Failed to load full texture for card '%s'", Info->DisplayName.c_str() );
                                       Output->FullSizedTexture = nullptr;
                                   }
                                   else
                                   {
                                       Output->FullSizedTexture = t;
                                   }
                               } );
        
        Output->RequireTexture( inOwner->GetBackTexture(), [ = ]( cocos2d::Texture2D* t )
                               {
                                   if( !t )
                                   {
                                       cocos2d::log( "[CardManager] Failed to load back texture for card '%s'", Info->DisplayName.c_str() );
                                       Output->BackTexture = nullptr;
                                   }
                                   else
                                   {
                                       Output->BackTexture = t;
                                   }
                               } );
    }
    
    return Output;
}

CardEntity* CardManager::CreateCard( uint16_t inId, Player *inOwner, bool bPreloadTextures /* = false */ )
{
    if( !inOwner )
    {
        cocos2d::log( "[CardManager] Failed to create new card.. specified player in invalid" );
        return nullptr;
    }
    
    // Try to load info
    CardInfo thisInfo;
    if( GetInfo( inId, thisInfo ) )
    {
        
        auto Output = IEntityManager::GetInstance().CreateEntity< CardEntity >();
        if( !Output )
        {
            cocos2d::log( "[CardManager] Failed to create new card.. entity couldnt be allocated" );
            return nullptr;
        }
        
        inOwner->AddChild( Output );
        
        // Setup dynamic state
        Output->State.FaceUp        = false;
        Output->State.Id            = inId;
        Output->State.ManaCost      = thisInfo.ManaCost;
        Output->State.Owner         = inOwner->GetEntityId();
        Output->State.Position      = CardPos::NONE;
        Output->State.Power         = thisInfo.Power;
        Output->State.Stamina       = thisInfo.Stamina;
        Output->State.EntId         = Output->GetEntityId();
        
        // Store a reference to the 'CardInfo' so we can easily lookup static info
        // without having to perform map lookups every time
        Output->Info = std::addressof( CachedCards[ inId ] );
        
        Output->OwningPlayer = inOwner;
        
        // Setup preload textures
        if( bPreloadTextures )
        {
            Output->RequireTexture( thisInfo.FrontTexture, [ = ]( cocos2d::Texture2D* t )
                                   {
                                       if( !t )
                                       {
                                           cocos2d::log( "[CardManager] Failed to load front texture for card '%s'", thisInfo.DisplayName.c_str() );
                                           Output->FrontTexture = nullptr;
                                       }
                                       else
                                       {
                                           Output->FrontTexture = t;
                                       }
                                   } );
            
            Output->RequireTexture( thisInfo.FullTexture, [ = ]( cocos2d::Texture2D* t )
                                   {
                                       if( !t )
                                       {
                                           cocos2d::log( "[CardManager] Failed to load full texture for card '%s'", thisInfo.DisplayName.c_str() );
                                           Output->FullSizedTexture = nullptr;
                                       }
                                       else
                                       {
                                           Output->FullSizedTexture = t;
                                       }
                                   } );
            
            Output->RequireTexture( inOwner->GetBackTexture(), [ = ]( cocos2d::Texture2D* t )
                                   {
                                       if( !t )
                                       {
                                           cocos2d::log( "[CardManager] Failed to load back texture for card '%s'", thisInfo.DisplayName.c_str() );
                                           Output->BackTexture = nullptr;
                                       }
                                       else
                                       {
                                           Output->BackTexture = t;
                                       }
                                   } );
        }
        
        return Output;
    }
    else
    {
        cocos2d::log( "[CardManager] Failed to create new card.. couldnt find script for card id '%d'", inId );
        return nullptr;
    }
}




CardEntity::CardEntity()
    : EntityBase( "card" )
{
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
    
    State.FaceUp = false;
    
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
    
    Overlay = cocos2d::Sprite::create();
    Overlay->setAnchorPoint( cocos2d::Vec2( 0.5f, 0.5f ) );
    Overlay->setName( "Overlay" );
    Overlay->setPosition( Sprite->getContentSize() * 0.5f );
    Overlay->setOpacity( 0 );
    
    auto Size = Sprite->getContentSize();
    
    PowerLabel = cocos2d::Label::createWithTTF( "", "fonts/arial.ttf", 65 );
    PowerLabel->setAnchorPoint( cocos2d::Vec2( 0.5f, 0.5f ) );
    PowerLabel->setTextColor( cocos2d::Color4B( 250, 250, 250, 255 ) );
    PowerLabel->setPosition( cocos2d::Vec2( Size.width - 30.f, 30.f ) );
    
    StaminaLabel = cocos2d::Label::createWithTTF( "", "fonts/arial.ttf", 65 );
    StaminaLabel->setAnchorPoint( cocos2d::Vec2( 0.5f, 0.5f ) );
    StaminaLabel->setTextColor( cocos2d::Color4B( 250, 250, 250, 255 ) );
    StaminaLabel->setPosition( cocos2d::Vec2( 30.f, 30.f ) );
    
    Sprite->addChild( StaminaLabel, 1 );
    Sprite->addChild( PowerLabel, 1 );
    Sprite->addChild( Highlight, 3 );
    Sprite->addChild( Overlay, 2 );
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


void CardEntity::MoveAnimation( const cocos2d::Vec2 &To, float Time )
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
    if( bInFaceUp == State.FaceUp )
        return;
    
    // Load correct texture
    auto desired = bInFaceUp ? FrontTexture : BackTexture ;
    if( !desired )
    {
        cocos2d::log( "[Card] ERROR: Failed to flip card properly.. couldnt load texture" );
        return;
    }
    
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
    
    State.FaceUp = bInFaceUp;
    
}

void CardEntity::SetZ( int In )
{
    if( Sprite )
    {
        Sprite->setLocalZOrder( In );
    }
}

void CardEntity::UpdatePower( int inPower )
{
    if( PowerLabel )
        PowerLabel->setString( std::to_string( inPower ) );
    
    State.Power = inPower;
}

void CardEntity::UpdateStamina( int inStamina )
{
    if( StaminaLabel )
        StaminaLabel->setString( std::to_string( inStamina ) );
    
    State.Stamina = inStamina;
}

void CardEntity::ShowPowerStamina()
{
    if( StaminaLabel )
        StaminaLabel->setString( std::to_string( State.Stamina ) );
    
    if( PowerLabel )
        PowerLabel->setString( std::to_string( State.Power ) );
}

void CardEntity::HidePowerStamina()
{
    if( StaminaLabel )
        StaminaLabel->setString( "" );
    
    if( PowerLabel )
        PowerLabel->setString( "" );
}
