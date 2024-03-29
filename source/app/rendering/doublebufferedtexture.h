/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

    size_t _currentIndex = 0;
    std::array<GLuint, 2> _textures = {};

public:
    DoubleBufferedTexture();
    ~DoubleBufferedTexture() override;

    GLuint front() const;

    // If this is called more than once, it will block until a subsequent call to swap
    GLuint back();

    GLuint swap() noexcept;
};

#endif // DOUBLEBUFFEREDTEXTURE_H
