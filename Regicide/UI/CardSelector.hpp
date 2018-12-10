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
    
    static CardButton* Create( Game::CardEntity* In )
    {
        auto Info = In->GetInfo();
        if( !Info )
            return nullptr;
        
        auto ret = new (std::nothrow) CardButton( In );
        if( ret && ret->init( Info->FrontTexture, "", "", cocos2d::ui::Button::TextureResType::LOCAL ) )
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
    }
    
    Game::CardEntity* LinkedCard;
    bool bSelected;
    
};


