#include "WindMap.h"
#include "INIReader.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h> 
#include <float.h>
#include <math.h>
#include <assert.h>

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;
using namespace glm;

namespace windmap {
    
    string strTrim(const std::string &s) {
        std::string::const_iterator it = s.begin();
        while (it != s.end() && isspace(*it))
            it++;
        
        std::string::const_reverse_iterator rit = s.rbegin();
        while (rit.base() != it && isspace(*rit))
            rit++;
        
        return std::string(it, rit.base());
    }
    
    vector<string> strSplit(const std::string &str) {
        vector<string> arr;
        istringstream ss(str);
        string s;
        while (getline(ss, s, ',')) {
            s = strTrim(s);
            if (s.length() == 0)
                continue;
            arr.push_back(s);
        }
        return arr;
    }
    
    WindMap::WindMap(const char* inifile): _initialized(false), _vbo(0), _drawProgram(0), _updateProgram(0), _screenProgram(0),
                        _currentInd(0), _vboQuad(0), _needUpdateScreenSize(false), _screenWidth(1366), _screenHeight(3072),
                        _vboPlane(0), _mapOpacity(1.0), _counter(0)
    {
        cout << "Config file: " << inifile << endl;
        INIReader reader(inifile);
        
        string str;
        
        _option.windFile = reader.Get("general", "windFile", "");
        if(_option.windFile.length() < 1)
            return;
        
        str = reader.Get("general", "windMin", "-10, -10");
        vector<string> minStrs = strSplit(str);
        assert(minStrs.size() == 2);
        _option.windMin = glm::vec2( ::atof(minStrs[0].c_str()), ::atof(minStrs[1].c_str()) );
        
        str = reader.Get("general", "windMax", "10, 10");
        vector<string> maxStrs = strSplit(str);
        assert(maxStrs.size() == 2);
        _option.windMax = glm::vec2( ::atof(maxStrs[0].c_str()), ::atof(maxStrs[1].c_str()) );
        
        str = reader.Get("general", "windTextureSize", "2048, 2048");
        vector<string> texStrs = strSplit(str);
        assert(texStrs.size() == 2);
        _option.windTextureSize = glm::vec2( ::atof(texStrs[0].c_str()), ::atof(texStrs[1].c_str()) );

        int linear = reader.GetInteger("general", "windFilterLinear", 0);
        _option.windFilterLinear = linear != 0;

        int xz = reader.GetInteger("general", "planeXZ", 1);
        _option.planeXZ = xz != 0;
        
        str = reader.Get("general", "planeCorner", "0, 1000, 0");
        vector<string> conerStrs = strSplit(str);
        assert(conerStrs.size() == 3);
        _option.planeCorner = glm::dvec3( ::atof(conerStrs[0].c_str()), ::atof(conerStrs[1].c_str()), ::atof(conerStrs[0].c_str()) );
        
        str = reader.Get("general", "planeSize", "1, 1");
        vector<string> sizeStrs = strSplit(str);
        assert(sizeStrs.size() == 2);
        _option.planeSize = glm::dvec2( ::atof(sizeStrs[0].c_str()), ::atof(sizeStrs[1].c_str()) );
        
        _option.colorFile = reader.Get("general", "colorFile", "jet.png");
     
        _option.numParticles = reader.GetInteger("general", "numParticles", 65536);
        _option.fadeOpacity = reader.GetReal("general", "fadeOpacity", 0.996);
        _option.speedFactor = reader.GetReal("general", "speedFactor", 0.25);
        _option.dropRate = reader.GetReal("general", "dropRate", 0.003);
        _option.dropRateBump = reader.GetReal("general", "dropRateBump", 0.01);
    
        _option.loaded = true;
    }
    
    WindMap::~WindMap()
    {
        if(_vbo)
            glDeleteBuffers(1,&_vbo);
        if(_vboQuad)
            glDeleteBuffers(1,&_vboQuad);
        if(_vboPlane)
            glDeleteBuffers(1,&_vboPlane);
        if(_windTex)
            delete _windTex;
        if(_colorTex)
            delete _colorTex;
        if(_drawProgram)
            delete _drawProgram;
        if(_updateProgram)
            delete _updateProgram;
        if(_screenProgram)
            delete _screenProgram;
        for(int i=0; i<2; i++) {
            if(_particleStateFB[i])
                delete _particleStateFB[i];
            if(_mapFB[i])
                delete _mapFB[i];
        }
    }
    
