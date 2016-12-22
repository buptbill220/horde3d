#include "egModules.h"
#include "egCom.h"
#include "utXML.h"
#include "egBackground.h"


namespace Horde3D {
    
    using namespace std;
    // 标准背景顶点坐标
    static Vec3f S_Back_VertPos[4] = {{.0f, .0f, -1.0f}, {.0f, 1.0f, -1.0f}, {1.0f, 1.0f, -1.0f}, {1.0f, .0f, -1.0f}};
    static Vec3f S_Back_TextPos[4] = {{.0f, .0f, .0f}, {1.0f, .0f, .0f}, {1.0f, 1.0f, -1.0f}, {.0f, 1.0f, .0f}};
    
    BackgroundResource::BackgroundResource( const string &name, int flags ) :
        Resource( ResourceTypes::Background, name, flags )
    {
        initDefault();
    }
    
    BackgroundResource::~BackgroundResource()
    {
        release();
    }
    
    Resource *BackgroundResource::clone()
    {
        BackgroundResource *res = new BackgroundResource( "", _flags );
        *res = *this;
        return res;
    }
    
    void BackgroundResource::initDefault()
    {
        _shaderRes = 0x0;
        _combMask = 0;
        _class = "";
    }
    
    void BackgroundResource::release()
    {
        _shaderRes = 0x0;
       
        for (size_t i = 0; i < _samplers.size(); ++i)
        {
            delete []_samplers[i].vertPos;
            _samplers[i].vertPos = NULL;
        }
        _samplers.clear();
        _shaderFlags.clear();
        _mapData.clear();
    }
   
    bool BackgroundResource::raiseError( const string &msg, int line )
    {
        // Reset
        release();
        initDefault();
        if ( line < 0 )
        {
            Modules::log().writeError( "Material resource '%s': %s", _name.c_str(), msg.c_str() );
        }
        else
        {
            Modules::log().writeError( "Material resource '%s' in line %i: %s", _name.c_str(), line, msg.c_str() );
        }                   
        return false;
    }

    bool BackgroundResource::load( const char *data, int size )
    {
        if( !Resource::load( data, size ) ) return false;
        
        XMLDoc doc;
        doc.parseBuffer( data, size );
        if( doc.hasError() )
            return raiseError( "XML parsing error" );
        
        XMLNode rootNode = doc.getRootNode();
        if( strcmp( rootNode.getName(), "Material" ) != 0 )
            return raiseError( "Not a material resource file" );
        
        // Class
        _class = rootNode.getAttribute( "class", "" );
        
        // Shader Flags
        XMLNode node1 = rootNode.getFirstChild( "ShaderFlag" );
        while( !node1.isEmpty() )
        {
            if( node1.getAttribute( "name" ) == 0x0 ) return raiseError( "Missing ShaderFlag attribute 'name'" );
            _shaderFlags.push_back( node1.getAttribute( "name" ) );
            node1 = node1.getNextSibling( "ShaderFlag" );
        }
        
        // Shader
        node1 = rootNode.getFirstChild( "Shader" );
        if( !node1.isEmpty() )
        {
            if( node1.getAttribute( "source" ) == 0x0 ) return raiseError( "Missing Shader attribute 'source'" );
            uint32 shader = Modules::resMan().addResource(
                                                          ResourceTypes::Shader, node1.getAttribute( "source" ), 0, false );
            _shaderRes = (ShaderResource *)Modules::resMan().resolveResHandle( shader );
            
            _combMask = ShaderResource::calcCombMask( _shaderFlags );
            _shaderRes->preLoadCombination( _combMask );
        }
        
        // Texture samplers
        node1 = rootNode.getFirstChild( "Sampler" );
        while( !node1.isEmpty() )
        {
            if( node1.getAttribute( "name" ) == 0x0 ) return raiseError( "Missing Sampler attribute 'name'" );
            if( node1.getAttribute( "map" ) == 0x0 ) return raiseError( "Missing Sampler attribute 'map'" );
            
            BackSampler sampler;
            //"<Sampler name="ambientMap" map="back_mem" vertPos="[[0,0,1,1],[1,0,1,1],[1,1,1,1],[0,1,1,1]]"/>"
            sampler.name = node1.getAttribute("name");
            sampler.mapname = node1.getAttribute("map");
            if (node1.getAttribute("vertPos") != 0x0)
            {
                // [[0,0,1,1],[1,0,1,1],[1,1,1,1],[0,1,1,1]]
                const char* vertPos = node1.getAttribute("vertPos");
                Vec3f* verts = NULL;
                size_t size = 0;
                if (!parseVertPos(vertPos, verts, size))
                {
                    return raiseError("load Background data, parse vertPos failed");
                }
                sampler.vertPos = verts;
                sampler.vertSize = size;
                sampler.is_new = true;
            }
            else
            {
                sampler.vertPos = S_Back_VertPos;
                sampler.vertSize = 4;
                sampler.is_new = false;
            }
            if (node1.getAttribute("texCoords") != 0x0)
            {
                const char* texCoords = node1.getAttribute("texCoords");
                Vec3f* verts = NULL;
                size_t size = 0;
                if (!parseVertPos(texCoords, verts, size))
                {
                    sampler.texCoord = verts;
                }
                else
                {
                    sampler.texCoord = S_Back_TextPos;
                }
            }
            _samplers.push_back( sampler );
            node1 = node1.getNextSibling( "Sampler" );
        }
        
        return true;
    }
   
