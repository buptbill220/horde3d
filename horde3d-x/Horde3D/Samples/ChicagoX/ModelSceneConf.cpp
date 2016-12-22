//
//  ModelSceneConf.cpp
//  Horde3d package of props model
//
//  Created by fangming.fm on 16/6/17.
//  Copyright © 2016年 fangming.fm. All rights reserved.
//

#include "ModelSceneConf.h"
#include "Horde3DUtils.h"
#include "egBackground.h"
#include "egModules.h"

ModelSceneConf::ModelSceneConf(SceneRes* sceneRes)
{
    this->_pSceneRes = sceneRes;
    this->_load = false;
    this->_model = 0;
    this->_cam = 0;
    this->_font = 0;
    this->_panel = 0;
    this->_logo = 0;
    this->_forword = 0;
    this->_deffer = 0;
    this->_anim = 0;
    this->_back = 0;
    this->_fileHander = NULL;
}

ModelSceneConf::ModelSceneConf(ModelSceneConf& modelSceneConf)
{
    this->_pSceneRes = modelSceneConf._pSceneRes;
    this->_load = modelSceneConf._load;
    this->_model = modelSceneConf._model;
    this->_cam = modelSceneConf._cam;
    this->_font = modelSceneConf._font;
    this->_panel = modelSceneConf._panel;
    this->_logo = modelSceneConf._logo;
    this->_forword = modelSceneConf._forword;
    this->_deffer = modelSceneConf._deffer;
    this->_anim = modelSceneConf._anim;
    this->_back = modelSceneConf._back;
    this->_fileHander = modelSceneConf._fileHander;
}

bool ModelSceneConf::init(std::string modelName, void* resDir, bool force)
{
    if (force || !_load || this->_modelName != modelName || this->_fileHander != resDir)
    {
        _fileHander = resDir;
        // first add resurce
        if (NULL == _pSceneRes)
        {
            return false;
        }
        SceneRes *back;
        SceneRes *p = _pSceneRes;
        while (p->_resName != "")
        {
            H3DRes res = h3dAddResource(p->_type, p->_resName.c_str(), 0);
            if (0 == res)
            {
                return false;
            }
            p->_res = res;
            p->_valid = true;
            if (p->_mrtType == MRT_BACKGROUND) {
                back = p;
            }
            ++p;
        }
        // second load resource, add node
#ifdef HORDE3D_D3D11
        const char* platformDir = "d3d11";
#elif HORDE3D_GL
        const char* platformDir = "gl";
#elif HORDE3D_GLES2
        const char* platformDir = "gles2";
#endif
        h3dutLoadResourcesFromDisk(resDir, modelName.c_str(), platformDir);
        p = _pSceneRes;
        while (p->_resName != "")
        {
            switch (p->_type)
            {
                case  H3DResTypes::Animation:
                    _anim = p->_res;
                    break;
                default:
                    break;
            }
            switch (p->_mrtType)
            {
                case MRT_MODEL:
                    _model = h3dAddNodes(H3DRootNode, p->_res);
                    break;
                case MRT_FONT:
                    _font = p->_res;
                    break;
                case MRT_LOGO:
                    _logo = p->_res;
                    break;
                case MRT_PANEL:
                    _panel = p->_res;
                    break;
                case MRT_DERRER:
                    _deffer = p->_res;
                    break;
                case MRT_FORWORD:
                    _forword = p->_res;
                    _cam = h3dAddCameraNode(H3DRootNode, "Camera", p->_res);
                    break;
                case MRT_BACKGROUND:
                    _back = p->_res;
                    break;
                default:
                    break;
            }
            ++p;
        }
        
        if (0 != _anim)
        {
            h3dSetupModelAnimStage(_model, 0, _anim, 0, "", false);
        }
        _load = true;
        return true;
    }
    return true;
}

void ModelSceneConf::clear()
{
    _pSceneRes = NULL;
    _model = 0;
    _cam = 0;
    _font = 0;
    _panel = 0;
    _logo = 0;
    _forword = 0;
    _deffer = 0;
    _back = 0;
    _contentDir = "Content";
    _modelName = "";
    h3dClear();
}

