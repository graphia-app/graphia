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

#include "edgedatatexels.h"

#include <algorithm>

EdgeDataTexels::EdgeDataTexels(int texelsPerEdge, GLenum internalFormat, GLenum format, size_t numTargets) :
    _texelsPerEdge(texelsPerEdge), _internalFormat(internalFormat), _format(format),
    _textures(numTargets, 0)
{
    resolveOpenGLFunctions();
}

EdgeDataTexels::~EdgeDataTexels()
{
    if(_fbo == 0)
        return;

    glDeleteFramebuffers(1, &_fbo);
    glDeleteTextures(static_cast<GLsizei>(_textures.size()), _textures.data());
}

bool EdgeDataTexels::reserve(size_t numEdges)
{
    const auto numTexels = static_cast<int>(numEdges) * _texelsPerEdge;

    if(numTexels <= _width * _height)
        return false;

    GLint maximumWidth = 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maximumWidth);

    _width = std::min(numTexels, maximumWidth);
    _height = (numTexels + _width - 1) / _width;

    if(_fbo == 0)
    {
        glGenTextures(static_cast<GLsizei>(_textures.size()), _textures.data());
        glGenFramebuffers(1, &_fbo);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

    for(size_t i = 0; i < _textures.size(); i++)
    {
        glBindTexture(GL_TEXTURE_2D, _textures.at(i));
        glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(_internalFormat),
            _width, _height, 0, _format, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + i),
            GL_TEXTURE_2D, _textures.at(i), 0);
    }

    _usable = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return true;
}

void EdgeDataTexels::bindForWriting()
{
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

    std::vector<GLenum> drawBuffers(_textures.size());
    for(size_t i = 0; i < drawBuffers.size(); i++)
        drawBuffers.at(i) = static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + i);

    glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
    glViewport(0, 0, _width, _height);
}

QVector2D EdgeDataTexels::size() const
{
    return {static_cast<float>(_width), static_cast<float>(_height)};
}