    bool BackgroundResource::parseVertPos(const char* data, Vec3f* &verts, size_t& size)
    {
        // [[1,1,1,1],[1,1,1,1]]
        static uint16_t head = *(uint16_t*)"[[";
        static uint16_t tail = *(uint16_t*)"]]";

        string str = data;
        if (str.size() > 4 && *(uint16_t*)data == head && *(uint16_t*)(data + str.size() - 2) == tail)
        {
            size_t pos = 0, start = 2, end = str.size() - 3;
            vector<size_t> filtPos;
            while (start < end)
            {
                pos = str.find("],[", start);
                if (pos == string::npos)
                {
                    break;
                }
                filtPos.push_back(pos - 1);
                start = pos + 3;
            }
            start = 2;
            verts = new Vec3f[filtPos.size() + 1];
            if (verts == NULL)
            {
                return raiseError("new Vec4f failed in parseVertPos");
            }
            size_t idx = 0;
            string sub;
            for (size_t i = 0; i < filtPos.size(); ++i)
            {
                sub = str.substr(start, filtPos[i] - start + 1);
                if (!parseVec3f(sub, verts + (idx++)))
                {
                    delete []verts;
                    verts = NULL;
                    return raiseError("parseVertPos [" + str + "] failed");
                }
                start = filtPos[i] + 4;
            }
            sub = str.substr(start, end - start + 1);
            if (!parseVec3f(sub, verts + (idx++)))
            {
                delete []verts;
                verts = NULL;
                return raiseError("parseVertPos [" + str + "] failed");
            }
            size = idx;

            return true;
        }
        return false;
    }

    bool BackgroundResource::parseVec3f(const std::string& data, Vec3f* vert)
    {
        // 1,1,1,1
        size_t pos = 0, start = 0, end = data.size() - 1;
        unsigned int idx = 0;
        while (start < end)
        {
            pos = data.find(",", start);
            if (pos == string::npos)
            {
                break;
            }
            string sub = data.substr(start, pos - start);
            (*vert)[idx++] = atof(sub.c_str());
            start = pos + 1;
        }
        string sub = data.substr(start, end - start + 1);
        (*vert)[idx] = atof(sub.c_str());
        return true;
    }

