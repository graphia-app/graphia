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

#ifndef DEPTHPYRAMID_H
#define DEPTHPYRAMID_H

#include "openglfunctions.h"
#include "screenpass.h"

#include <QOpenGLShaderProgram>

// A hierarchical depth buffer where each mip level stores the furthest depth
// within progressively larger regions of the original depth texture
class DepthPyramid : public OpenGLFunctions
{
public:
    DepthPyramid();
    ~DepthPyramid() override;

    DepthPyramid(const DepthPyramid&) = delete;
    DepthPyramid& operator=(const DepthPyramid&) = delete;

    bool resize(int width, int height);
    bool usable() const { return _usable; }

    void build(GLuint depthTexture);

    void bind(GLenum textureUnit, QOpenGLShaderProgram& shader);
    void release(GLenum textureUnit);

private:
    int _width = 0;
    int _height = 0;
    int _numLevels = 0;
    bool _usable = false;

    GLuint _texture = 0;
    GLuint _levelFBO = 0;

    // A level can't be read from the texture while another level of it is
    // being drawn to, so each is copied out here to be read
    GLuint _previousLevelTexture = 0;
    GLuint _previousLevelFBO = 0;
    GLuint _readFBO = 0;

    QOpenGLShaderProgram _resolveShader;
    QOpenGLShaderProgram _reduceShader;
    ScreenPass _screenPass;

    void allocate(GLuint texture, int numLevels);
    void copyLevel(int level);
    void drawLevel(int level, QOpenGLShaderProgram& shader, GLuint source);
};

#endif // DEPTHPYRAMID_H
