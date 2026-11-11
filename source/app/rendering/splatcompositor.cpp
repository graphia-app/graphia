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

#include <utility>

using namespace Qt::Literals::StringLiterals;

SplatCompositor::SplatCompositor(GLenum firstNodeUnit) :
    _nearestNodeUnit(firstNodeUnit), _farthestNodeUnit(firstNodeUnit + 1), _nodeOcclusionUnit(firstNodeUnit + 2)
{
    resolveOpenGLFunctions();

    glGenTextures(1, &_sums);
    glGenTextures(1, &_weights);
    glGenTextures(1, &_overlap);
    glGenFramebuffers(1, &_framebuffer);

    glGenTextures(1, &_nearestNode);
    glGenTextures(1, &_farthestNode);
    glGenTextures(1, &_nodeOcclusion);
    glGenFramebuffers(1, &_nearestNodeFramebuffer);
    glGenFramebuffers(1, &_farthestNodeFramebuffer);
    glGenFramebuffers(1, &_nodeOcclusionFramebuffer);

    ShaderTools::loadShaderProgram(_resolveShader, u":/shaders/fullscreen.vert"_s, u":/shaders/coverage.frag"_s);
    _resolveShader.bind();
    _resolveShader.setUniformValue("sums", 0);
    _resolveShader.setUniformValue("weights", 1);
    _resolveShader.setUniformValue("overlap", 2);
    _resolveShader.release();
}

SplatCompositor::~SplatCompositor()
{
    glDeleteFramebuffers(1, &_nodeOcclusionFramebuffer);
    glDeleteFramebuffers(1, &_farthestNodeFramebuffer);
    glDeleteFramebuffers(1, &_nearestNodeFramebuffer);
    glDeleteTextures(1, &_nodeOcclusion);
    glDeleteTextures(1, &_farthestNode);
    glDeleteTextures(1, &_nearestNode);
    glDeleteFramebuffers(1, &_framebuffer);
    glDeleteTextures(1, &_overlap);
    glDeleteTextures(1, &_weights);
    glDeleteTextures(1, &_sums);
}

bool SplatCompositor::resize(int width, int height, GLuint depthTexture)
{
    _width = width;
    _height = height;

    auto resizeTexture = [&](GLuint texture, GLint internalFormat, GLenum format, GLenum type)
    {
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    };

    resizeTexture(_sums, GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT);
    resizeTexture(_weights, GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT);
    resizeTexture(_overlap, GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT);
    resizeTexture(_nearestNode, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT);
    resizeTexture(_farthestNode, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT);
    resizeTexture(_nodeOcclusion, GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT);

    glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _sums, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _weights, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, _overlap, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;

    const GLenum noColor = GL_NONE;

    for(auto [framebuffer, depth] : {std::pair{_nearestNodeFramebuffer, _nearestNode},
        std::pair{_farthestNodeFramebuffer, _farthestNode}})
    {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
        glDrawBuffers(1, &noColor);
        glReadBuffer(GL_NONE);
        complete = complete && glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    }

    // Finding the occlusion reads both depths, so it has a target of its own
    // ES draws the node shader's third output only to the third attachment
    glBindFramebuffer(GL_FRAMEBUFFER, _nodeOcclusionFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, _nodeOcclusion, 0);
    const GLenum occlusion[] = {GL_NONE, GL_NONE, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, static_cast<const GLenum*>(occlusion));
    complete = complete && glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return complete;
}

void SplatCompositor::bindFramebuffer(GLuint framebuffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glViewport(0, 0, _width, _height);
}

// Nodes drawn after this leave the nearest depth in each pixel
// The nodes' occlusion is cleared here, whether any nodes are drawn or not
void SplatCompositor::beginNearestNodes()
{
    bindFramebuffer(_nodeOcclusionFramebuffer);
    const GLfloat unoccluded[] = {0.0f, 0.0f, 0.0f, 0.0f};
    glClearBufferfv(GL_COLOR, 2, static_cast<const GLfloat*>(unoccluded));

    bindFramebuffer(_nearestNodeFramebuffer);
    const GLfloat farthest = 1.0f;
    glClearBufferfv(GL_DEPTH, 0, &farthest);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
}

// Nodes drawn after this leave the depth of the farthest in each pixel
void SplatCompositor::beginFarthestNodes()
{
    bindFramebuffer(_farthestNodeFramebuffer);

    const GLfloat nearest = 0.0f;
    glClearBufferfv(GL_DEPTH, 0, &nearest);
    glDepthFunc(GL_GREATER);
}

// Nodes drawn after this sum -log of what each leaves uncovered
// They also record where that cover lies between the nearest and farthest depth
void SplatCompositor::beginNodeOcclusion()
{
    bindFramebuffer(_nodeOcclusionFramebuffer);
    glDepthFunc(GL_LESS);

    glActiveTexture(_nearestNodeUnit);
    glBindTexture(GL_TEXTURE_2D, _nearestNode);
    glActiveTexture(_farthestNodeUnit);
    glBindTexture(GL_TEXTURE_2D, _farthestNode);
    glActiveTexture(GL_TEXTURE0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);
}

void SplatCompositor::begin()
{
    bindFramebuffer(_framebuffer);
    glEnable(GL_DEPTH_TEST);

    glActiveTexture(_nearestNodeUnit);
    glBindTexture(GL_TEXTURE_2D, _nearestNode);
    glActiveTexture(_farthestNodeUnit);
    glBindTexture(GL_TEXTURE_2D, _farthestNode);
    glActiveTexture(_nodeOcclusionUnit);
    glBindTexture(GL_TEXTURE_2D, _nodeOcclusion);
    glActiveTexture(GL_TEXTURE0);

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

    glActiveTexture(_nodeOcclusionUnit);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(_farthestNodeUnit);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(_nearestNodeUnit);
    glBindTexture(GL_TEXTURE_2D, 0);

    // The layer's colour and selection; element numbers can't be blended
    bindFramebuffer(layerFramebuffer);
    const GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_NONE, GL_NONE, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, static_cast<const GLenum*>(drawBuffers));
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    _screenPass.draw(_resolveShader, {_sums, _weights, _overlap});

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}
