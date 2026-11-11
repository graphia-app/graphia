/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#ifndef SCREENTEXTURE_H
#define SCREENTEXTURE_H

#include "openglfunctions.h"

// A texture the size of the view, and the framebuffer that draws into it
class ScreenTexture : public OpenGLFunctions
{
public:
    ScreenTexture(GLint internalFormat, GLenum format, GLenum type, GLint filter);
    ~ScreenTexture() override;

    ScreenTexture(const ScreenTexture&) = delete;
    ScreenTexture& operator=(const ScreenTexture&) = delete;

    bool resize(int width, int height);

    GLuint texture() const { return _texture; }

    void bindForDrawing();

    // Leaves framebuffer bound
    void copyTo(GLuint framebuffer);

private:
    GLint _internalFormat = 0;
    GLenum _format = 0;
    GLenum _type = 0;
    GLint _filter = 0;

    int _width = 0;
    int _height = 0;

    GLuint _texture = 0;
    GLuint _framebuffer = 0;
};

#endif // SCREENTEXTURE_H
