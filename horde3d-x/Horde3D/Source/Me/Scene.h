//
//  ModelSceneConf.cpp
//  Horde3d package of props model
//
//  Created by fangming.fm on 16/6/17.
//  Copyright © 2016年 fangming.fm. All rights reserved.
//

#ifndef _SCENE_H_
#define _SCENE_H_

#include <string>
#include "ModelSceneConf.h"
#include <sys/time.h>

struct SCNSize {
    int width, height;
    int left, top;
    SCNSize(): width(0), height(0), left(0), top(0) {}
};

class Scene
{
public:
    Scene(void* projPath, const char* modelName="models/boy", int width = 720, int height = 1280);
    virtual ~Scene() { release(); }
    
    void update( uint8_t *data, int width, int height, int stride );
    void setTranseform(float *t) { memcpy(_transform, t, sizeof(_transform)); }
    
    void release();
    void resize( int width, int height );
    unsigned int getFrameBuffer() { return _frameBuffer; }
    unsigned int getFrameTexture() { return _frameTexture; }
    unsigned char* getRawData() { return _textData; }
    SCNSize& getSize() { return _size; }
    unsigned long getFrameId() { return _frame; }
    
public:
    void keyHandler();
    
    void mouseMoveEvent( float dX, float dY );
    
    
public:
    ModelSceneConf  *_modelSceneConf;
    float           _pointerX, _pointerY;
    float           _x, _y, _z, _rx, _ry;  // Viewer position and orientation
    float           _curFPS;
    float           _transform[9];
    SCNSize         _size;
    
    bool            _pointerDown;
    int             _statMode;
    int             _freezeMode;
    float           _animTime;
    float           _animWeight;
    unsigned long   _frame;
    bool            _debugViewMode, _wireframeMode;
    struct timeval  _lasttime;
    unsigned int    _frameBuffer, _frameTexture;
    unsigned char   *_textData;
    int             _ow, _oh, _oc;
    const char*     _modelName;
};
#endif