    void WindMap::printOption() {
        cout << endl << "============== OPTION ==========" << endl;
        cout << _option.windFile << endl;
        cout << _option.windMin[0] << " " << _option.windMin[1] << endl;
        cout << _option.windMax[0] << " " << _option.windMax[1] << endl;
        cout << _option.windTextureSize[0] << " " << _option.windTextureSize[1] << endl;
        cout << _option.windFilterLinear << endl;
        cout << _option.planeXZ << endl;
        cout << _option.planeCorner[0] << " " << _option.planeCorner[1] << " " << _option.planeCorner[2] << endl;
        cout << _option.planeSize[0] << " " << _option.planeSize[1] << endl;
        cout << _option.colorFile << endl;
        cout << _option.numParticles << endl;
        cout << _option.fadeOpacity << endl;
        cout << _option.speedFactor << endl;
        cout << _option.dropRate << endl;
        cout << _option.dropRateBump << endl;
        cout << "============== END OPTION ==========" << endl;
    }
    
    void WindMap::setup() {
        if(_initialized)
            return;
        
        cout << "setup..." << endl;
        
        // programs
        string shaderDir = SHADER_DIR;
        cout << "Shader dir: " << shaderDir << endl;
        string filename;
        
        _drawProgram = new GLSLProgram();
        filename = shaderDir; filename.append("draw.vert");
        _drawProgram->compileShader(filename.c_str());
        filename = shaderDir; filename.append("draw.frag");
        _drawProgram->compileShader(filename.c_str());
        _drawProgram->link();
        
        _updateProgram = new GLSLProgram();
        filename = shaderDir; filename.append("quad.vert");
        _updateProgram->compileShader(filename.c_str());
        filename = shaderDir; filename.append("update.frag");
        _updateProgram->compileShader(filename.c_str());
        _updateProgram->link();
        
        _screenProgram = new GLSLProgram();
        filename = shaderDir; filename.append("quad.vert");
        _screenProgram->compileShader(filename.c_str());
        filename = shaderDir; filename.append("screen.frag");
        _screenProgram->compileShader(filename.c_str());
        _screenProgram->link();
        
        _meshProgram = new GLSLProgram();
        filename = shaderDir; filename.append("mesh.vert");
        _meshProgram->compileShader(filename.c_str());
        filename = shaderDir; filename.append("mesh.frag");
        _meshProgram->compileShader(filename.c_str());
        _meshProgram->link();
        
        // textures & framebuffers
        ColorTexture::resetUnit(0);
        _windTex = ColorTexture::newFromNextUnit(100, 100);
        _colorTex = ColorTexture::newFromNextUnit(100, 100);
        
        vector<string> names;
        names.push_back("tex0");
        
        int offset = 2;
        for(int i=0; i < 2; i++) {
            _particleStateFB[i] = new FrameBuffer(names, 100, 100);
            _particleStateFB[i]->init(offset++, false);
            
            _mapFB[i] = new FrameBuffer(names, _option.windTextureSize[0], _option.windTextureSize[1]);
            _mapFB[i]->init(offset++, _option.windFilterLinear, GL_RGBA);
        }
        
        _windTex->update(_option.windFile.c_str());
        _colorTex->update(_option.colorFile.c_str());
        
        setNumParticles(_option.numParticles);
        
        // other
        float quad[] = {0, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 1};
        glGenBuffers(1,&_vboQuad);
        glBindBuffer(GL_ARRAY_BUFFER, _vboQuad);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*12, quad, GL_STATIC_DRAW);
        
        initPlane();
        
        _initialized = true;
        
