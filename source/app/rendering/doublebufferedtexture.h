#ifndef DOUBLEBUFFEREDTEXTURE_H
#define DOUBLEBUFFEREDTEXTURE_H

#include "openglfunctions.h"

#include <array>
#include <mutex>

class DoubleBufferedTexture :
    public OpenGLFunctions
{
private:
    std::mutex _mutex;

    int _currentIndex = 0;
    std::array<GLuint, 2> _textures = {};

public:
    DoubleBufferedTexture();
    ~DoubleBufferedTexture() override;

    GLuint front() const;

    // If this is called more than once, it will block until a subsequent call to swap
    GLuint back();

    GLuint swap();
};

#endif // DOUBLEBUFFEREDTEXTURE_H
