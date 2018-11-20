//
//    UpdateScene.hpp
//    Regicide Mobile
//
//    Created: 11/5/18
//    Updated: 11/20/18
//
//    © 2018 Zachary Berry, All Rights Reserved
//

#pragma once

#include "cocos2d.h"
#include "Numeric.hpp"
#include "ui/CocosGUI.h"

using namespace cocos2d;
using namespace cocos2d::ui;


class UpdateScene : public cocos2d::Scene
{
public:
    
    // Cocos2d Implementation
    static Scene* createScene();
    virtual bool init();
    CREATE_FUNC( UpdateScene );
    
    ~UpdateScene();
    
    void UpdateComplete( bool bSuccess, uint64 Bytes );
    void UpdateProgress( uint64 Downloaded, uint64 Total, std::string BlockName );
    
private:
    
    Label* Header;
    LoadingBar* Progress;
    Label* CurrentFile;
    
    void Internal_Exit( float Delay, bool bSuccess, uint64 Bytes );
};
