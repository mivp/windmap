#ifndef __WINMAP_H__
#define __WINMAP_H__

#include <vector>
#include <string>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtx/rotate_vector.hpp>

using std::vector;
using std::string;

#include "Program.h"
#include "FrameBuffer.h"

namespace windmap {
    
    // options stored in an INI file
    typedef struct Option_t {
        bool loaded;
        
        string  windFile;
        glm::vec2   windMin;
        glm::vec2   windMax;
        glm::vec2   windTextureSize;
        
        bool        planeXZ;
        glm::dvec3  planeCorner;
        glm::dvec2  planeSize;
        
        string  colorFile;
        
        unsigned int    numParticles;
        float           fadeOpacity;
        float           speedFactor;
        float           dropRate;
        float           dropRateBump;
        
        Option_t(): loaded(false), fadeOpacity(0.996), speedFactor(0.25), dropRate(0.003),
                    dropRateBump(0.01), planeXZ(true), numParticles(65536)
        {}
        
    } Option;
   
    
    //=================================
    class WindMap {
        
    private:
        Option _option;
        
        bool _initialized;
        int _screenWidth, _screenHeight;
        bool _needUpdateScreenSize;
        
        unsigned int _numParticles;
        float _particleResolution;
        
        GLSLProgram* _drawProgram;
        GLSLProgram* _updateProgram;
        GLSLProgram* _screenProgram;
        GLSLProgram* _meshProgram;
        
        ColorTexture*   _windTex;
        ColorTexture*   _colorTex;
        FrameBuffer*    _particleStateFB[2];
        FrameBuffer*    _mapFB[2];
        glm::vec2       _mapSize;
        int             _currentInd;
        
        unsigned int    _vbo;
        unsigned int    _vboQuad;
        
        // map plane mesh
        unsigned int    _vboPlane;
        glm::dvec3      _planeCorner;
        glm::dvec2      _planeSize;
        float           _mapOpacity;
        
    private:
        void setNumParticles(float numParticles = 65536);
        void initPlane();
        void setup();
        void drawParticles();
        void drawScreen();
        void updateParticles();
        void drawTexture(ColorTexture* texture, float opacity = 1.0);
        void drawMapPlane(ColorTexture* texture, const float MV[16], const float P[16]);

    public:
        WindMap(const char* inifile);
        ~WindMap();
        
        void printOption();
        void setWindMapOpacity(float o) { _mapOpacity = o; }
        void setWindMin(float u_min, float v_min) { _option.windMin = glm::vec2(u_min, v_min); }
        void setWindMax(float u_max, float v_max) { _option.windMin = glm::vec2(u_max, v_max); }
        void setScreenSize(int width, int height);
        void render(const float MV[16], const float P[16]);
    };
    
}; //namespace windmap

#endif
