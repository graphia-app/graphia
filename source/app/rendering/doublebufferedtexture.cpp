/* Copyright © 2013-2022 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "doublebufferedtexture.h"

DoubleBufferedTexture::DoubleBufferedTexture()
{
    resolveOpenGLFunctions();
    glGenTextures(static_cast<GLsizei>(_textures.size()), _textures.data());
}

DoubleBufferedTexture::~DoubleBufferedTexture()
{
    if(_textures[0] != 0)
    {
        glDeleteTextures(static_cast<GLsizei>(_textures.size()), _textures.data());
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
    const std::unique_lock<std::mutex> lock(_mutex);
    _currentIndex = 1 - _currentIndex;

    // Notify any other users that the texture has been swapped
    _swapped = true;
    _cv.notify_all();

    return front();
}
