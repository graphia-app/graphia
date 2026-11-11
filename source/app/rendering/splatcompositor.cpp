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

#include "splatcompositor.h"
#include "shadertools.h"

#include <QString>

using namespace Qt::Literals::StringLiterals;

SplatCompositor::SplatCompositor()
{
    resolveOpenGLFunctions();

    glGenTextures(1, &_sums);
    glGenTextures(1, &_weights);
    glGenTextures(1, &_overlap);
    glGenFramebuffers(1, &_framebuffer);

    ShaderTools::loadShaderProgram(_resolveShader, u":/shaders/fullscreen.vert"_s, u":/shaders/coverage.frag"_s);
    _resolveShader.bind();
    _resolveShader.setUniformValue("sums", 0);
    _resolveShader.setUniformValue("weights", 1);
    _resolveShader.setUniformValue("overlap", 2);
    _resolveShader.release();
}

SplatCompositor::~SplatCompositor()
{
    glDeleteFramebuffers(1, &_framebuffer);
    glDeleteTextures(1, &_overlap);
    glDeleteTextures(1, &_weights);
    glDeleteTextures(1, &_sums);
}

bool SplatCompositor::resize(int width, int height, GLuint depthTexture)
{
    _width = width;
    _height = height;

    auto resizeTexture = [&](GLuint texture, GLint internalFormat, GLenum format)
    {
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_HALF_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    };

    resizeTexture(_sums, GL_RGBA16F, GL_RGBA);
    resizeTexture(_weights, GL_RGBA16F, GL_RGBA);
    resizeTexture(_overlap, GL_RGBA16F, GL_RGBA);

    glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _sums, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _weights, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, _overlap, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    const bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return complete;
}

void SplatCompositor::bindFramebuffer(GLuint framebuffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glViewport(0, 0, _width, _height);
}

void SplatCompositor::begin()
{
    bindFramebuffer(_framebuffer);
    glEnable(GL_DEPTH_TEST);

    const GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, static_cast<const GLenum*>(drawBuffers));

    const GLfloat none[] = {0.0f, 0.0f, 0.0f, 0.0f};

    for(GLint drawBuffer = 0; drawBuffer < 3; drawBuffer++)
        glClearBufferfv(GL_COLOR, drawBuffer, static_cast<const GLfloat*>(none));

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDepthMask(GL_FALSE);
}

void SplatCompositor::resolveInto(GLuint layerFramebuffer)
{
    glDepthMask(GL_TRUE);
    glDisable(GL_DEPTH_TEST);

    // The layer's colour and selection; element numbers can't be blended
    bindFramebuffer(layerFramebuffer);
    const GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_NONE, GL_NONE, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, static_cast<const GLenum*>(drawBuffers));
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    _screenPass.draw(_resolveShader, {_sums, _weights, _overlap});

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}
