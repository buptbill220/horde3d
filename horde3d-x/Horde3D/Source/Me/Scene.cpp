//
//  ModelSceneConf.cpp
//  Horde3d package of props model
//
//  Created by fangming.fm on 16/6/17.
//  Copyright © 2016年 fangming.fm. All rights reserved.
//

#include "Scene.h"
#include "Horde3DUtils.h"
#include <math.h>
#include <iomanip>
#include "utMath.h"
#include "egAnimation.h"
#include "egTexture.h"
#include "egModules.h"
#include "egPipeline.h"
#include <sys/time.h>

//#undef VS3D_RENDER_TEXTURE
//#undef VS3D_RENDERBUF
#undef VS3D_RAWDATA

const float MAX_ANIM_TIME = 10;

using namespace std;
using namespace Horde3D;

SceneRes pModelConf[] = {
#ifdef VS3D_RENDERBUF
    // 渲染内容绑定到纹理
    { "pipelines/model-buf.pipeline.xml", H3DResTypes::Pipeline, 0, false, MRT_FORWORD },
#else
    // 渲染内容直接显存
    { "pipelines/model.pipeline.xml", H3DResTypes::Pipeline, 0, false, MRT_FORWORD },
#endif
    //{ "pipelines/deferred.pipeline.xml", H3DResTypes::Pipeline, 0, false, MRT_DERRER },
    { "model.scene.xml", H3DResTypes::SceneGraph, 0, false, MRT_MODEL },
    { "model.anim", H3DResTypes::Animation, 0, false, MRT_OTHER },
    { "overlays/font.material.xml", H3DResTypes::Material, 0, false, MRT_FONT },
    { "overlays/panel.material.xml", H3DResTypes::Material, 0, false, MRT_PANEL },
    { "overlays/logo.material.xml", H3DResTypes::Material, 0, false, MRT_LOGO },
    { "pipelines/background.material.xml", H3DResTypes::Background, 0, false, MRT_BACKGROUND},
    // must have this node, to be the end
    { "", H3DResTypes::Undefined, 0, false, MRT_UNKNOWN }
};

static SceneRes *modelConf = pModelConf;
//const char* pmodels[] = {"models/flower", "models/flower", "models/moto", "models/car", "models/door"};
//const char* ModelName = pmodels[1];

Scene::Scene(void* projPath, const char* modelName, int width, int height)
{
    _modelName = modelName;
    _size.height = height, _size.width = width, _size.top = 0, _size.left = 0;
    _x = 15; _y = 3; _z = 20; _rx = -10; _ry = 60;
    float t[] = {-20, -12, 0, _rx ,_ry, 0, 1, 1, 1};
    setTranseform(t);
    _curFPS = 60;
    _animTime = 0.0;
    _frame = 0;
    _animWeight = 1.0;
    
    _statMode = 1;
    _freezeMode = 0; _debugViewMode = false; _wireframeMode = false;
    _frameBuffer = 0, _frameTexture = 0;
    _textData = NULL;
    _ow = 0, _oh = 0, _oc = 0;
    
    // Initialize engine
    if( !h3dInit( NULL ) )
    {
        h3dutDumpMessages();
        exit(-1);
    }
    
    // Set options
    h3dSetOption( H3DOptions::LoadTextures, 1 );
    h3dSetOption( H3DOptions::MaxAnisotropy, 4 );
    h3dSetOption( H3DOptions::ShadowMapSize, 2048 );
    h3dSetOption( H3DOptions::FastAnimation, 1 );
    
    _modelSceneConf = new ModelSceneConf(modelConf);
    if (NULL == _modelSceneConf || !_modelSceneConf->init(_modelName, projPath))
    {
        return ;
    }
    h3dSetNodeTransform(_modelSceneConf->getModel(), -10, -50, 10, 0, 0, 0, 1, 1, 1 );
    h3dSetNodeFlags(_modelSceneConf->getModel(), H3DNodeFlags::NoCastShadow, true );
    
    // Add light source
    H3DNode light = h3dAddLightNode( H3DRootNode, "Light1", 0, "LIGHTING", "SHADOWMAP" );
    h3dSetNodeTransform( light, 0, 20, 0, 0, 0, 0, 1, 1, 1 );
    h3dSetNodeParamF( light, H3DLight::RadiusF, 0, 200 );
    h3dSetNodeParamF( light, H3DLight::FovF, 0, 90 );
    h3dSetNodeParamI( light, H3DLight::ShadowMapCountI, 3 );
    h3dSetNodeParamF( light, H3DLight::ShadowSplitLambdaF, 0, 0.9f );
    h3dSetNodeParamF( light, H3DLight::ShadowMapBiasF, 0, 0.001f );
    h3dSetNodeParamF( light, H3DLight::ColorF3, 0, 0.9f );
    h3dSetNodeParamF( light, H3DLight::ColorF3, 1, 0.7f );
    h3dSetNodeParamF( light, H3DLight::ColorF3, 2, 0.75f );
    
    resize( _size.width, _size.height );
    Vec3f verts[] = {{0, 1.0, -1}, {(float)width/height, 1.0, -1}, {(float)width/height, 0, -1}, {0, 0, -1}};
    Vec3f texCoords[] = {{0, 1.0, 0}, {1.0, 1.0, 0}, {1.0, 0, 0}, {0, 0, 0}};
    //h3dShowBackground((const float*)verts, (const float*)texCoords, 4, _modelSceneConf->getBackRes());
    gettimeofday( &_lasttime, NULL );
#ifdef VS3D_RENDER_TEXTURE
#ifdef VS3D_RENDERBUF
    static PipelineResource *resObj = NULL;
    static RenderTarget* rt = NULL;
    if (rt == NULL)
    {
        h3dRender(_modelSceneConf->getCam());
        resObj = (PipelineResource*)Modules::resMan().resolveResHandle( _modelSceneConf->getForwordRes());
        rt = resObj->findRenderTarget("vs3d");
    }
    _frameBuffer = (gRDI->getRenderBuffer(rt->rendBuf)).fbo;
    _frameTexture = (gRDI->getTexture(gRDI->getRenderBufferTex(rt->rendBuf, 0))).glObj;
#endif
#endif
}


