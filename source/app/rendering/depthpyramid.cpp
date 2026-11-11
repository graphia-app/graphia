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

#include "depthpyramid.h"
#include "shadertools.h"

#include <QString>
#include <QVector2D>

#include <algorithm>

using namespace Qt::Literals::StringLiterals;

DepthPyramid::DepthPyramid()
{
    resolveOpenGLFunctions();

    glGenTextures(1, &_texture);
    glGenTextures(1, &_previousLevelTexture);
    glGenFramebuffers(1, &_levelFBO);
    glGenFramebuffers(1, &_previousLevelFBO);
    glGenFramebuffers(1, &_readFBO);

    ShaderTools::loadShaderProgram(_resolveShader, u":/shaders/fullscreen.vert"_s,
        u":/shaders/depthpyramid.frag"_s, {u"RESOLVE_DEPTH"_s});
    ShaderTools::loadShaderProgram(_reduceShader, u":/shaders/fullscreen.vert"_s,
        u":/shaders/depthpyramid.frag"_s);

    _resolveShader.bind();
    _resolveShader.setUniformValue("depthTexture", 0);
    _reduceShader.bind();
    _reduceShader.setUniformValue("previousLevel", 0);
    _reduceShader.release();
}

DepthPyramid::~DepthPyramid()
{
    glDeleteFramebuffers(1, &_readFBO);
    glDeleteFramebuffers(1, &_previousLevelFBO);
    glDeleteFramebuffers(1, &_levelFBO);
    glDeleteTextures(1, &_previousLevelTexture);
    glDeleteTextures(1, &_texture);
}

bool DepthPyramid::resize(int width, int height)
{
    _width = width;
    _height = height;

    _numLevels = 1;
    while((std::max(width, height) >> _numLevels) > 0)
        _numLevels++;

    allocate(_texture, _numLevels);
    allocate(_previousLevelTexture, 1);

    glBindFramebuffer(GL_FRAMEBUFFER, _previousLevelFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _previousLevelTexture, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, _levelFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture, 0);
    _usable = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return _usable;
}

void DepthPyramid::allocate(GLuint texture, int numLevels)
{
    glBindTexture(GL_TEXTURE_2D, texture);

    for(int level = 0; level < numLevels; level++)
    {
        glTexImage2D(GL_TEXTURE_2D, level, GL_R32F, std::max(1, _width >> level),
            std::max(1, _height >> level), 0, GL_RED, GL_FLOAT, nullptr);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, numLevels > 1 ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, numLevels - 1);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void DepthPyramid::build(GLuint depthTexture)
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDepthMask(GL_FALSE);

    drawLevel(0, _resolveShader, depthTexture);

    for(int level = 1; level < _numLevels; level++)
    {
        copyLevel(level - 1);

        _reduceShader.bind();
        _reduceShader.setUniformValue("previousLevelSize", QVector2D(
            static_cast<float>(std::max(1, _width >> (level - 1))),
            static_cast<float>(std::max(1, _height >> (level - 1)))));

        drawLevel(level, _reduceShader, _previousLevelTexture);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
}

void DepthPyramid::copyLevel(int level)
{
    const auto width = std::max(1, _width >> level);
    const auto height = std::max(1, _height >> level);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, _readFBO);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture, level);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _previousLevelFBO);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void DepthPyramid::drawLevel(int level, QOpenGLShaderProgram& shader, GLuint source)
{
    glBindFramebuffer(GL_FRAMEBUFFER, _levelFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture, level);

    const GLenum drawBuffer = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &drawBuffer);
    glViewport(0, 0, std::max(1, _width >> level), std::max(1, _height >> level));
    _screenPass.draw(shader, {source});
}

void DepthPyramid::bind(GLenum textureUnit, QOpenGLShaderProgram& shader)
{
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, _texture);
    shader.setUniformValue("depthPyramidLevels", _numLevels);
}

void DepthPyramid::release(GLenum textureUnit)
{
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, 0);
}
