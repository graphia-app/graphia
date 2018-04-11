#include "doublebufferedtexture.h"

DoubleBufferedTexture::DoubleBufferedTexture()
{
    resolveOpenGLFunctions();
    glGenTextures(static_cast<GLsizei>(_textures.size()), &_textures[0]);
}

DoubleBufferedTexture::~DoubleBufferedTexture()
{
    if(_textures[0] != 0)
    {
        glDeleteTextures(static_cast<GLsizei>(_textures.size()), &_textures[0]);
        _textures = {};
    }
}

GLuint DoubleBufferedTexture::front() const
{
    return _textures.at(_currentIndex);
}

GLuint DoubleBufferedTexture::back()
{
    _mutex.lock();
    return _textures.at(1 - _currentIndex);
}

GLuint DoubleBufferedTexture::swap()
{
    _currentIndex = 1 - _currentIndex;
    _mutex.unlock();

    return front();
}