void Scene::update( uint8_t *data, int width, int height, int stride  )
{
    struct timeval now;
    gettimeofday( &now, NULL );
    float fDelta = (float)(now.tv_sec - _lasttime.tv_sec) + (now.tv_usec - _lasttime.tv_usec) / 1000000.0f;
    _lasttime = now;
    _curFPS = 1.0f / fDelta;
    
    h3dSetOption( H3DOptions::DebugViewMode, _debugViewMode ? 1.0f : 0.0f );
    h3dSetOption( H3DOptions::WireframeMode, _wireframeMode ? 1.0f : 0.0f );
    
    // Set camera parameters
    h3dSetNodeTransform(_modelSceneConf->getModel(), _transform[0], _transform[1],
                        _transform[2], _transform[3] ,_transform[4], _transform[5],
                        _transform[6], _transform[7], _transform[8] );
    h3dSetNodeTransform( _modelSceneConf->getCam(), 0, 5, 40, 0, 0, 0, 1, 1, 1 );
    // Show stats
    h3dutShowFrameStats( _modelSceneConf->getFontRes(), _modelSceneConf->getPanelRes(), _statMode );
    
    // 设置背景
    uint32 size = width * height * stride;
    // 设置背景
    //static char* d = new char[512*512*3];
    //static int realframe = 0;
    //memset(d, (realframe++) % 255, 512*512*3);
    //h3dSetBackgroundTexture("albedoMap", d, 512*512*3, 512, 512, 3, _modelSceneConf->getBackRes());
    //h3dSetBackgroundTexture("albedoMap", (char*)data, size, width, height, stride, _modelSceneConf->getBackRes());
    // 溢出考虑？正常情况不会溢出
    float frameTime = 1.0f / _curFPS;
    _animTime += frameTime;
    int frameNum = h3dGetResParamI(_modelSceneConf->getAnimRes(), AnimationResData::EntityElem, 0, AnimationResData::EntFrameCountI);
    unsigned int frame = _frame % frameNum;
    if( _statMode > 0 )
    {
        char text[256] = "";
        snprintf(text, 256, "fps: %.0f, anm time: %.1f, frame: %02d, frame num: %d", _curFPS, _animTime, frame, frameNum);
        h3dutShowText(text, 0.03f, 0.24f, 0.026f, 1, 1, 0, _modelSceneConf->getFontRes());
    }
    
    // Do animation blending
    // 动画最大时长
    static float t = _animTime;
    t = (t > MAX_ANIM_TIME) ? 0: (t + frameTime);
    if (frameNum > 1)
    {
        // 多帧才有动画
        h3dSetModelAnimParams( _modelSceneConf->getModel(), 0, t * frameNum, _animWeight );
        h3dUpdateModel(_modelSceneConf->getModel(), H3DModelUpdateFlags::Animation | H3DModelUpdateFlags::Geometry);
    }
    
    // Show logo
    const float ww = (float)h3dGetNodeParamI(_modelSceneConf->getCam(), H3DCamera::ViewportWidthI) /
    (float)h3dGetNodeParamI(_modelSceneConf->getCam(), H3DCamera::ViewportHeightI);
    const float ovLogo[] = { ww-0.4f, 0.8f, 0, 1,  ww-0.4f, 1, 0, 0,  ww, 1, 1, 0,  ww, 0.8f, 1, 1 };
    //h3dShowOverlays( ovLogo, 4, 1.f, 1.f, 1.f, 1.f, _modelSceneConf->getLogoRes(), 0 );
    
    // Render scene
    h3dRender(_modelSceneConf->getCam());
    
    // 获取渲染后的framebuffer，texture给外部用
#ifdef VS3D_RENDER_TEXTURE
#ifdef VS3D_RENDERBUF
    static PipelineResource *resObj = NULL;
    static RenderTarget* rt = NULL;
    if (rt == NULL)
    {
        h3dRender(_modelSceneConf->getCam());
        resObj = (PipelineResource*)Modules::resMan().resolveResHandle( _modelSceneConf->getForwordRes());
        rt = resObj->findRenderTarget("vs3d");
    }
    _frameBuffer = (gRDI->getRenderBuffer(rt->rendBuf)).fbo;
    _frameTexture = (gRDI->getTexture(gRDI->getRenderBufferTex(rt->rendBuf, 0))).glObj;
#endif
#ifdef VS3D_RAWDATA
    static unsigned int bufSize = 2048*1024*4;
    if (_textData == NULL)
    {
        _textData = new unsigned char[bufSize];
        memset(_textData, 0, bufSize);
    }
#ifndef VS3D_RENDERBUF
    bool ret = h3dGetRenderTargetData(0, "", 0, &_ow, &_oh, &_oc, _textData, bufSize);
#else
    bool ret = h3dGetRenderTargetData(_modelSceneConf->getForwordRes(), "vs3d", 0, &_ow, &_oh, &_oc, _textData, bufSize);
#endif
    if (!ret)
    {
        delete[] _textData;
        bufSize = _ow * _oh * _oc;
        _textData = new unsigned char[bufSize];
        memset(_textData, 0, bufSize);
#ifndef VS3D_REDERBUF
        h3dGetRenderTargetData(0, "", 0, &_ow, &_oh, &_oc, _textData, bufSize);
#else
        h3dGetRenderTargetData(_modelSceneConf->getForwordRes(), "vs3d", 0, &_ow, &_oh, &_oc, _textData, bufSize);
#endif
    }
#ifndef VS3D_RENDERBUF
    static TextureResource* s_ResMat = NULL;
    if (s_ResMat == NULL)
    {
        s_ResMat = (TextureResource*)h3dGenerateTexture("VS3DProp", _ow, _oh, _oc, 0, (const char*)_textData);
        _frameTexture = gRDI->getTextureGlobj(s_ResMat->getTexObject());
    }
    else
    {
        gRDI->updateTextureData(s_ResMat->getTexObject(), 0, 0, _textData);
    }
    _frameBuffer = h3dGetBindingFBO();
#endif
#endif
#endif
    
    h3dFinalizeFrame();
    
    h3dClearOverlays();
    h3dutDumpMessages();
    ++_frame;
}

