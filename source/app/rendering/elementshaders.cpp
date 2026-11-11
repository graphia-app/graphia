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

#include "elementshaders.h"

#include "shadertools.h"

#include <QByteArray>
#include <QColor>
#include <QString>
#include <QStringList>
#include <QVector3D>

#include <algorithm>
#include <vector>

using namespace Qt::Literals::StringLiterals;

void ElementShaders::setLighting(QOpenGLShaderProgram& program)
{
    struct Light
    {
        QVector3D _position;
        QColor _color;
        float _specular = 1.0f;
    };

    const std::vector<Light> lights
    {
        {._position = {-0.707f,  0.0f,    0.707f}, ._color = {100, 100, 100}},
        {._position = { 0.0f,    0.0f,    1.0f  }, ._color = {150, 150, 150}},
        {._position = { 0.707f, -0.707f,  0.0f  }, ._color = {100, 100, 100}},
    };

    program.setUniformValue("numberOfLights", static_cast<GLuint>(lights.size()));

    for(size_t i = 0; i < lights.size(); i++)
    {
        const QByteArray light = u"lights[%1]"_s.arg(i).toLatin1();
        program.setUniformValue((light + ".position").constData(), lights.at(i)._position);
        program.setUniformValue((light + ".color").constData(), lights.at(i)._color);
        program.setUniformValue((light + ".specular").constData(), lights.at(i)._specular);
    }

    program.setUniformValue("material.ks", QVector3D(1.0f, 1.0f, 1.0f));
    program.setUniformValue("material.kd", QVector3D(1.0f, 1.0f, 1.0f));
    program.setUniformValue("material.ka", QVector3D(0.02f, 0.02f, 0.02f));
    program.setUniformValue("material.shininess", 50.0f);
}

void ElementShaders::setSamplerUnits(QOpenGLShaderProgram& program)
{
    const auto setUnit = [&](const char* name, Unit unit, int offset = 0)
    {
        program.setUniformValue(name, static_cast<GLint>(unit) + offset);
    };

    setUnit("componentData", Unit::ComponentData);
    setUnit("nodeGeometry", Unit::NodeGeometry);
    setUnit("visibleRanges", Unit::VisibleRanges);
    setUnit("previousVisibleRanges", Unit::PreviousVisibleRanges);
    setUnit("prepassHits", Unit::PrepassHits);
    setUnit("depthPyramid", Unit::DepthPyramid);
    setUnit("nearestNode", Unit::NearestNode);
    setUnit("farthestNode", Unit::FarthestNode);
    setUnit("nodeOcclusion", Unit::NodeOcclusion);
    setUnit("approximateShadingThirds0", Unit::ApproximateShadingThirds);
    setUnit("approximateShadingThirds1", Unit::ApproximateShadingThirds, 1);
    setUnit("approximateShadingThirds2", Unit::ApproximateShadingThirds, 2);
}

ElementShaders::ElementShaders()
{
    const auto nodeVertexShader = u":/shaders/nodeimpostor.vert"_s;
    const auto nodeImpostor = u":/shaders/nodeimpostor.frag"_s;
    const auto nodeCoverage = u":/shaders/nodecoverage.frag"_s;

    ShaderTools::loadShaderProgram(_nodes, nodeVertexShader, nodeImpostor);
    ShaderTools::loadShaderProgram(_nodeDepthPrepass, nodeVertexShader, nodeImpostor, {u"NODE_DEPTH_PREPASS"_s});
    ShaderTools::loadShaderProgram(_splattedNodes, nodeVertexShader, nodeCoverage, {u"NODE_COVERAGE"_s});
    ShaderTools::loadShaderProgram(_nodeDepths, nodeVertexShader, nodeCoverage,
        {u"NODE_COVERAGE"_s, u"NODE_DEPTHS"_s});
    ShaderTools::loadShaderProgram(_nodeOcclusion, nodeVertexShader, nodeCoverage,
        {u"NODE_COVERAGE"_s, u"NODE_OCCLUSION"_s});

    const auto edgeVertexShader = u":/shaders/edgeimpostor.vert"_s;
    const auto edgeImpostor = u":/shaders/edgeimpostor.frag"_s;
    const auto edgeCoverage = u":/shaders/edgecoverage.frag"_s;

    ShaderTools::loadShaderProgram(_fastSplattedEdges, edgeVertexShader, edgeCoverage, {u"EDGE_COVERAGE"_s});
    ShaderTools::loadShaderProgram(_splattedEdges, edgeVertexShader, edgeCoverage,
        {u"EDGE_COVERAGE"_s, u"EDGE_ACROSS"_s});
    ShaderTools::loadShaderProgram(_approximateTracedEdges, edgeVertexShader, edgeImpostor,
        {u"EDGE_APPROXIMATE_TRACED"_s});
    ShaderTools::loadShaderProgram(_tracedEdges, edgeVertexShader, edgeImpostor, {u"EDGE_TRACED"_s});
    ShaderTools::loadShaderProgram(_edgeDepthPrepass, edgeVertexShader, edgeImpostor,
        {u"EDGE_TRACED"_s, u"EDGE_DEPTH_PREPASS"_s});
    ShaderTools::loadShaderProgram(_edgeCull, u":/shaders/edgecull.vert"_s, u":/shaders/edgecull.frag"_s);
    ShaderTools::loadShaderProgram(_edgeFit, u":/shaders/edgefit.vert"_s, u":/shaders/edgefit.frag"_s);

    const auto setConstants = [](QOpenGLShaderProgram* program)
    {
        program->bind();
        setLighting(*program);
        setSamplerUnits(*program);
        program->release();
    };

    std::ranges::for_each(nodeShaders(), setConstants);
    std::ranges::for_each(edgeShaders(), setConstants);
}

void ElementShaders::setFrameUniforms(const FrameUniforms& uniforms)
{
    const auto setShared = [&](QOpenGLShaderProgram& program)
    {
        program.setUniformValue("viewportSize", uniforms._viewportSize);
        program.setUniformValue("viewportFraction", uniforms._viewportFraction);
        program.setUniformValue("flatness", uniforms._flatness);
        program.setUniformValue("outlineColor", uniforms._outlineColor);
        program.setUniformValue("outlineScale", uniforms._outlineScale);
    };

    for(auto* program : nodeShaders())
    {
        program->bind();
        setShared(*program);
        program->setUniformValue("widestEdgeSize", uniforms._widestEdgeSize);
        program->setUniformValue("minimumTracedWidthPixels", uniforms._thresholds._minimumTracedNodeWidth);
        program->release();
    }

    for(auto* program : edgeShaders())
    {
        program->bind();
        setShared(*program);
        program->setUniformValue("minimumApproximateTracedWidthPixels",
            uniforms._thresholds._minimumApproximateTracedEdgeWidth);
        program->setUniformValue("minimumTracedWidthPixels", uniforms._thresholds._minimumTracedEdgeWidth);
        program->setUniformValue("minimumSplattedWidthPixels", uniforms._thresholds._minimumSplattedEdgeWidth);
        program->release();
    }
}

std::vector<QOpenGLShaderProgram*> ElementShaders::nodeShaders()
{
    return {&_nodes, &_nodeDepthPrepass, &_splattedNodes, &_nodeDepths, &_nodeOcclusion};
}

std::vector<QOpenGLShaderProgram*> ElementShaders::edgeShaders()
{
    return {&_fastSplattedEdges, &_splattedEdges, &_approximateTracedEdges, &_tracedEdges,
        &_edgeDepthPrepass, &_edgeCull, &_edgeFit};
}
