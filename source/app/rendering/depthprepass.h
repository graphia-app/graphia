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

#ifndef DEPTHPREPASS_H
#define DEPTHPREPASS_H

#include "depthpyramid.h"
#include "openglfunctions.h"

#include <QOpenGLShaderProgram>

// Nodes and traced edges drawn with their depth reduced to a pyramid
class DepthPrepass : public OpenGLFunctions
{
public:
    DepthPrepass();
    ~DepthPrepass() override;

    DepthPrepass(const DepthPrepass&) = delete;
    DepthPrepass& operator=(const DepthPrepass&) = delete;

    void resize(int width, int height);
    bool usable() const { return _complete && _depthPyramid.usable(); }

    void begin();
    void buildPyramid() { _depthPyramid.build(_depthTexture); }

    void bindPyramid(GLenum textureUnit, QOpenGLShaderProgram& shader) { _depthPyramid.bind(textureUnit, shader); }
    void releasePyramid(GLenum textureUnit) { _depthPyramid.release(textureUnit); }

    void bindHits(GLenum textureUnit);
    void releaseHits(GLenum textureUnit);

private:
    GLuint _depthTexture = 0;

    // The element nearest in each pixel, and the z it was hit at
    GLuint _hitsTexture = 0;

    GLuint _fbo = 0;
    int _width = 0;
    int _height = 0;
    bool _complete = false;
    DepthPyramid _depthPyramid;
};

#endif // DEPTHPREPASS_H
