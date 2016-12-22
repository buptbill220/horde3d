/*
 author: fangming.fm
 date: 7/6/2016
 functional: realize background material and render
 */

#ifndef _egBackground_H_
#define _egBackground_H_

#include "egResource.h"
#include "egShader.h"
#include "egTexture.h"
#include "utMath.h"
#include <map>

namespace Horde3D {
    struct BackTexture {
        void            *data;
        uint32_t        size;
        uint32_t        stride;
        uint32_t        width;
        uint32_t        height;
        PTextureResource texRes;
        BackTexture() : data(NULL), size(0), stride(3), width(0), height(0), texRes(NULL) {}
    };

    struct BackSampler {
        std::string     name;
        std::string     mapname;
        Vec3f           *vertPos;
        Vec3f           *texCoord;
        uint32_t        vertSize;
        bool            is_new;
        BackSampler() : name("albedoMap"), mapname("back_mem"), vertPos(NULL), vertSize(4), is_new(false) {}
    };

    class BackgroundResource;
    typedef SmartResPtr< BackgroundResource > PBackgroundResource;

    class BackgroundResource : public Resource
    {
    public:
        static Resource *factoryFunc( const std::string &name, int flags )
        { return new BackgroundResource( name, flags ); }
        
        BackgroundResource( const std::string &name, int flags );
        ~BackgroundResource();
        Resource *clone();
        
        void initDefault();
        void release();
        bool load( const char *data, int size );
        
        /*
         设置背景数据texture，目前只支持rgba或者rgb类型数据，0-255
        */
        bool setMemTexture(const std::string& name, const char *texture, int size, int width, int height, int offset = 3);
        /*
         初始化时候设置
         设置顶点以及texture坐标，2者个数相等，其中texture坐标只用到x，y
         其中定点坐标：x=0~aspect，其中aspect为h3dSetupCameraView参数中的值，y=0~1
        */
        bool setVertPos(const std::string& name, const std::string& mapname, const Vec3f* verts, const Vec3f* texCoods, int size);
        
        bool isOfClass( const std::string &theClass );
        const std::string& getClass() { return _class; }
        const PShaderResource getShader() { return _shaderRes; };
        void setShader(const char*data, int size);
        bool getTexture(const std::string& name, BackTexture& texture);
    private:
        bool parseVertPos(const char* data, Vec3f* &verts, size_t& size);
        bool parseVec3f(const std::string& data, Vec3f* vert);
        bool raiseError( const std::string &msg, int line = -1 );
    private:
        PShaderResource             _shaderRes;
        uint32                      _combMask;
        std::string                 _class;
        std::vector< BackSampler >  _samplers;
        std::vector< std::string >  _shaderFlags;
        std::map<std::string, BackTexture> _mapData;
        
        friend class ResourceManager;
        friend class Renderer;
    };
    
}
#endif // _egMaterial_H_
