//
//	AbilityText.cpp
//	Regicide Mobile
//
//	Created: 11/25/18
//	Updated: 11/25/18
//
//	© 2018 Zachary Berry, All Rights Reserved
//

#include "AbilityText.hpp"
#include "Game/World.hpp"
#include "Game/Player.hpp"
#include "Game/AuthorityBase.hpp"
#include "Game/GameContext.hpp"

AbilityText* AbilityText::Create( Game::CardEntity* InCard, Game::Ability &inAbility, float inWidth, bool bDrawSep, int inFont )
{
    auto* Output = new (std::nothrow) AbilityText();
    if( Output && Output->init( InCard, inAbility, inWidth, bDrawSep, inFont ) )
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

AbilityText::AbilityText()
: Widget(), Text( nullptr ), ManaCost( nullptr ), StaminaCost( nullptr )
{
    bCanTrigger = false;
    bDrawSeperator = false;
}

AbilityText::~AbilityText()
{
    if( Listener )
        _eventDispatcher->removeEventListener( Listener );
    
    Listener = nullptr;
}

bool AbilityText::init( Game::CardEntity* InCard, Game::Ability& In, float inWidth, bool bDrawSep, int inFont )
{
    if( !Widget::init() )
        return false;
    
    Ability = In;
    Card = InCard;
    
    bDrawSeperator = bDrawSep;
    
    // Create Draw Node
    Draw = cocos2d::DrawNode::create();
    
    addChild( Draw, 1 );
    
    // Create Mana Cost
    float TextSpacing = 0.f;
    if( Ability.ManaCost > 0 )
    {
        ManaCost = IconCount::Create( "ManaIcon.png", Ability.ManaCost, inFont + 2 );
        
        if( Ability.StaminaCost > 0 )
            ManaCost->setAnchorPoint( cocos2d::Vec2( 0.f, 0.f ) );
        else
            ManaCost->setAnchorPoint( cocos2d::Vec2( 0.f, 0.5f ) );
        
        addChild( ManaCost, 3 );
        
        TextSpacing = ManaCost->getContentSize().width;
    }
    
    if( Ability.StaminaCost > 0 )
    {
        StaminaCost = IconCount::Create( "StaminaIcon.png", Ability.StaminaCost, inFont + 2 );
        
        if( Ability.ManaCost > 0 )
            StaminaCost->setAnchorPoint( cocos2d::Vec2( 0.f, 1.f ) );
        else
            StaminaCost->setAnchorPoint( cocos2d::Vec2( 0.f, 0.5f ) );

        addChild( StaminaCost, 3 );
        
        float thisSpacing = StaminaCost->getContentSize().width;
        TextSpacing = TextSpacing < thisSpacing ? thisSpacing : TextSpacing;
    }
    
    // Create Label
    Text = cocos2d::Label::createWithTTF( Ability.Description, "fonts/arial.ttf", inFont );
    Text->enableWrap( true );
    Text->setDimensions( inWidth - 15.f - TextSpacing - 4.f, 0.f );
    Text->setAlignment( cocos2d::TextHAlignment::LEFT, cocos2d::TextVAlignment::TOP );
    Text->setTextColor( cocos2d::Color4B( 255, 255, 255, 255 ) );
    Text->setAnchorPoint( cocos2d::Vec2( 0.f, 0.5f ) );

    addChild( Text, 2 );
    
    // Check if player is able to activate this ability
    auto World = Game::World::GetWorld();
    auto Player = World ? World->GetLocalPlayer() : nullptr;
    
    if( Player && InCard )
    {
        if( Player->GetMana() < Ability.ManaCost )
        {
            if( ManaCost )
                ManaCost->SetTextColor( cocos2d::Color4B( 250, 40, 40, 255 ) );
        }
        
        if( InCard->GetState().Stamina < Ability.StaminaCost )
        {
            if( StaminaCost )
                StaminaCost->SetTextColor( cocos2d::Color4B( 250, 40, 40, 255 ) );
        }
        
        if( Player->GetMana() >= Ability.ManaCost && InCard->GetState().Stamina >= Ability.StaminaCost )
        {
            // Perform Check
            bool bCheck = true;
            if( Ability.CheckFunc && Ability.CheckFunc->isFunction() )
            {
                // Create context for Lua call
                auto Context = Game::GameContext();
                Context.SetState( InCard->GetState() );
                
                bCheck = ( *Ability.CheckFunc )( Context );
            }
            
            if( bCheck )
            {
                // Allow ability to be triggered by tap
                bCanTrigger = true;
                
                Draw->clear();
                Draw->drawRect( cocos2d::Vec2( 0.f, 0.f ), getContentSize(), cocos2d::Color4F( 0.95f, 0.1f, 0.1f, 0.8f ) );
            }
        }
    }
    
    Listener = cocos2d::EventListenerTouchOneByOne::create();
    CC_ASSERT( Listener );
    
    Listener->onTouchBegan = CC_CALLBACK_2( AbilityText::onTouch, this );
    Listener->onTouchEnded = CC_CALLBACK_2( AbilityText::onTouchEnd, this );
    Listener->setSwallowTouches( false );
    
    _eventDispatcher->addEventListenerWithFixedPriority( Listener, -90 );
    
    return true;
}

void AbilityText::onSizeChanged()
{
    Widget::onSizeChanged();
    
    // Reposition Elements
    auto Size = getContentSize();
    float TextSpacing = 0.f;
    
    if( ManaCost && StaminaCost )
    {
        ManaCost->setAnchorPoint( cocos2d::Vec2( 0.f, 0.f ) );
        StaminaCost->setAnchorPoint( cocos2d::Vec2( 0.f, 1.f ) );
        
        ManaCost->setPosition( cocos2d::Vec2( 4.f, Size.height / 2.f + 2.f ) );
        StaminaCost->setPosition( cocos2d::Vec2( 4.f, Size.height / 2.f - 2.f ) );
        
        float ManaWide = ManaCost->getContentSize().width;
        float StaminaWide = StaminaCost->getContentSize().width;
        
        TextSpacing = ManaWide > StaminaWide ? ManaWide : StaminaWide;
    }
    else
    {
        if( ManaCost )
        {
            ManaCost->setAnchorPoint( cocos2d::Vec2( 0.f, 0.5f ) );
            ManaCost->setPosition( cocos2d::Vec2( 4.f, Size.height / 2.f ) );
            
            TextSpacing = ManaCost->getContentSize().width;
        }
        if( StaminaCost )
        {
            StaminaCost->setAnchorPoint( cocos2d::Vec2( 0.f, 0.5f ) );
            StaminaCost->setPosition( cocos2d::Vec2( 4.f, Size.height / 2.f ) );
            
            TextSpacing = StaminaCost->getContentSize().width;
        }
    }
    
    if( Text )
    {
        auto TextSize = Text->getContentSize();
        Text->setContentSize( cocos2d::Size( Size.width - TextSpacing - 15.f, TextSize.height ) );
        Text->setAnchorPoint( cocos2d::Vec2( 0.f, 1.f ) );
        Text->setPosition( cocos2d::Vec2( TextSpacing + 15.f, Size.height / 2.f + TextSize.height / 2.f ) );
    }
    
    if( Draw )
    {
        Draw->clear();
        
        if( !bCanTrigger && bDrawSeperator )
        {
            Draw->drawSegment( cocos2d::Vec2( 6.f, Size.height ), cocos2d::Vec2( Size.width - 6.f, Size.height ), 1.f, cocos2d::Color4F( 0.95f, 0.95f, 0.95f, 1.f ) );
        }
        
        if( bCanTrigger )
        {
            Draw->drawRect( cocos2d::Vec2( 0.f, 0.f ), getContentSize(), cocos2d::Color4F( 0.95f, 0.1f, 0.1f, 0.8f ) );
        }
        
    }
}

float AbilityText::GetDesiredHeight()
{
    float IconHeight = 4.f;
    
    if( ManaCost )
    {
        IconHeight += ManaCost->getContentSize().height + 4.f;
    }
    if( StaminaCost )
    {
        IconHeight += StaminaCost->getContentSize().height + 4.f;
    }
    
    float TextHeight = 4.f;
    if( Text )
    {
        TextHeight += Text->getContentSize().height + 4.f;
    }
    
    return IconHeight > TextHeight ? IconHeight : TextHeight;
}

bool AbilityText::onTouch( cocos2d::Touch *inTouch, cocos2d::Event *inEvent )
{
    _touchStart = inTouch->getLocation();
    
    auto Anchor = getAnchorPoint();
    auto Size = getContentSize();
    auto Rect = cocos2d::Rect( getWorldPosition() - cocos2d::Vec2( Size.width * Anchor.x, Size.height * Anchor.y ), Size );
    if( Rect.containsPoint( inTouch->getLocation() ) )
    {
        return true;
    }
    else
    {
        return false;
    }
}

void AbilityText::onTouchEnd( cocos2d::Touch *inTouch, cocos2d::Event *inEvent )
{
    // Check if the user was just scrolling and this ability can be triggered
    if( bCanTrigger && inTouch->getLocation().distance( _touchStart ) < 9.f )
    {
        if(  Card && Ability.MainFunc && Ability.MainFunc->isFunction() )
        {
            // Perform Check Again
            if( Ability.CheckFunc && Ability.CheckFunc->isFunction() )
            {
                auto Context = Game::GameContext();
                Context.SetState( Card->GetState() );
                
                if( !( *Ability.CheckFunc )( Context ) )
                {
                    cocos2d::log( "[CardViewer] Ability Check Failed!" );
                    
                    bCanTrigger = false;
                    if( Draw )
                        Draw->clear();
                    
                    return;
                }
            }
            
            cocos2d::log( "[CardViewer] Triggering Ability" );
            
            // Good To Trigger
            auto World = Game::World::GetWorld();
            CC_ASSERT( World );
            
            auto Auth = World->GetAuthority();
            CC_ASSERT( Auth );
            
            Auth->TriggerAbility( Card, Ability.Index );
        }
    }
}