void Scene::release()
{
    if (_modelSceneConf)
    {
        delete _modelSceneConf;
        _modelSceneConf = NULL;
    }
    if (_textData)
    {
        delete _textData;
        _textData = NULL;
    }
    h3dRelease();
}


void Scene::resize( int width, int height )
{
    _size.height = height, _size.width = width, _size.top = 0, _size.left = 0;
    // Resize viewport
    H3DNode cam = _modelSceneConf->getCam();
    h3dSetNodeParamI( cam, H3DCamera::ViewportXI, 0 );
    h3dSetNodeParamI( cam, H3DCamera::ViewportYI, 0 );
    h3dSetNodeParamI( cam, H3DCamera::ViewportWidthI, width );
    h3dSetNodeParamI( cam, H3DCamera::ViewportHeightI, height );
    
    // Set virtual camera parameters
    h3dSetupCameraView(cam, 80.0f, (float)width / height, 0.1f, 1000.0f );
    h3dResizePipelineBuffers( _modelSceneConf->getForwordRes(), width, height );
    h3dResizePipelineBuffers( _modelSceneConf->getDefferRes(), width, height );
}

void Scene::mouseMoveEvent( float dX, float dY )
{
    if( _freezeMode == 2 ) return;
    
    // Look left/right
    _ry -= dX / 100.0f * 30.0f;
    
    // Loop up/down but only in a limited range
    _rx += dY / 100.0f * 30.0f;
    //if( _rx > 90 ) _rx = 90;
    //if( _rx < -90 ) _rx = -90;
}
