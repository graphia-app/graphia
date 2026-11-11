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

#include "ambientocclusion.h"
#include "shadertools.h"
#include "texelbuffer.h"

#include <QString>
#include <QVector2D>

#include <algorithm>

using namespace Qt::Literals::StringLiterals;

static const int SMOOTHING_REACH = 4;

AmbientOcclusion::AmbientOcclusion()
{
    resolveOpenGLFunctions();

    glGenFramebuffers(1, &_darkenFBO);

    ShaderTools::loadShaderProgram(_occlusionShader, u":/shaders/fullscreen.vert"_s,
        u":/shaders/ambientocclusion.frag"_s);
    ShaderTools::loadShaderProgram(_smoothAcrossShader, u":/shaders/fullscreen.vert"_s,
        u":/shaders/ambientocclusion.frag"_s, {u"AO_SMOOTH_ACROSS"_s});
    ShaderTools::loadShaderProgram(_darkenShader, u":/shaders/fullscreen.vert"_s,
        u":/shaders/ambientocclusion.frag"_s, {u"AO_DARKEN"_s});

    _occlusionShader.bind();
    _occlusionShader.setUniformValue("depthTexture", 0);
    _occlusionShader.setUniformValue("elementTexture", 1);
    _occlusionShader.setUniformValue("componentData", 2);
    _occlusionShader.setUniformValue("elementComponents", 3);
    _occlusionShader.release();

    for(auto* shader : {&_smoothAcrossShader, &_darkenShader})
    {
        shader->bind();
        shader->setUniformValue("occlusion", 0);
        shader->setUniformValue("elementTexture", 1);
        shader->setUniformValue("smoothingReach", SMOOTHING_REACH);
        shader->release();
    }
}

AmbientOcclusion::~AmbientOcclusion()
{
    glDeleteFramebuffers(1, &_darkenFBO);
}

bool AmbientOcclusion::resize(int width, int height)
{
    _width = width;
    _height = height;

    return _occlusion.resize(width, height) && _smoothedAcross.resize(width, height);
}

void AmbientOcclusion::darken(const Layer& layer, TexelBuffer& componentData, float radius, float rotation,
    float logicalPixels)
{
    const float MAXIMUM_SAMPLE_RADIUS = 128.0f;

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_BLEND);

    _occlusion.bindForDrawing();

    componentData.bind(GL_TEXTURE2);
    layer._elementComponents->bind(GL_TEXTURE3);

    _occlusionShader.bind();
    _occlusionShader.setUniformValue("numNodes", layer._numNodes);
    _occlusionShader.setUniformValue("viewportSize",
        QVector2D(static_cast<float>(_width), static_cast<float>(_height)));
    _occlusionShader.setUniformValue("radius", radius);
    _occlusionShader.setUniformValue("rotation", rotation);
    const auto reachableSampleRadius = static_cast<float>(_maximumReach - 1 - SMOOTHING_REACH);
    _occlusionShader.setUniformValue("maximumPixels",
        std::min(MAXIMUM_SAMPLE_RADIUS * logicalPixels, reachableSampleRadius));
    _screenPass.draw(_occlusionShader, {layer._depthTexture, layer._elementTexture});

    layer._elementComponents->release(GL_TEXTURE3);
    componentData.release(GL_TEXTURE2);

    _smoothedAcross.bindForDrawing();
    _screenPass.draw(_smoothAcrossShader, {_occlusion.texture(), layer._elementTexture});

    // The layer's colour is premultiplied, so all of it is scaled, and its alpha kept
    glBindFramebuffer(GL_FRAMEBUFFER, _darkenFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, layer._colorTexture, 0);
    const GLenum drawBuffer = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &drawBuffer);

    glBlendFuncSeparate(GL_ZERO, GL_SRC_COLOR, GL_ZERO, GL_ONE);
    glEnable(GL_BLEND);

    _screenPass.draw(_darkenShader, {_smoothedAcross.texture(), layer._elementTexture});

    glDisable(GL_BLEND);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
}
