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

#ifndef ELEMENTSHADERS_H
#define ELEMENTSHADERS_H

#include "openglfunctions.h"

#include <QColor>
#include <QOpenGLShaderProgram>
#include <QVector2D>

#include <cstdint>
#include <vector>

struct DetailThresholds
{
    float _minimumTracedEdgeWidth = 4.0f;
    float _minimumTracedNodeWidth = 3.0f;

    // Edges this wide are traced, since splatting blends overlapping edges that ought to hide each other
    float _minimumApproximateTracedEdgeWidth = 2.0f;
    float _minimumSplattedEdgeWidth = 0.5f;

    static DetailThresholds exact()
    {
        return {._minimumTracedEdgeWidth = 0.0f, ._minimumTracedNodeWidth = 0.0f,
            ._minimumApproximateTracedEdgeWidth = 0.0f, ._minimumSplattedEdgeWidth = 0.0f};
    }
};

struct ElementShaders
{
    enum class Unit : uint8_t
    {
        ComponentData,
        NodeGeometry,
        VisibleRanges,
        PreviousVisibleRanges,
        PrepassHits,
        DepthPyramid,
        NearestNode,
        FarthestNode,
        NodeOcclusion,
        ApproximateShadingThirds, // and the two units after it
    };

    static GLenum glUnit(Unit unit) { return GL_TEXTURE0 + static_cast<GLenum>(unit); }

    QOpenGLShaderProgram _nodes;
    QOpenGLShaderProgram _nodeDepthPrepass;
    QOpenGLShaderProgram _splattedNodes;
    QOpenGLShaderProgram _nodeDepths;
    QOpenGLShaderProgram _nodeOcclusion;

    QOpenGLShaderProgram _fastSplattedEdges;
    QOpenGLShaderProgram _splattedEdges;
    QOpenGLShaderProgram _approximateTracedEdges;
    QOpenGLShaderProgram _tracedEdges;
    QOpenGLShaderProgram _edgeDepthPrepass;
    QOpenGLShaderProgram _edgeCull;
    QOpenGLShaderProgram _edgeFit;

    struct FrameUniforms
    {
        QVector2D _viewportSize;
        float _flatness = 0.0f;
        QColor _outlineColor;
        float _widestEdgeSize = 0.0f;
        DetailThresholds _thresholds;
    };

    ElementShaders();
    void setFrameUniforms(const FrameUniforms& uniforms);

private:
    static void setLighting(QOpenGLShaderProgram& program);
    static void setSamplerUnits(QOpenGLShaderProgram& program);

    std::vector<QOpenGLShaderProgram*> nodeShaders();
    std::vector<QOpenGLShaderProgram*> edgeShaders();
};

#endif // ELEMENTSHADERS_H
