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

AbilityText* AbilityText::Create( Game::CardEntity* InCard, Game::Ability &inAbility, float inWidth )
{
    auto* Output = new (std::nothrow) AbilityText();
    if( Output && Output->init( InCard, inAbility, inWidth ) )
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
: Widget(), Text( nullptr )
{
    bCanTrigger = false;
}

bool AbilityText::init( Game::CardEntity* InCard, Game::Ability& In, float inWidth )
{
    if( !Widget::init() )
        return false;
    
    Ability = In;
    Card = InCard;
    
    // Create Draw Node
    Draw = cocos2d::DrawNode::create();
    Draw->setGlobalZOrder( 405 );
    
    addChild( Draw );
    
    // Create Label
    Text = cocos2d::Label::createWithTTF( Ability.Description, "fonts/arial.ttf", 28, cocos2d::Size( inWidth - 15.f, 0.f ) );
    Text->enableWrap( true );
    Text->setTextColor( cocos2d::Color4B( 255, 255, 255, 255 ) );
    Text->setAnchorPoint( cocos2d::Vec2( 0.5f, 1.f ) );
    Text->setGlobalZOrder( 410 );
    
    addChild( Text );
    
    // Create Mana Cost
    if( Ability.ManaCost > 0 )
    {
        ManaCost = IconCount::Create( "ManaIcon.png", Ability.ManaCost );
        ManaCost->setAnchorPoint( cocos2d::Vec2( 1.f, 1.f ) );
        ManaCost->setPosition( cocos2d::Vec2( inWidth / 2.f, 0.f ) );
        ManaCost->setGlobalZOrder( 414 );
        ManaCost->SetZ( 415 );
        
        addChild( ManaCost );
    }
    
    if( Ability.StaminaCost > 0 )
    {
        StaminaCost = IconCount::Create( "StaminaIcon.png", Ability.StaminaCost );
        StaminaCost->setAnchorPoint( cocos2d::Vec2( 0.f, 1.f ) );
        StaminaCost->setPosition( cocos2d::Vec2( inWidth / 2.f, 0.f ) );
        StaminaCost->setGlobalZOrder( 414 );
        StaminaCost->SetZ( 415 );
        
        addChild( StaminaCost );
    }
    
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
        
        if( InCard->Stamina < Ability.StaminaCost )
        {
            if( StaminaCost )
                StaminaCost->SetTextColor( cocos2d::Color4B( 250, 40, 40, 255 ) );
        }
        
        if( Player->GetMana() >= Ability.ManaCost && InCard->Stamina >= Ability.StaminaCost )
        {
            // Perform Check
            bool bCheck = true;
            if( Ability.CheckFunc && Ability.CheckFunc->isFunction() )
            {
                bCheck = ( *Ability.CheckFunc )( InCard );
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
    
    return true;
}

void AbilityText::onSizeChanged()
{
    Widget::onSizeChanged();
    
    // Reposition Text
    auto Size = getContentSize();
    if( Text )
    {
        float Height = 0.f;
        if( ManaCost )
        {
            Height = ManaCost->getContentSize().height;
            
            ManaCost->setPosition( cocos2d::Vec2( Size.width / 2.f, Size.height ) );
        }
        
        if( StaminaCost )
        {
            StaminaCost->setPosition( cocos2d::Vec2( Size.width / 2.f, Size.height ) );
        }
        
        Text->setPosition( cocos2d::Vec2( Size.width / 2.f, Size.height - Height ) );
        
    }
    
    if( Draw && bCanTrigger )
    {
        Draw->clear();
        Draw->drawRect( cocos2d::Vec2( 0.f, 0.f ), getContentSize(), cocos2d::Color4F( 0.95f, 0.1f, 0.1f, 0.8f ) );
    }
}

float AbilityText::GetDesiredHeight()
{
    float Output = 15.f;
    if( ManaCost )
    {
        Output += ManaCost->getContentSize().height + 8.f;
    }
    
    if( Text )
    {
        Output += Text->getContentSize().height;
    }
    
    return Output;
}

void AbilityText::OnTouch( cocos2d::Ref *Caller, cocos2d::ui::Widget::TouchEventType Type )
{
    cocos2d::log( "[CardViewer] Ability Touched" );
    
    if( Type == cocos2d::ui::Widget::TouchEventType::ENDED )
    {
        if( bCanTrigger && Card && Ability.MainFunc && Ability.MainFunc->isFunction() )
        {
            // Perform Check Again
            if( Ability.CheckFunc && Ability.CheckFunc->isFunction() )
            {
                if( !( *Ability.CheckFunc )( Card ) )
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
