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
    std::unique_lock<std::mutex> lock(_mutex);

    // Wait until any other users have swapped the texture
    _cv.wait(lock, [this] { return _swapped; });
    _swapped = false;

    return _textures.at(1 - _currentIndex);
}

GLuint DoubleBufferedTexture::swap()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _currentIndex = 1 - _currentIndex;

    // Notify any other users that the texture has been swapped
    _swapped = true;
    _cv.notify_all();

    return front();
}
