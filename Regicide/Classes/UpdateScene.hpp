//
//  UpdateScene.hpp
//  Regicide-mobile
//
//  Created by Zachary Berry on 11/5/18.
//

#pragma once

#include "cocos2d.h"
#include "Numeric.h"
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
