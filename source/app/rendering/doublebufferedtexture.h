#ifndef DOUBLEBUFFEREDTEXTURE_H
#define DOUBLEBUFFEREDTEXTURE_H

#include "openglfunctions.h"

#include <array>
#include <mutex>
#include <condition_variable>

class DoubleBufferedTexture :
    public OpenGLFunctions
{
private:
    std::mutex _mutex;
    std::condition_variable _cv;
    bool _swapped = true;

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
