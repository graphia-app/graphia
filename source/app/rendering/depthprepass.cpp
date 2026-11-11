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

#include "depthprepass.h"

#include <array>

DepthPrepass::DepthPrepass()
{
    resolveOpenGLFunctions();

    glGenTextures(1, &_depthTexture);
    glGenTextures(1, &_hitsTexture);
    glGenFramebuffers(1, &_fbo);
}

DepthPrepass::~DepthPrepass()
{
    glDeleteFramebuffers(1, &_fbo);
    glDeleteTextures(1, &_hitsTexture);
    glDeleteTextures(1, &_depthTexture);
}

void DepthPrepass::resize(int width, int height)
{
    _width = width;
    _height = height;

    auto allocate = [&](GLuint texture, GLint internalFormat, GLenum format)
    {
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);
    };

    allocate(_depthTexture, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT);
    allocate(_hitsTexture, GL_RG32F, GL_RG);

    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _hitsTexture, 0);

    // The prepass shaders write the element they hit to their second output, as the layer shaders do
    const std::array<GLenum, 2> drawBuffers{{GL_NONE, GL_COLOR_ATTACHMENT1}};
    glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
    glReadBuffer(GL_NONE);
    _complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    _depthPyramid.resize(width, height);
}

void DepthPrepass::begin()
{
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    glViewport(0, 0, _width, _height);
    glClear(GL_DEPTH_BUFFER_BIT);

    // No element is 0, so where nothing is drawn, nothing is nearest
    const std::array<GLfloat, 4> noHit{};
    glClearBufferfv(GL_COLOR, 1, noHit.data());
}

void DepthPrepass::bindHits(GLenum textureUnit)
{
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, _hitsTexture);
}

void DepthPrepass::releaseHits(GLenum textureUnit)
{
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, 0);
}