    bool BackgroundResource::setVertPos(const string& name, const string& mapname, const Vec3f* verts, const Vec3f* texCoods, int size)
    {
        for (size_t i = 0; i < _samplers.size(); ++i)
        {
            if (_samplers[i].name == name && _samplers[i].mapname == mapname)
            {
                if (_samplers[i].is_new)
                {
                    delete _samplers[i].vertPos;
                    _samplers[i].vertPos = NULL;
                }
                _samplers[i].vertPos = (Vec3f*)verts;
                _samplers[i].texCoord = (Vec3f*)texCoods;
                _samplers[i].vertSize = size;
                _samplers[i].is_new = false;
                return true;
            }
        }
        return false;
    }

    bool BackgroundResource::getTexture(const std::string &name, Horde3D::BackTexture &texture)
    {
        map<string, BackTexture>::iterator it = _mapData.find(name);
        if (it == _mapData.end())   return false;
        texture = (it->second);
        return true;
    }
    
    bool BackgroundResource::setMemTexture(const string& name, const char* texture, int size, int width, int height, int stride)
    {
        map<string, BackTexture>::iterator it = _mapData.find(name);
        if (it != _mapData.end())
        {
            //效率高一点
            /*
            if (it->second.size == size && it->second.width == width && it->second.height == height && it->second.stride == stride)
            {
                gRDI->updateTextureData(it->second.texRes->getTexObject(), 0, 0, texture);
            }
            else
             */
            {
                it->second.texRes->release();
                if (!it->second.texRes->load(texture, size, width, height, stride))
                {
                    raiseError("load memory texture: " + name + " failed");
                    _mapData.erase(it);
                    return false;
                }
                it->second.size = size;
                it->second.stride = stride;
                it->second.width = width;
                it->second.height = height;
            }
            it->second.data = (void*)texture;
            return true;
        }
        BackTexture backtexture;
        backtexture.data = (void*)texture;
        backtexture.size = size;
        backtexture.width = width;
        backtexture.height = height;
        backtexture.stride = stride;
        
        ResHandle texMap;
        uint32 flags = 0;
        if( !Modules::config().loadTextures ) flags |= ResourceFlags::NoQuery;
        flags |= ResourceFlags::TexSRGB;

        texMap = Modules::resMan().addResource(ResourceTypes::Texture, name, flags, false);
        PTextureResource texRes = (TextureResource *)Modules::resMan().resolveResHandle( texMap );
        if (!texRes->load(texture, size, width, height, stride))
        {
            return raiseError("load memory texture: " + name + " failed");
        }
        backtexture.texRes = texRes;

        _mapData[name] = backtexture;
        return true;
    }

    bool BackgroundResource::isOfClass( const string &theClass )
    {
        static string theClass2;
        
        if( theClass != "" )
        {
            if( theClass[0]	!= '~' )
            {
                if( _class.find( theClass, 0 ) != 0 ) return false;
                if( _class.length() > theClass.length() && _class[theClass.length()] != '.' ) return false;
            }
            else	// Not operator
            {
                theClass2 = theClass.substr( 1, theClass.length() - 1);
                
                if( _class.find( theClass2, 0 ) == 0 )
                {
                    if( _class.length() == theClass2.length() )
                    {
                        return false;
                    }
                    else
                    {
                        if( _class[theClass2.length()] == '.' ) return false;
                    }
                }
            }
        }
        else
        {
            // Special name which is hidden when drawing objects of "all classes"
            if( _class == "_DEBUG_" ) return false;
        }
        
        return true;
    }
    
    void BackgroundResource::setShader(const char* data, int size)
    {

        uint32 shader = Modules::resMan().addResource(ResourceTypes::Shader, "mem_back", 0, false );
        _shaderRes = (ShaderResource *)Modules::resMan().resolveResHandle( shader );
        _shaderRes->release();
        _shaderRes->load(data, size);
        _combMask = ShaderResource::calcCombMask( _shaderFlags );
        _shaderRes->preLoadCombination( _combMask );
    }
}  // namespace
