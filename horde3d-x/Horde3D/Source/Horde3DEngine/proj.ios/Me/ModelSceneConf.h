//
//  ModelSceneConf.h
//  ChiCagoX
//
//  Created by fangming.fm on 16/6/17.
//  Copyright © 2016年 fangming.fm. All rights reserved.
//

#ifndef ModelSceneConf_h
#define ModelSceneConf_h

#include <string>
#include "Horde3D.h"

enum MatResType
{
    MRT_UNKNOWN = 0,
    MRT_LOGO    = 1,
    MRT_PANEL   = 2,
    MRT_FONT    = 3,
    MRT_OTHER   = 4,
    MRT_FORWORD = 5,
    MRT_DERRER  = 6,
    MRT_ENV     = 7,
    MRT_MODEL   = 8,
    MRT_BACKGROUND = 9
};

struct SceneRes
{
    std::string         _resName;
    H3DResTypes::List   _type;      //horde3d 资源类型
    H3DRes              _res;       //资源句柄
    bool                _valid;     //资源是否有效
    MatResType          _mrtType;   //视觉资源类型，字体，盒子。logo
    /*
    SceneRes() : _resName(""), _type(H3DResTypes::Undefined), _res(0), _valid(false), _mrtType(MRT_UNKNOWN) {}
    SceneRes(std::string& a, H3DResTypes::List b, H3DRes c, bool d, MatResType e) : _resName(a), _type(b), _res(c), _valid(d), _mrtType(e){}
     */
};

class ModelSceneConf
{
private:
    SceneRes    *_pSceneRes;
    H3DNode     _cam;
    H3DNode     _model;
    H3DRes      _font;
    H3DRes      _panel;
    H3DRes      _logo;
    H3DRes      _forword;
    H3DRes      _deffer;
    H3DRes      _anim;
    H3DRes      _back;
    std::string _contentDir;
    std::string _modelName;
    void*       _fileHander;
    bool        _load;
    
private:
    ModelSceneConf(){}
    
public:
    ModelSceneConf(SceneRes* sceneRes);
    ModelSceneConf(ModelSceneConf& modelSceneConf);
    
public:
    /*
     modelName: earch props name, namely its dir name
     resDir:    all props were put in the resDir, default is "./Content"
     force:     used when load another props after load one
     */
    bool    init(std::string modelName, void* resDir = (void*)"Content", bool force = false);
    void    clear();
    H3DNode getCam()        { return (_cam != 0) ? _cam: 0; }
    H3DNode getModel()      { return (_model != 0) ? _model: 0; }
    H3DRes  getFontRes()    { return (_font != 0) ? _font: 0; }
    H3DRes  getPanelRes()   { return (_panel != 0) ? _panel: 0; }
    H3DRes  getLogoRes()    { return (_logo != 0) ? _logo: 0; }
    H3DRes  getForwordRes() { return (_forword != 0) ? _forword: 0; }
    H3DRes  getDefferRes()  { return (_deffer != 0) ? _deffer: 0; }
    H3DRes  getAnimRes()    { return (_anim != 0) ? _anim: 0; }
    H3DRes  getBackRes()    { return (_back != 0) ? _back: 0; }
};

#endif /* ModelSceneConf_h */