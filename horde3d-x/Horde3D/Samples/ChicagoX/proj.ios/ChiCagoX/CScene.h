//
//  ModelSceneConf.cpp
//  Horde3d package of props model
//
//  Created by fangming.fm on 16/6/17.
//  Copyright © 2016年 fangming.fm. All rights reserved.
//

#ifndef _CSCENE_H_
#define _CSCENE_H_

#include "Scene.h"
#include "CCEvent.h"
#include "nodes/CCNode.h"


class CScene: public cocos2d::CCNode
{
public:
    CScene(void* projPath, const char* modelName="models/boy", int width = 720, int height = 1280);
    virtual ~CScene() {m_scene->release();}
private:
    bool onEvent( const cocos2d::CCEvent& e  );
private:
    Scene *m_scene;
};

#endif



