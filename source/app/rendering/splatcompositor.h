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

#ifndef SPLATCOMPOSITOR_H
#define SPLATCOMPOSITOR_H

#include "openglfunctions.h"
#include "screenpass.h"

#include <QOpenGLShaderProgram>

// Blends shapes that cover parts of pixels over a layer, in any order, by weighted
// blended order independent transparency
class SplatCompositor : public OpenGLFunctions
{
public:
    // The nodes' nearest and farthest depths and their occlusion are read from
    // three units from firstNodeUnit
    explicit SplatCompositor(GLenum firstNodeUnit);
    ~SplatCompositor() override;

    SplatCompositor(const SplatCompositor&) = delete;
    SplatCompositor& operator=(const SplatCompositor&) = delete;

    bool resize(int width, int height, GLuint depthTexture);

    // The node passes, which come before begin()
    // The shader tells them apart by which of its outputs are drawn
    void beginNearestNodes();
    void beginFarthestNodes();
    void beginNodeOcclusion();

    void begin();
    void resolveInto(GLuint layerFramebuffer);

private:
    GLenum _nearestNodeUnit;
    GLenum _farthestNodeUnit;
    GLenum _nodeOcclusionUnit;

    GLuint _sums = 0;
    GLuint _weights = 0;
    GLuint _overlap = 0;

    GLuint _framebuffer = 0;
    int _width = 0;
    int _height = 0;

    // The depths of the nearest and farthest node in each pixel, and what the
    // nodes leave uncovered, with where among them that is
    GLuint _nearestNode = 0;
    GLuint _farthestNode = 0;
    GLuint _nodeOcclusion = 0;
    GLuint _nearestNodeFramebuffer = 0;
    GLuint _farthestNodeFramebuffer = 0;
    GLuint _nodeOcclusionFramebuffer = 0;

    QOpenGLShaderProgram _resolveShader;
    ScreenPass _screenPass;

    void bindFramebuffer(GLuint framebuffer);
};

#endif // SPLATCOMPOSITOR_H
