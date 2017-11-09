#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>

#ifdef STANDALONE_APP
#include "app/GLInclude.h"
#else
#include <omegaGl.h>
#endif

using namespace std;

namespace windmap {

class ColorTexture
{
public:
    //ColorTexture(const char* filename, unsigned int _format=GL_RGBA, unsigned int _globalFormat=GL_RGBA);
    ColorTexture(unsigned int _index, unsigned int _width, unsigned int _height,
                 unsigned int _format=GL_RGBA, unsigned int _globalFormat=GL_RGBA,
                 unsigned int _minFilter=GL_LINEAR, unsigned int _magFilter=GL_LINEAR);
    ~ColorTexture();

    void bind();
    void unbind();
    int getWidth() { return width; }
    int getHeight() { return height; }
    void resize(unsigned int _width, unsigned int _height);
    void update(const char* filename);
    void update(const unsigned char* buffer, unsigned int w, unsigned int h);
    void update(const float* buffer, unsigned int w, unsigned int h);
    void setFilters(unsigned int min, unsigned int mag);

    static void resetUnit(int textureUnitOffset = 0);
    static ColorTexture* newFromNextUnit(unsigned int _width, unsigned int _height, unsigned int _format=GL_RGBA, unsigned int _globalFormat=GL_RGBA);
    static unsigned int unitFromIndex(unsigned int _index);
    
    // Needs to be public to be accessed by GL calls
    unsigned int gluid;
    unsigned int glunit;
    int index;
   
    static const unsigned int LINEAR;
    static const unsigned int NEAREST;
    static const unsigned int MIPMAP;

private:
    static unsigned int unitCount;
    static float borderColor[];
    static float borderColorB[];

    unsigned int height;
    unsigned int width;
    unsigned int minFilter;
    unsigned int magFilter;
    unsigned int format;
    unsigned int globalFormat;
};

}; // namespace gigapoint

#endif // TEXTURE_H
