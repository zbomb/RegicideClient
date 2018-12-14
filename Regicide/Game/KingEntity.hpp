//
//    KingEntity.hpp
//    Regicide Mobile
//
//    Created: 11/19/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#pragma once

#include "EntityBase.hpp"
#include "LuaEngine.hpp"
#include "CardEntity.hpp"
#include "ObjectStates.hpp"

namespace Game
{
    // Forward Delcarations
    class Player;
    
    // Class Declaration
    class KingEntity : public EntityBase
    {
    public:
        
        KingEntity();
        ~KingEntity();
        
        virtual void AddToScene( cocos2d::Node* inNode ) override;
        void UpdateHealth( int inHealth );
        void UpdateMana( int inMana );
        
        virtual void Cleanup() override;
        
        inline std::string GetName() const { return DisplayName; }
        inline Player* GetOwningPlayer() { return OwningPlayer; }
        
        void Load( KingState& Source, bool bOpponent );
        
    protected:
        
        virtual void Invalidate() override;
        
        cocos2d::Sprite* Sprite;
        
        std::string DisplayName;
        std::string TextureName;
        cocos2d::Texture2D* Texture;
        
        cocos2d::Label* HealthLabel;
        cocos2d::Label* ManaLabel;
        Player* OwningPlayer;
        
        bool bAddedToScene;
        bool bIsOpponent;
        
        friend class SingleplayerLauncher;
    };
}