        cout << "end setup" << endl;
    }
    
    void WindMap::setScreenSize(int width, int height) {
        _screenWidth = width; _screenHeight = height;
        _needUpdateScreenSize = true;
    }
    
    void WindMap::setNumParticles(float numParticles) {
        _particleResolution = ceil(sqrt(numParticles));
        _numParticles = _particleResolution * _particleResolution;
        
        srand (time(NULL));
        float* data = new float[_numParticles*4];
        for(int i=0; i < _numParticles*4; i++) {
            data[i] = (float) rand() / RAND_MAX;
        }
        _particleStateFB[0]->update(data, _particleResolution, _particleResolution);
        _particleStateFB[1]->update(data, _particleResolution, _particleResolution);
        
        float* particleIndices = new float[_numParticles];
        for(int i=0; i < _numParticles; i++)
            particleIndices[i] = i;
        
        if(_vbo) {
            glDeleteBuffers(1,&_vbo);
            _vbo = 0;
        }
        glGenBuffers(1,&_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*_numParticles, particleIndices, GL_STATIC_DRAW);
        
        delete []data;
        delete []particleIndices;
    }
    
    void WindMap::initPlane() {
        if(_vboPlane) {
            glDeleteBuffers(1,&_vboPlane);
            _vboPlane = 0;
        }
        
        float x0 = _option.planeCorner[0];
        float height = _option.planeCorner[1];
        float z0 = _option.planeCorner[2];
        float w = _option.planeSize[0];
        float h = _option.planeSize[1];
        
        float data[] = {
            x0, height, z0, 0, 0,
            x0, height, z0+h, 0, 1,
            x0+w, height, z0+h, 1, 1,
            x0, height, z0, 0, 0,
            x0+w, height, z0+h, 1, 1,
            x0+w, height, z0, 1, 0
        };
        glGenBuffers(1,&_vboPlane);
        glBindBuffer(GL_ARRAY_BUFFER, _vboPlane);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*30, data, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    
    void WindMap::drawParticles() {
        //glDisable(GL_DEPTH_TEST);
        //glDisable(GL_STENCIL_TEST);
        
        _drawProgram->bind();
        _particleStateFB[_currentInd]->getTexture("tex0")->bind();
        _drawProgram->setUniform("u_particles", (int)_particleStateFB[_currentInd]->getTexture("tex0")->index);
        _drawProgram->setUniform("u_particles_res", (float)_particleResolution);
        
        _windTex->bind();
        _drawProgram->setUniform("u_wind", (int)_windTex->index);
        _drawProgram->setUniform("u_wind_min", _option.windMin);
        _drawProgram->setUniform("u_wind_max", _option.windMax);
        
        _colorTex->bind();
        _drawProgram->setUniform("u_color_ramp", (int)_colorTex->index);
        
        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        
        unsigned int att = glGetAttribLocation(_drawProgram->getHandle(), "a_index");
        glEnableVertexAttribArray(att);
        glVertexAttribPointer( att,  1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
        
        glDrawArrays(GL_POINTS, 0, _numParticles);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glUseProgram(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    void WindMap::drawTexture(ColorTexture* texture, float opacity) {
        _screenProgram->bind();
        texture->bind();
        _screenProgram->setUniform("u_screen", (int)texture->index);
        _screenProgram->setUniform("u_opacity", opacity);
        
        glBindBuffer(GL_ARRAY_BUFFER, _vboQuad);
        unsigned int att = glGetAttribLocation(_screenProgram->getHandle(), "a_pos");
        glEnableVertexAttribArray(att);
        glVertexAttribPointer( att,  2, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
        
        glDisable(GL_DEPTH_TEST);
        //glDisable(GL_STENCIL_TEST);
        glDisable(GL_CULL_FACE);
        glDrawArrays(GL_TRIANGLES, 0, 12);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glUseProgram(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
    void WindMap::drawScreen() {
        
        // draw previous screen to buffer to create trails
        _mapFB[_currentInd]->bind();
        glViewport(0, 0, _option.windTextureSize[0], _option.windTextureSize[1]);
        glScissor(0, 0, _option.windTextureSize[0], _option.windTextureSize[1]);
        //_mapFB[_currentInd]->clear();
        
        int prevMap = 1 - _currentInd;
        drawTexture(_mapFB[prevMap]->getTexture("tex0"),_option.fadeOpacity);
        
        // then draw particle
        drawParticles();
        
        _mapFB[_currentInd]->unbind();
    }
    
    void WindMap::drawMapPlane(ColorTexture* texture, const float MV[16], const float P[16]) {
        _meshProgram->bind();
        
        _meshProgram->setUniform("u_mv", MV);
        _meshProgram->setUniform("u_p", P);
        texture->bind();
        _meshProgram->setUniform("u_tex", (int)texture->index);
        _meshProgram->setUniform("u_opacity", _mapOpacity);
        
        glBindBuffer(GL_ARRAY_BUFFER, _vboPlane);
        unsigned int att = glGetAttribLocation(_meshProgram->getHandle(), "a_pos");
        glEnableVertexAttribArray(att);
        glVertexAttribPointer( att,  3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
        
        att = glGetAttribLocation(_meshProgram->getHandle(), "a_tex_pos");
        glEnableVertexAttribArray(att);
        glVertexAttribPointer( att,  2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));
        
        glDisable(GL_CULL_FACE);
        glDrawArrays(GL_TRIANGLES, 0, 30);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glUseProgram(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glEnableVertexAttribArray(0);
    }
    
    void WindMap::updateParticles() {
        int nextInd = 1 - _currentInd;
        _particleStateFB[nextInd]->bind();
        //_particleStateFB[nextInd]->clear();
        glViewport(0, 0, _particleResolution, _particleResolution);
        glScissor(0, 0, _particleResolution, _particleResolution);
        
        _updateProgram->bind();
        _windTex->bind();
        _updateProgram->setUniform("u_wind", (int)_windTex->index);
        _particleStateFB[_currentInd]->getTexture("tex0")->bind();
        _updateProgram->setUniform("u_particles", (int)_particleStateFB[_currentInd]->getTexture("tex0")->index);
        
        _updateProgram->setUniform("u_rand_seed", (float)rand() / RAND_MAX);
        _updateProgram->setUniform("u_wind_res", glm::vec2((float)_windTex->getWidth(), (float)_windTex->getHeight()));
        _updateProgram->setUniform("u_wind_min", _option.windMin );
        _updateProgram->setUniform("u_wind_max", _option.windMax ); 
        _updateProgram->setUniform("u_speed_factor", _option.speedFactor );
        _updateProgram->setUniform("u_drop_rate", _option.dropRate );
        _updateProgram->setUniform("u_drop_rate_bump", _option.dropRateBump );
        
        glBindBuffer(GL_ARRAY_BUFFER, _vboQuad);
        unsigned int att = glGetAttribLocation(_updateProgram->getHandle(), "a_pos");
        glEnableVertexAttribArray(att);
        glVertexAttribPointer( att,  2, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
        
        glDisable(GL_DEPTH_TEST);
        //glDisable(GL_STENCIL_TEST);
        glDisable(GL_CULL_FACE);
        glDrawArrays(GL_TRIANGLES, 0, 12);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glUseProgram(0);
        glBindTexture(GL_TEXTURE_2D, 0);
       _particleStateFB[nextInd]->unbind();
    }
    
    void WindMap::render(const float MV[16], const float P[16]) {
        
        if(!_option.loaded)
            return;
        
        setup();
        if(_needUpdateScreenSize) {
            cout << "Update screen textures" << endl;
            _needUpdateScreenSize = false;
        }
        
        glEnable(GL_POINT_SPRITE);
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
        
        /*
        drawScreen();
        updateParticles(); // for next draw call
        
        // draw to screen for testing
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, _screenWidth, _screenHeight);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glClearColor(0.3, 0.3, 0.3, 1.0);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        //drawTexture(_mapFB[_currentInd]->getTexture("tex0"));
        drawMapPlane(_mapFB[_currentInd]->getTexture("tex0"), MV, P);
        //drawMapPlane(_windTex, MV, P);
        
        glDisable(GL_BLEND);
        */
        if(_counter %2 == 0) {
            drawScreen();
            updateParticles();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, _screenWidth, _screenHeight);
        glScissor(0, 0, _screenWidth, _screenHeight);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        drawMapPlane(_mapFB[_currentInd]->getTexture("tex0"), MV, P);
        //drawMapPlane(_windTex, MV, P);

        glDisable(GL_BLEND);
        
        // update buffer index
        if(_counter %2 == 0)
            _currentInd = 1 - _currentInd;

        _counter++;
    }
    
}; //namespace windmap
