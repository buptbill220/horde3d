//
//  ModelSceneConf.cpp
//  Horde3d package of props model
//
//  Created by fangming.fm on 16/6/17.
//  Copyright © 2016年 fangming.fm. All rights reserved.
//

#include "CScene.h"

using namespace std;
using namespace cocos2d;

CScene::CScene(void* projPath, const char* modelName, int width, int height) : CCNode()
{
    m_scene = new Scene(projPath, modelName, width, height);
}

 bool CScene::onEvent( const CCEvent& e  )
 {
	switch(e.Type)
	{
	case CCEvent::Quit:
        m_scene->release();
        return true;
	case CCEvent::TouchBegan:
        m_scene->_pointerDown = true;
        m_scene->_pointerX = e.Touch.X; m_scene->_pointerY = e.Touch.Y;
        break;
	case CCEvent::TouchEnded:
	case CCEvent::TouchCancelled:
        m_scene->_pointerDown = false;
        break;
	case CCEvent::TouchMoved:
        if (m_scene->_pointerDown)
        {
            m_scene->mouseMoveEvent( e.Touch.X - m_scene->_pointerX, e.Touch.Y - m_scene->_pointerY);
            m_scene->_pointerX = e.Touch.X; m_scene->_pointerY = e.Touch.Y;
        }
        break;
	}
 
	return false;
 }