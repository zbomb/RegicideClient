//
//    CardSelector.hpp
//    Regicide Mobile
//
//    Created: 11/20/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#pragma once

#include "cocos2d.h"
#include "ui/CocosGUI.h"
#include "Game/CardEntity.hpp"
#include "Game/HandEntity.hpp"
#include "AbilityText.hpp"
#include "DescriptionText.hpp"

class CardSelector : public cocos2d::Layer
{
    
public:
    
    static CardSelector* Create( Game::CardIter Begin, Game::CardIter End );
    virtual bool init();
    
    void LoadCards( Game::CardIter Begin, Game::CardIter End );
    void SetSelectCheck( std::function< bool( Game::CardEntity* ) > In ) { _fCanSelect = In; }
    void SetSelectionChanged( std::function< void( std::vector< Game::CardEntity* > ) > In ) { _fSelectionChanged = In; }
    void SetConfirm( std::function< void() > In ) { _fConfirm = In; }
    std::vector< Game::CardEntity* > GetSelection();
    void SetConfirmLabel( const std::string& In );
    
    void Lock();
    void UnLock();
    
    void Deselect( Game::CardEntity* In );
    void DeselectAll();
    
protected:
    
    std::function< bool( Game::CardEntity* ) > _fCanSelect;
    std::function< void( std::vector< Game::CardEntity* > ) > _fSelectionChanged;
    std::function< void() > _fConfirm;
    
    cocos2d::ui::ScrollView* ScrollPanel;
    cocos2d::ui::Button* Confirm;
    cocos2d::Label* ButtonLabel;
    cocos2d::DrawNode* Draw;
    
    bool _bLocked = false;
    
    void Invalidate();
    
};

class CardButton : public cocos2d::ui::Button
{
public:
    
    cocos2d::ui::ScrollView* ScrollPanel;
    std::map< int, AbilityText* > Abilities;
    Game::CardEntity* LinkedCard;
    bool bSelected;
    cocos2d::Sprite* Overlay;
    
    static CardButton* Create( Game::CardEntity* In )
    {
        auto ret = new (std::nothrow) CardButton( In );
        if( ret && ret->init( In ) )
        {
            ret->autorelease();
            return ret;
        }
        else
        {
            delete ret;
            ret = nullptr;
            return nullptr;
        }
    }
    
    CardButton( Game::CardEntity* In )
    {
        LinkedCard = In;
        bSelected = false;
        ScrollPanel = nullptr;
        Overlay     = nullptr;
    }
    
    virtual bool init( Game::CardEntity* In )
    {
        auto Info = In ? In->GetInfo() : nullptr;
        if( !Info || !Button::init( Info->FullTexture, "", "", cocos2d::ui::Button::TextureResType::LOCAL ) )
        {
            return false;
        }
        
        auto CardSize = getContentSize() * getScale();
        setCascadeOpacityEnabled( true );
        
        Overlay = cocos2d::Sprite::create( "LargeOverlay.png" );
        Overlay->setAnchorPoint( cocos2d::Vec2( 0.5f, 0.5f ) );
        Overlay->setScale( getScale() );
        Overlay->setPosition( getContentSize() * 0.5f );
        Overlay->setOpacity( 255 );
        
        // TODO: Create labels for manacost, attack and stamina
        
        // Create Scroll Panel
        ScrollPanel = cocos2d::ui::ScrollView::create();
        ScrollPanel->setBackGroundColorType( cocos2d::ui::Layout::BackGroundColorType::SOLID );
        ScrollPanel->setBackGroundColor( cocos2d::Color3B( 20, 20, 20 ) );
        ScrollPanel->setBackGroundColorOpacity( 255 );
        ScrollPanel->setAnchorPoint( cocos2d::Vec2( 0.f, 0.f ) );
        ScrollPanel->setPosition( cocos2d::Vec2( 12.f, 32.f ) );
        ScrollPanel->setContentSize( cocos2d::Size( CardSize.width - 24.f, CardSize.height * 0.4f - 32.f ) );
        ScrollPanel->setDirection( cocos2d::ui::ScrollView::Direction::VERTICAL );
        ScrollPanel->setLayoutType( cocos2d::ui::Layout::Type::VERTICAL );
        ScrollPanel->setCascadeOpacityEnabled( true );
        addChild( ScrollPanel, 1 );
        
        bool bFirst = true;
        bool bActuallyFirst = true;
        float TotalHeight = 0.f;
        
        for( auto It = Info->Abilities.begin(); It != Info->Abilities.end(); It++ )
        {
            auto Text = AbilityText::Create( In, It->second, CardSize.width * 0.85f, !bFirst, 28, false );
            Text->setCascadeOpacityEnabled( true );
            Text->setContentSize( cocos2d::Size( CardSize.width * 0.85f, Text->GetDesiredHeight() ) );
            //Text->setGlobalZOrder( 405 );
            
            // If this ability is triggerable, we dont want the next text to display a seperator
            bFirst = Text->CanTrigger();
            
            auto Layout = cocos2d::ui::LinearLayoutParameter::create();
            Layout->setGravity( cocos2d::ui::LinearLayoutParameter::LinearGravity::CENTER_HORIZONTAL );
            Layout->setMargin( cocos2d::ui::Margin( 4.f,  bActuallyFirst ? 10.f : 4.f, 4.f, 4.f ) );
            
            Text->setLayoutParameter( Layout );
            ScrollPanel->addChild( Text );
            
            TotalHeight += ( Text->getContentSize().height + ( bActuallyFirst ? 14.f : 8.f ) );
            
            Abilities[ It->first ] = Text;
            bActuallyFirst = false;
        }
        
        if( Info->Description.size() > 0 )
        {
            auto Description = DescriptionText::Create( Info->Description, CardSize.width * 0.85f, !bFirst );
            Description->setCascadeOpacityEnabled( true );
            Description->setContentSize( cocos2d::Size( CardSize.width * 0.8f, Description->GetDesiredHeight() ) );

            auto Layout = cocos2d::ui::LinearLayoutParameter::create();
            Layout->setGravity( cocos2d::ui::LinearLayoutParameter::LinearGravity::CENTER_HORIZONTAL );
            Layout->setMargin( cocos2d::ui::Margin( 4.f, 4.f, 4.f, 4.f ) );
            
            Description->setLayoutParameter( Layout );
            ScrollPanel->addChild( Description );
            
            TotalHeight += Description->getContentSize().height + 8.f;
        }
        
        ScrollPanel->setInnerContainerSize( cocos2d::Size( CardSize.width - 24.f, TotalHeight + 10.f ) );
        
        return true;
    }
    
};


