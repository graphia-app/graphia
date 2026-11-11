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

#ifndef AMBIENTOCCLUSION_H
#define AMBIENTOCCLUSION_H

#include "openglfunctions.h"
#include "screenpass.h"
#include "screentexture.h"

#include <QOpenGLShaderProgram>

#include <limits>

class TexelBuffer;

class AmbientOcclusion : public OpenGLFunctions
{
public:
    AmbientOcclusion();
    ~AmbientOcclusion() override;

    AmbientOcclusion(const AmbientOcclusion&) = delete;
    AmbientOcclusion& operator=(const AmbientOcclusion&) = delete;

    bool resize(int width, int height);
    void setMaximumReach(int pixels) { _maximumReach = pixels; }

    struct Layer
    {
        GLuint _colorTexture = 0;
        GLuint _elementTexture = 0;
        GLuint _depthTexture = 0;
        TexelBuffer* _elementComponents = nullptr;
        int _numNodes = 0;
    };

    void darken(const Layer& layer, TexelBuffer& componentData, float radius, float rotation,
        float logicalPixels);

private:
    int _width = 0;
    int _height = 0;

    int _maximumReach = std::numeric_limits<int>::max();

    ScreenTexture _occlusion{GL_R8, GL_RED, GL_UNSIGNED_BYTE, GL_NEAREST};
    ScreenTexture _smoothedAcross{GL_R8, GL_RED, GL_UNSIGNED_BYTE, GL_NEAREST};

    GLuint _darkenFBO = 0;

    QOpenGLShaderProgram _occlusionShader;
    QOpenGLShaderProgram _smoothAcrossShader;
    QOpenGLShaderProgram _darkenShader;
    ScreenPass _screenPass;
};

#endif // AMBIENTOCCLUSION_H
