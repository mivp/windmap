#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include <list>
#include <string>
#include <vector>
#include "ColorTexture.h"
using std::string;
using std::vector;

using namespace std;

namespace windmap {

class FrameBuffer
{
public:
    FrameBuffer(bool storeDepth = false);
    FrameBuffer(vector<string> textures, unsigned int _width = RT_SIZE_DEFAULT, unsigned int _height = RT_SIZE_DEFAULT,
                bool storeDepth = false);
    ~FrameBuffer();

    void clear();
    void init(int textureUnitOffset = 0, bool linearFilter = false);
    void bind();
    void unbind();
    void resize(unsigned int _width, unsigned int _height);
    void update(const float* _buffer, unsigned int _width, unsigned int _height);
    void resizeViewport();

    unsigned int getAttachementFromIndex(unsigned int index);

    ColorTexture *getTexture(string name);
    unsigned int getWidth();
    unsigned int getHeight();

    static const int RT_SIZE_DEFAULT;

protected:
    unsigned int uid;

    unsigned int width;
    unsigned int height;

    bool hasDepth;
    unsigned int depthBuffer;

    vector<string> texturesNames;
    vector<ColorTexture*> textures;

    unsigned int size;
    unsigned int* drawBuffers;
};

}; // namespace gigapoint

#endif // FRAMEBUFFER_H
