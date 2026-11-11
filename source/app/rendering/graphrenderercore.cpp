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

#include "graphrenderercore.h"

#include "app/preferences.h"

#include "glyphmap.h"
#include "shadertools.h"

#include "app/ui/document.h"

#include <QColor>
#include <QDebug>
#include <QDir>
#include <QImage>
#include <QOpenGLContext>
#include <QPoint>
#include <QString>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QtGlobal>

#include <algorithm>
#include <limits>
#include <ranges>

using namespace Qt::Literals::StringLiterals;

namespace
{
struct TextureFormatAndType
{
    GLenum _format = 0;
    GLenum _type = 0;
};

TextureFormatAndType textureFormatAndTypeFor(GLint internalFormat)
{
    TextureFormatAndType tfat;

    switch(internalFormat)
    {
    case GL_RGBA:               tfat = {._format = GL_RGBA,            ._type = GL_UNSIGNED_BYTE}; break;
    case GL_RG32F:              tfat = {._format = GL_RG,              ._type = GL_FLOAT};         break;
    case GL_R8:                 tfat = {._format = GL_RED,             ._type = GL_UNSIGNED_BYTE}; break;
    case GL_DEPTH_COMPONENT32F: tfat = {._format = GL_DEPTH_COMPONENT, ._type = GL_FLOAT};         break;

    default: qFatal("Unhandled OpenGL internal format");
    }

    return tfat;
}

void setupTexture(OpenGLFunctions* t, GLuint& texture, int width, int height, GLint internalFormat)
{
    if(texture == 0)
        t->glGenTextures(1, &texture);

    t->glBindTexture(GL_TEXTURE_2D, texture);

    auto tfat = textureFormatAndTypeFor(internalFormat);
    t->glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, tfat._format, tfat._type, nullptr);

    t->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    t->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    t->glBindTexture(GL_TEXTURE_2D, 0);
}
} // namespace

GPUGraphData::GPUGraphData()
{
    resolveOpenGLFunctions();
}

void GPUGraphData::initialise(QOpenGLShaderProgram& nodesShader,
                              QOpenGLShaderProgram& edgesShader,
                              QOpenGLShaderProgram& textShader)
{
    _nodeQuad.create(nodesShader);
    _edgeQuads.create(edgesShader);

    _rectangle.create(textShader);

    prepareVertexBuffers();
    prepareNodeVAO(nodesShader);
    prepareEdgeVAO(edgesShader);
    prepareTextVAO(textShader);
}

GPUGraphData::~GPUGraphData()
{
    if(_fbo != 0)
    {
        glDeleteFramebuffers(1, &_fbo);
        _fbo = 0;
    }

    if(_colorTexture != 0)
    {
        glDeleteTextures(1, &_colorTexture);
        _colorTexture = 0;
    }

    if(_elementTexture != 0)
    {
        glDeleteTextures(1, &_elementTexture);
        _elementTexture = 0;
    }

    if(_subElementTexture != 0)
    {
        glDeleteTextures(1, &_subElementTexture);
        _subElementTexture = 0;
    }

    if(_selectionTexture != 0)
    {
        glDeleteTextures(1, &_selectionTexture);
        _selectionTexture = 0;
    }
}

void GPUGraphData::prepareVertexBuffers()
{
    if(!_nodeVBO.isCreated())
    {
        _nodeVBO.create();
        _nodeVBO.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    }

    if(!_textVBO.isCreated())
    {
        _textVBO.create();
        _textVBO.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    }

    if(!_edgeVBO.isCreated())
    {
        _edgeVBO.create();
        _edgeVBO.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    }
}

void GPUGraphData::prepareTextVAO(QOpenGLShaderProgram& shader)
{
    _rectangle.vertexArrayObject()->bind();
    shader.bind();
    _textVBO.bind();

    shader.enableAttributeArray("component");
    shader.enableAttributeArray("textureCoord");
    shader.enableAttributeArray("textureLayer");
    shader.enableAttributeArray("anchorNodes");
    shader.enableAttributeArray("basePosition");
    shader.enableAttributeArray("glyphOffset");
    shader.enableAttributeArray("glyphSize");
    shader.enableAttributeArray("glyphScale");
    shader.enableAttributeArray("textHeight");
    shader.enableAttributeArray("color");

    glVertexAttribIPointer(static_cast<GLuint>(shader.attributeLocation("component")),              1, GL_INT,  sizeof(GlyphData),
                             reinterpret_cast<const void*>(offsetof(GlyphData, _component))); // NOLINT(performance-no-int-to-ptr)
    shader.setAttributeBuffer("textureCoord",    GL_FLOAT, offsetof(GlyphData, _textureCoord),      2,          sizeof(GlyphData));
    glVertexAttribIPointer(static_cast<GLuint>(shader.attributeLocation("textureLayer")),           1, GL_INT,  sizeof(GlyphData),
                             reinterpret_cast<const void*>(offsetof(GlyphData, _textureLayer))); // NOLINT(performance-no-int-to-ptr)
    glVertexAttribIPointer(static_cast<GLuint>(shader.attributeLocation("anchorNodes")),            2, GL_INT,  sizeof(GlyphData),
                             reinterpret_cast<const void*>(offsetof(GlyphData, _anchorNodes))); // NOLINT(performance-no-int-to-ptr)
    shader.setAttributeBuffer("basePosition",    GL_FLOAT, offsetof(GlyphData, _basePosition),      3,          sizeof(GlyphData));
    shader.setAttributeBuffer("glyphOffset",     GL_FLOAT, offsetof(GlyphData, _glyphOffset),       2,          sizeof(GlyphData));
    shader.setAttributeBuffer("glyphSize",       GL_FLOAT, offsetof(GlyphData, _glyphSize),         2,          sizeof(GlyphData));
    shader.setAttributeBuffer("glyphScale",      GL_FLOAT, offsetof(GlyphData, _glyphScale),        1,          sizeof(GlyphData));
    shader.setAttributeBuffer("textHeight",      GL_FLOAT, offsetof(GlyphData, _textHeight),        1,          sizeof(GlyphData));
    shader.setAttributeBuffer("color",           GL_FLOAT, offsetof(GlyphData, _color),             3,          sizeof(GlyphData));
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("component")),               1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("textureCoord")),            1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("textureLayer")),            1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("anchorNodes")),             1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("basePosition")),            1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("glyphOffset")),             1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("glyphSize")),               1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("glyphScale")),              1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("textHeight")),              1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("color")),                   1);

    _textVBO.release();
    shader.release();
    _rectangle.vertexArrayObject()->release();
}

void GPUGraphData::prepareNodeVAO(QOpenGLShaderProgram& shader)
{
    _nodeQuad.vertexArrayObject()->bind();
    shader.bind();

    _nodeVBO.bind();
    shader.enableAttributeArray("nodeIndex");
    shader.enableAttributeArray("component");
    shader.enableAttributeArray("outerColor");
    shader.enableAttributeArray("innerColor");
    shader.enableAttributeArray("selected");
    glVertexAttribIPointer(static_cast<GLuint>(shader.attributeLocation("nodeIndex")),      1, GL_INT,  sizeof(NodeData),
                          reinterpret_cast<const void*>(offsetof(NodeData, _nodeIndex))); // NOLINT(performance-no-int-to-ptr)
    glVertexAttribIPointer(static_cast<GLuint>(shader.attributeLocation("component")),      1, GL_INT,  sizeof(NodeData),
                          reinterpret_cast<const void*>(offsetof(NodeData, _component))); // NOLINT(performance-no-int-to-ptr)
    glVertexAttribPointer(static_cast<GLuint>(shader.attributeLocation("outerColor")),      4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(NodeData),
                          reinterpret_cast<const void*>(offsetof(NodeData, _outerColor))); // NOLINT(performance-no-int-to-ptr)
    glVertexAttribPointer(static_cast<GLuint>(shader.attributeLocation("innerColor")),      4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(NodeData),
                          reinterpret_cast<const void*>(offsetof(NodeData, _innerColor))); // NOLINT(performance-no-int-to-ptr)
    shader.setAttributeBuffer("selected",     GL_FLOAT, offsetof(NodeData, _selected),      1,          sizeof(NodeData));
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("nodeIndex")),       1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("component")),       1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("innerColor")),      1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("outerColor")),      1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("selected")),        1);
    _nodeVBO.release();

    shader.release();
    _nodeQuad.vertexArrayObject()->release();
}

void GPUGraphData::prepareEdgeVAO(QOpenGLShaderProgram& shader)
{
    _edgeQuads.vertexArrayObject()->bind();
    shader.bind();

    _edgeVBO.bind();
    shader.enableAttributeArray("sourceIndex");
    shader.enableAttributeArray("targetIndex");
    shader.enableAttributeArray("edgeType");
    shader.enableAttributeArray("component");
    shader.enableAttributeArray("size");
    shader.enableAttributeArray("outerColor");
    shader.enableAttributeArray("innerColor");
    shader.enableAttributeArray("selected");
    glVertexAttribIPointer(static_cast<GLuint>(shader.attributeLocation("sourceIndex")),        1, GL_INT,  sizeof(EdgeData),
                            reinterpret_cast<const void*>(offsetof(EdgeData, _sourceIndex))); // NOLINT(performance-no-int-to-ptr)
    glVertexAttribIPointer(static_cast<GLuint>(shader.attributeLocation("targetIndex")),        1, GL_INT,  sizeof(EdgeData),
                            reinterpret_cast<const void*>(offsetof(EdgeData, _targetIndex))); // NOLINT(performance-no-int-to-ptr)
    glVertexAttribIPointer(static_cast<GLuint>(shader.attributeLocation("edgeType")),           1, GL_INT,  sizeof(EdgeData),
                            reinterpret_cast<const void*>(offsetof(EdgeData, _edgeType))); // NOLINT(performance-no-int-to-ptr)
    glVertexAttribIPointer(static_cast<GLuint>(shader.attributeLocation("component")),          1, GL_INT,  sizeof(EdgeData),
                           reinterpret_cast<const void*>(offsetof(EdgeData, _component))); // NOLINT(performance-no-int-to-ptr)
    shader.setAttributeBuffer("size",           GL_FLOAT, offsetof(EdgeData, _size),            1,          sizeof(EdgeData));
    glVertexAttribPointer(static_cast<GLuint>(shader.attributeLocation("outerColor")),          4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(EdgeData),
                            reinterpret_cast<const void*>(offsetof(EdgeData, _outerColor))); // NOLINT(performance-no-int-to-ptr)
    glVertexAttribPointer(static_cast<GLuint>(shader.attributeLocation("innerColor")),          4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(EdgeData),
                            reinterpret_cast<const void*>(offsetof(EdgeData, _innerColor))); // NOLINT(performance-no-int-to-ptr)
    shader.setAttributeBuffer("selected",       GL_FLOAT, offsetof(EdgeData, _selected),        1,          sizeof(EdgeData));
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("sourceIndex")),         1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("targetIndex")),         1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("edgeType")),            1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("component")),           1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("size")),                1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("outerColor")),          1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("innerColor")),          1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("selected")),            1);
    _edgeVBO.release();

    shader.release();
    _edgeQuads.vertexArrayObject()->release();
}

bool GPUGraphData::prepareRenderBuffers(int width, int height, GLuint depthTexture)
{
    setupTexture(this, _colorTexture,      width, height, GL_RGBA);
    setupTexture(this, _elementTexture,    width, height, GL_RG32F);
    setupTexture(this, _subElementTexture, width, height, GL_R8);
    setupTexture(this, _selectionTexture,  width, height, GL_RGBA);

    // SMAA and the outline read the selection part way between its texels
    glBindTexture(GL_TEXTURE_2D, _selectionTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    if(_fbo == 0)
        glGenFramebuffers(1, &_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, _elementTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, _subElementTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, _selectionTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, depthTexture, 0);

    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    const bool fboValid = (status == GL_FRAMEBUFFER_COMPLETE);
    Q_ASSERT(fboValid);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return fboValid;
}

void GPUGraphData::reset()
{
    _componentAlpha = 0.0f;
    _unhighlightAlpha = 0.0f;
    _isOverlay = false;
    _elementsSelected = false;
    _extents = {};
    _nodeData.clear();
    _edgeData.clear();
    _glyphData.clear();
    _tallestTextHeight = 0.0f;
}

void GPUGraphData::clearFramebuffer(GLbitfield buffers)
{
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, static_cast<GLenum*>(drawBuffers));

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(buffers);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GPUGraphData::clearDepthbuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_NONE, GL_NONE, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, static_cast<GLenum*>(drawBuffers));

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GPUGraphData::drawToFramebuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
    glDrawBuffers(4, static_cast<GLenum*>(drawBuffers));
}

void GPUGraphData::upload()
{
    _nodeVBO.bind();
    _nodeVBO.allocate(_nodeData.data(), static_cast<int>(_nodeData.size() * sizeof(NodeData)));
    _nodeVBO.release();

    _edgeVBO.bind();
    _edgeVBO.allocate(_edgeData.data(), static_cast<int>(_edgeData.size() * sizeof(EdgeData)));
    _edgeVBO.release();

    _textVBO.bind();
    _textVBO.allocate(_glyphData.data(), static_cast<int>(_glyphData.size() * sizeof(GlyphData)));
    _textVBO.release();
}

size_t GPUGraphData::numNodes() const
{
    return _nodeData.size();
}

size_t GPUGraphData::numEdges() const
{
    return _edgeData.size();
}

void GPUGraphData::addNode(const NodeData& nodeData, float size)
{
    _nodeData.push_back(nodeData);
    _extents._smallestNodeSize = std::min(_extents._smallestNodeSize, size);
    _extents._largestNodeSize = std::max(_extents._largestNodeSize, size);
}

void GPUGraphData::addEdge(const EdgeData& edgeData)
{
    _edgeData.push_back(edgeData);
    _extents._hasArrowHeads = _extents._hasArrowHeads || edgeData._edgeType == static_cast<int>(EdgeVisualType::Arrow);
    _extents._smallestEdgeSize = std::min(_extents._smallestEdgeSize, edgeData._size);
    _extents._largestEdgeSize = std::max(_extents._largestEdgeSize, edgeData._size);
}

void GPUGraphData::addGlyph(const GlyphData& glyphData)
{
    _glyphData.push_back(glyphData);
    _tallestTextHeight = std::max(_tallestTextHeight, glyphData._textHeight);
}

size_t GPUGraphData::numGlyphs() const
{
    return _glyphData.size();
}

float GPUGraphData::alpha() const
{
    return _componentAlpha * _unhighlightAlpha;
}

float GPUGraphData::selectionAlpha() const
{
    return ((_componentAlpha * 1.0f) + (_unhighlightAlpha * 2.0f)) / 3.0f;
}

float GPUGraphData::componentAlpha() const
{
    return _componentAlpha;
}

float GPUGraphData::unhighlightAlpha() const
{
    return _unhighlightAlpha;
}

bool GPUGraphData::unused() const
{
    return _componentAlpha == 0.0f && _unhighlightAlpha == 0.0f;
}

bool GPUGraphData::empty() const
{
    return _nodeData.empty() && _edgeData.empty() && _glyphData.empty();
}

bool GPUGraphData::invisible() const
{
    return alpha() <= 0.0f || empty();
}

bool GPUGraphData::hasGraphElements() const
{
    return !_nodeData.empty() || !_edgeData.empty();
}

GPUGraphData::Color GPUGraphData::colorFrom(const QColor& color)
{
    return {static_cast<uint8_t>(color.red()), static_cast<uint8_t>(color.green()),
        static_cast<uint8_t>(color.blue()), static_cast<uint8_t>(color.alpha())};
}

void GPUGraphData::copyState(const GPUGraphData& gpuGraphData,
    QOpenGLShaderProgram& nodesShader,
    QOpenGLShaderProgram& edgesShader,
    QOpenGLShaderProgram& textShader)
{
    _componentAlpha = gpuGraphData._componentAlpha;
    _unhighlightAlpha = gpuGraphData._unhighlightAlpha;
    _isOverlay = gpuGraphData._isOverlay;
    _extents = gpuGraphData._extents;
    _nodeData = gpuGraphData._nodeData;
    _glyphData = gpuGraphData._glyphData;
    _tallestTextHeight = gpuGraphData._tallestTextHeight;
    _edgeData = gpuGraphData._edgeData;
    _elementsSelected = gpuGraphData._elementsSelected;

    // Cause VBO to be recreated
    _fbo = 0;
    _colorTexture = 0;
    _elementTexture = 0;
    _subElementTexture = 0;
    _selectionTexture = 0;

    _edgeVBO.destroy();
    _nodeVBO.destroy();
    _textVBO.destroy();

    initialise(nodesShader, edgesShader, textShader);
}

GraphRendererCore::GraphRendererCore(DetailThresholds detailThresholds) :
    _context(QOpenGLContext::currentContext()),
    _detailThresholds(detailThresholds)
{
    resolveOpenGLFunctions();

    ShaderTools::loadShaderProgram(_screenShader, u":/shaders/fullscreen.vert"_s, u":/shaders/screen.frag"_s);
    ShaderTools::loadShaderProgram(_outlineShader, u":/shaders/fullscreen.vert"_s, u":/shaders/outline.frag"_s);
    ShaderTools::loadShaderProgram(_selectionShader, u":/shaders/fullscreen.vert"_s, u":/shaders/selection.frag"_s);
    ShaderTools::loadShaderProgram(_compositeElementShader, u":/shaders/fullscreen.vert"_s,
        u":/shaders/compositeelement.frag"_s);
    _compositeElementShader.bind();
    _compositeElementShader.setUniformValue("subElementTexture", 1);
    _compositeElementShader.release();

    ShaderTools::loadShaderProgram(_selectionMarkerShader, u":/shaders/2d.vert"_s, u":/shaders/selectionMarker.frag"_s);

    ShaderTools::loadShaderProgram(_sdfShader, u":/shaders/screen.vert"_s, u":/shaders/sdf.frag"_s);
    ShaderTools::loadShaderProgram(_textShader, u":/shaders/textrender.vert"_s, u":/shaders/textrender.frag"_s);

    for(auto& gpuGraphData : _gpuGraphData)
        gpuGraphData.initialise(_elementShaders._nodes, _elementShaders._tracedEdges, _textShader);

    prepareSelectionMarkerVAO();
    prepareScreenQuad();
}

GraphRendererCore::~GraphRendererCore()
{
    if(_depthTexture != 0)
    {
        glDeleteTextures(1, &_depthTexture);
        _depthTexture = 0;
    }
}

void GraphRendererCore::bindComponentDataTexture(GLenum textureUnit, QOpenGLShaderProgram& shader)
{
    _componentDataTexels.bind(textureUnit);
    shader.setUniformValue("componentData", textureUnit - GL_TEXTURE0);
}

void GraphRendererCore::bindNodeGeometryTexture(GLenum textureUnit, QOpenGLShaderProgram& shader)
{
    _nodeGeometry.bind(textureUnit);
    shader.setUniformValue("nodeGeometry", textureUnit - GL_TEXTURE0);
}

bool GraphRendererCore::makeContextCurrent()
{
    return _context->makeCurrent(_context->surface());
}

void GraphRendererCore::renderToLayer(GPUGraphData& gpuGraphData)
{
    gpuGraphData.drawToFramebuffer();
    glViewport(0, 0, _width, _height);
}

void GraphRendererCore::renderNodesWithShader(QOpenGLShaderProgram& shader, GPUGraphData& gpuGraphData,
    NamedBool<"asPoints"> asPoints)
{
    if(gpuGraphData.numNodes() == 0)
        return;

    shader.bind();
    gpuGraphData._nodeVBO.bind();

    _componentDataTexels.bind(ElementShaders::glUnit(ElementShaders::Unit::ComponentData));
    _nodeGeometry.bind(ElementShaders::glUnit(ElementShaders::Unit::NodeGeometry));

    gpuGraphData._nodeQuad.vertexArrayObject()->bind();

    if(asPoints)
    {
#ifndef OPENGL_ES
        glEnable(GL_PROGRAM_POINT_SIZE);
#endif
        glDrawArraysInstanced(GL_POINTS, 0, 1, static_cast<GLsizei>(gpuGraphData.numNodes()));
#ifndef OPENGL_ES
        glDisable(GL_PROGRAM_POINT_SIZE);
#endif
    }
    else
    {
        glDrawElementsInstanced(GL_TRIANGLES, Primitive::ImpostorQuads::glIndexCount(1),
            GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(gpuGraphData.numNodes()));
    }

    gpuGraphData._nodeQuad.vertexArrayObject()->release();

    _nodeGeometry.release(ElementShaders::glUnit(ElementShaders::Unit::NodeGeometry));
    _componentDataTexels.release(ElementShaders::glUnit(ElementShaders::Unit::ComponentData));
    gpuGraphData._nodeVBO.release();
    shader.release();
}

void GraphRendererCore::bindEdgeParameters(GPUGraphData& gpuGraphData)
{
    gpuGraphData._edgeVBO.bind();

    _componentDataTexels.bind(ElementShaders::glUnit(ElementShaders::Unit::ComponentData));
    _nodeGeometry.bind(ElementShaders::glUnit(ElementShaders::Unit::NodeGeometry));
}

void GraphRendererCore::releaseEdgeParameters(GPUGraphData& gpuGraphData)
{
    _nodeGeometry.release(ElementShaders::glUnit(ElementShaders::Unit::NodeGeometry));
    _componentDataTexels.release(ElementShaders::glUnit(ElementShaders::Unit::ComponentData));
    gpuGraphData._edgeVBO.release();
}

void GraphRendererCore::renderEdgesWithShader(QOpenGLShaderProgram& shader, GPUGraphData& gpuGraphData,
    CullingResults cullingResults)
{
    shader.bind();
    bindEdgeParameters(gpuGraphData);
    gpuGraphData._edgeCullResults.bind(ElementShaders::glUnit(ElementShaders::Unit::VisibleRanges), shader);

    const bool withPrepassHits = cullingResults == CullingResults::VisibleRangesAndPrepassHits;

    if(withPrepassHits)
    {
        gpuGraphData._edgeCullResults.bindPrevious(ElementShaders::glUnit(ElementShaders::Unit::PreviousVisibleRanges),
            shader);
        _depthPrepass.bindHits(ElementShaders::glUnit(ElementShaders::Unit::PrepassHits));
    }

    shader.setUniformValue("hasVisibleRanges", cullingResults != CullingResults::None ? 1 : 0);

    gpuGraphData._edgeQuads.vertexArrayObject()->bind();
    glDrawElementsInstanced(GL_TRIANGLES,
        Primitive::ImpostorQuads::glIndexCount(gpuGraphData._extents._hasArrowHeads ? 2 : 1),
        GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(gpuGraphData.numEdges()));
    gpuGraphData._edgeQuads.vertexArrayObject()->release();

    if(withPrepassHits)
    {
        _depthPrepass.releaseHits(ElementShaders::glUnit(ElementShaders::Unit::PrepassHits));
        gpuGraphData._edgeCullResults.release(ElementShaders::glUnit(ElementShaders::Unit::PreviousVisibleRanges));
    }

    gpuGraphData._edgeCullResults.release(ElementShaders::glUnit(ElementShaders::Unit::VisibleRanges));
    releaseEdgeParameters(gpuGraphData);
    shader.release();
}

// Traced edges write their depth, which turns off the early depth test
// So the stretch of each that may be seen is found first, by walking it along the depth prepass
bool GraphRendererCore::cullHiddenEdgeStretches(GPUGraphData& gpuGraphData)
{
    auto& cullResults = gpuGraphData._edgeCullResults;
    cullResults.reserve(gpuGraphData.numEdges());

    if(!_depthPrepass.usable() || !cullResults.usable())
        return false;

    _depthPrepass.begin();

    renderNodesWithShader(_elementShaders._nodeDepthPrepass, gpuGraphData);
    renderEdgesWithShader(_elementShaders._edgeDepthPrepass, gpuGraphData,
        cullResults.written() ? CullingResults::VisibleRanges : CullingResults::None);

    _depthPrepass.buildPyramid();

    auto& cullShader = _elementShaders._edgeCull;
    cullShader.bind();
    cullResults.bindForWriting(cullShader);
    bindEdgeParameters(gpuGraphData);
    _depthPrepass.bindPyramid(ElementShaders::glUnit(ElementShaders::Unit::DepthPyramid), cullShader);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    // A point for each part of each edge
    gpuGraphData._edgeQuads.vertexArrayObject()->bind();
    glDrawArraysInstanced(GL_POINTS, 0, gpuGraphData._extents._hasArrowHeads ? 2 : 1,
        static_cast<GLsizei>(gpuGraphData.numEdges()));
    gpuGraphData._edgeQuads.vertexArrayObject()->release();

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    _depthPrepass.releasePyramid(ElementShaders::glUnit(ElementShaders::Unit::DepthPyramid));
    releaseEdgeParameters(gpuGraphData);
    cullShader.release();

    return true;
}

void GraphRendererCore::fitEdgeShading(GPUGraphData& gpuGraphData)
{
    _approximateEdgeShading.reserve(gpuGraphData.numEdges());

    auto& fitShader = _elementShaders._edgeFit;
    fitShader.bind();
    _approximateEdgeShading.bindForWriting(fitShader);
    bindEdgeParameters(gpuGraphData);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    gpuGraphData._edgeQuads.vertexArrayObject()->bind();
    glDrawArraysInstanced(GL_POINTS, 0, 1, static_cast<GLsizei>(gpuGraphData.numEdges()));
    gpuGraphData._edgeQuads.vertexArrayObject()->release();

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    releaseEdgeParameters(gpuGraphData);
    fitShader.release();
}

GraphRendererCore::DetailPasses GraphRendererCore::detailPasses(const GPUGraphData& gpuGraphData,
    const PixelScaleRange& pixelScale) const
{
    const auto& thresholds = _detailThresholds;
    const auto& extents = gpuGraphData._extents;
    DetailPasses passes;

    const auto narrowestNode = 2.0f * extents._smallestNodeSize * pixelScale._minimum;
    const auto widestNode = 2.0f * extents._largestNodeSize * pixelScale._maximum;
    passes._tracedNodes = widestNode >= thresholds._minimumTracedNodeWidth;
    passes._splattedNodes = narrowestNode < thresholds._minimumTracedNodeWidth;

    if(!u::pref(u"visuals/showEdges"_s).toBool() || gpuGraphData.numEdges() == 0)
        return passes;

    const auto narrowestShaft = 2.0f * extents._smallestEdgeSize * pixelScale._minimum;
    const auto widestShaft = 2.0f * extents._largestEdgeSize * pixelScale._maximum;

    // An arrow's head is twice as wide as its shaft
    const auto widening = extents._hasArrowHeads ? 2.0f : 1.0f;

    const auto anyTracedEdges = widestShaft * widening >= thresholds._minimumApproximateTracedEdgeWidth;
    const auto anySplattedEdges = narrowestShaft * widening < thresholds._minimumApproximateTracedEdgeWidth;

    passes._approximateTracedEdges = anyTracedEdges && narrowestShaft < thresholds._minimumTracedEdgeWidth;
    passes._tracedEdges = anyTracedEdges && widestShaft >= thresholds._minimumTracedEdgeWidth;
    passes._fastSplattedEdges = anySplattedEdges && narrowestShaft < thresholds._minimumSplattedEdgeWidth;

    // The pixels at an approximate traced edge's boundaries are drawn by their coverage
    passes._splattedEdges = (anySplattedEdges && widestShaft >= thresholds._minimumSplattedEdgeWidth) ||
        passes._approximateTracedEdges;

    return passes;
}

void GraphRendererCore::renderEdges(GPUGraphData& gpuGraphData, const DetailPasses& passes)
{
    // Both the approximate traced build and renderSplats read the fit
    if(passes._splattedEdges)
        fitEdgeShading(gpuGraphData);

    const bool culled = passes._tracedEdges && cullHiddenEdgeStretches(gpuGraphData);

    renderToLayer(gpuGraphData);

    if(passes._approximateTracedEdges)
    {
        auto& approximateTracedShader = _elementShaders._approximateTracedEdges;
        approximateTracedShader.bind();
        _approximateEdgeShading.bind(ElementShaders::glUnit(ElementShaders::Unit::ApproximateShadingThirds),
            approximateTracedShader);
        renderEdgesWithShader(approximateTracedShader, gpuGraphData);
        _approximateEdgeShading.release(ElementShaders::glUnit(ElementShaders::Unit::ApproximateShadingThirds));
    }

    if(passes._tracedEdges)
    {
        renderEdgesWithShader(_elementShaders._tracedEdges, gpuGraphData,
            culled ? CullingResults::VisibleRangesAndPrepassHits : CullingResults::None);
    }
}

// What is too narrow to trace is drawn by how much of each pixel it covers,
// over everything else in the layer and in any order
void GraphRendererCore::renderSplats(GPUGraphData& gpuGraphData, const DetailPasses& passes)
{
    if(!passes.anySplatted())
        return;

    _splatCompositor.beginNearestNodes();

    if(passes._splattedNodes)
    {
        renderNodesWithShader(_elementShaders._nodeDepths, gpuGraphData, "asPoints"_yes);
        _splatCompositor.beginFarthestNodes();
        renderNodesWithShader(_elementShaders._nodeDepths, gpuGraphData, "asPoints"_yes);
        _splatCompositor.beginNodeOcclusion();
        renderNodesWithShader(_elementShaders._nodeOcclusion, gpuGraphData, "asPoints"_yes);
    }

    _splatCompositor.begin();

    if(passes._splattedNodes)
        renderNodesWithShader(_elementShaders._splattedNodes, gpuGraphData, "asPoints"_yes);

    if(passes._fastSplattedEdges)
        renderEdgesWithShader(_elementShaders._fastSplattedEdges, gpuGraphData);

    if(passes._splattedEdges)
    {
        auto& splattedShader = _elementShaders._splattedEdges;
        splattedShader.bind();
        _approximateEdgeShading.bind(ElementShaders::glUnit(ElementShaders::Unit::ApproximateShadingThirds),
            splattedShader);
        renderEdgesWithShader(splattedShader, gpuGraphData);
        _approximateEdgeShading.release(ElementShaders::glUnit(ElementShaders::Unit::ApproximateShadingThirds));
    }

    _splatCompositor.resolveInto(gpuGraphData._fbo);
}

void GraphRendererCore::renderText(GPUGraphData& gpuGraphData, const PixelScaleRange& pixelScale)
{
    // Text whose x-height is shorter than this on screen can't be read, so isn't drawn...
    constexpr float IllegibleTextPixels = 3.5f;

    // ...and it fades in until it is this tall, rather than popping in
    constexpr float LegibleTextPixels = 4.0f;

    if(gpuGraphData.numGlyphs() == 0)
        return;

    if(gpuGraphData._tallestTextHeight * pixelScale._maximum <= IllegibleTextPixels)
        return;

    renderToLayer(gpuGraphData);

    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    _textShader.bind();
    gpuGraphData._textVBO.bind();

     // Bind SDF textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, sdfTexture());

    // Set to linear filtering for SDF text
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    _textShader.setUniformValue("tex", 0);

    bindComponentDataTexture(GL_TEXTURE0 + 1, _textShader);
    bindNodeGeometryTexture(GL_TEXTURE0 + 2, _textShader);

    _textShader.setUniformValue("viewportHeight", static_cast<float>(_height));
    _textShader.setUniformValue("illegibleTextPixels", IllegibleTextPixels);
    _textShader.setUniformValue("legibleTextPixels", LegibleTextPixels);

    gpuGraphData._rectangle.vertexArrayObject()->bind();
    glDrawElementsInstanced(GL_TRIANGLES, Primitive::Rectangle::glIndexCount(),
        GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(gpuGraphData.numGlyphs()));
    gpuGraphData._rectangle.vertexArrayObject()->release();

    _nodeGeometry.release(GL_TEXTURE0 + 2);
    _componentDataTexels.release(GL_TEXTURE0 + 1);

    gpuGraphData._textVBO.release();
    _textShader.release();
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
}

GPUGraphData* GraphRendererCore::gpuGraphDataForAlpha(float componentAlpha, float unhighlightAlpha)
{
    for(auto& gpuGraphData : _gpuGraphData)
    {
        // cppcheck-suppress useStlAlgorithm
        if(qFuzzyCompare(gpuGraphData._componentAlpha, componentAlpha) && qFuzzyCompare(gpuGraphData._unhighlightAlpha, unhighlightAlpha))
            return &gpuGraphData;
    }

    for(auto& gpuGraphData : _gpuGraphData)
    {
        if(gpuGraphData.unused())
        { // cppcheck-suppress useStlAlgorithm
            gpuGraphData._componentAlpha = componentAlpha;
            gpuGraphData._unhighlightAlpha = unhighlightAlpha;
            return &gpuGraphData;
        }
    }

    qWarning() << "Not enough gpuGraphData instances for" << componentAlpha << unhighlightAlpha;
    for(const auto& gpuGraphData : _gpuGraphData)
        qWarning() << "  " << gpuGraphData._componentAlpha << gpuGraphData._unhighlightAlpha;

    return nullptr;
}

GPUGraphData* GraphRendererCore::gpuGraphDataForOverlay(float alpha)
{
    for(auto& gpuGraphData : _gpuGraphData)
    {
        // cppcheck-suppress useStlAlgorithm
        if(gpuGraphData._isOverlay && qFuzzyCompare(gpuGraphData._componentAlpha, alpha))
            return &gpuGraphData;
    }

    for(auto& gpuGraphData : _gpuGraphData)
    {
        if(gpuGraphData.unused())
        { // cppcheck-suppress useStlAlgorithm
            gpuGraphData._componentAlpha = alpha;
            gpuGraphData._unhighlightAlpha = 1.0f;
            gpuGraphData._isOverlay = true;
            return &gpuGraphData;
        }
    }

    qWarning() << "Not enough gpuGraphData instances for overlay" << alpha;
    for(const auto& gpuGraphData : _gpuGraphData)
    {
        qWarning() << "  " << gpuGraphData._componentAlpha <<
            gpuGraphData._unhighlightAlpha << gpuGraphData._isOverlay;
    }

    return nullptr;
}

void GraphRendererCore::resetGPUGraphData()
{
    for(auto& gpuGraphData : _gpuGraphData)
        gpuGraphData.reset();
}

void GraphRendererCore::uploadGPUGraphData()
{
    for(auto& gpuGraphData : _gpuGraphData)
    {
        if(gpuGraphData.alpha() > 0.0f)
            gpuGraphData.upload();
    }
}

void GraphRendererCore::resetGPUComponentData()
{
    _componentData.clear();
}

void GraphRendererCore::appendGPUComponentData(const QMatrix4x4& modelViewMatrix,
    const QMatrix4x4& projectionMatrix, float distance, float lightScale, float radius)
{
    _componentData.push_back({modelViewMatrix, projectionMatrix, distance, lightScale, radius});
}

void GraphRendererCore::uploadGPUComponentData()
{
    std::vector<GLfloat> texels;
    texels.reserve(_componentData.size() * (16 + 16 + 4));

    for(const auto& componentData : _componentData)
    {
        const auto* modelViewMatrix = componentData._modelViewMatrix.constData();
        const auto* projectionMatrix = componentData._projectionMatrix.constData();

        texels.insert(texels.end(), modelViewMatrix, modelViewMatrix + 16);
        texels.insert(texels.end(), projectionMatrix, projectionMatrix + 16);
        texels.insert(texels.end(), {componentData._distance, componentData._lightScale, componentData._radius, 0.0f});
    }

    _componentDataTexels.upload(texels);
}

Shading GraphRendererCore::shading() const
{
    return _shading;
}

void GraphRendererCore::setShading(Shading shading)
{
    if(shading != _shading)
        _shading = shading;
}

void GraphRendererCore::resizeScreenQuad(int width, int height)
{
    auto w = static_cast<GLfloat>(width);
    auto h = static_cast<GLfloat>(height);
    GLfloat quadData[] =
    {
        0, 0,
        w, 0,
        w, h,

        w, h,
        0, h,
        0, 0,
    };

    _screenQuadDataBuffer.bind();
    _screenQuadDataBuffer.allocate(static_cast<void*>(quadData), static_cast<int>(sizeof(quadData)));
    _screenQuadDataBuffer.release();
}

bool GraphRendererCore::resize(int width, int height)
{
    _width = width;
    _height = height;

    bool FBOcomplete = false;

    if(width > 0 && height > 0)
    {
        setupTexture(this, _depthTexture, width, height, GL_DEPTH_COMPONENT32F);
        _depthPrepass.resize(width, height);

        if(!_gpuGraphData.empty())
        {
            FBOcomplete = _composite.resize(width, height) && _compositeElements.resize(width, height) &&
                _smaaAntialiasing.resize(width, height) && _smoothedSelection.resize(width, height) &&
                _splatCompositor.resize(width, height, _depthTexture);

            for(auto& gpuGraphData : _gpuGraphData)
                FBOcomplete = FBOcomplete && gpuGraphData.prepareRenderBuffers(width, height, _depthTexture);
        }
        else
            FBOcomplete = false;
    }

    return FBOcomplete;
}

void GraphRendererCore::prepareSelectionMarkerVAO()
{
    _selectionMarkerDataVAO.create();

    _selectionMarkerDataVAO.bind();
    _selectionMarkerShader.bind();

    _selectionMarkerDataBuffer.create();
    _selectionMarkerDataBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    _selectionMarkerDataBuffer.bind();

    _selectionMarkerShader.enableAttributeArray("position");
    _selectionMarkerShader.enableAttributeArray("color");
    _selectionMarkerShader.disableAttributeArray("texCoord");
    _selectionMarkerShader.setAttributeBuffer("position", GL_FLOAT, 0, 2, 5 * sizeof(GLfloat));
    _selectionMarkerShader.setAttributeBuffer("color", GL_FLOAT, 2 * sizeof(GLfloat), 3, 5 * sizeof(GLfloat));

    _selectionMarkerDataBuffer.release();
    _selectionMarkerDataVAO.release();
    _selectionMarkerShader.release();
}

void GraphRendererCore::prepareScreenQuad()
{
    if(!_screenQuadVAO.isCreated())
        _screenQuadVAO.create();

    _screenQuadVAO.bind();

    if(!_screenQuadDataBuffer.isCreated())
        _screenQuadDataBuffer.create();

    _screenQuadDataBuffer.bind();
    _screenQuadDataBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);

    _sdfShader.bind();
    _sdfShader.enableAttributeArray("position");
    _sdfShader.setAttributeBuffer("position", GL_FLOAT, 0, 2, 2 * sizeof(GLfloat));
    _sdfShader.release();

    _screenQuadDataBuffer.release();
    _screenQuadVAO.release();
}

GraphRendererCore::PixelScaleRange GraphRendererCore::pixelScaleRange() const
{
    PixelScaleRange range{._minimum = std::numeric_limits<float>::max(), ._maximum = 0.0f};
    const auto halfHeight = static_cast<float>(_height) * 0.5f;

    for(const auto& componentData : _componentData)
    {
        const auto& projectionMatrix = componentData._projectionMatrix;
        const auto modelViewProjectionMatrix = projectionMatrix * componentData._modelViewMatrix;
        const auto focalLengthPixels = projectionMatrix(1, 1) * halfHeight;

        // Clip w is linear in position, so over a box it is at its extremes at the corners
        for(const auto& corner : _nodeGeometry.bounds().corners())
        {
            const auto w = (modelViewProjectionMatrix * QVector4D(corner, 1.0f)).w();

            // A corner at or behind the eye leaves no bound on how large anything is
            // The farthest corner still bounds how small
            if(w <= 0.0f)
            {
                range._maximum = std::numeric_limits<float>::infinity();
                continue;
            }

            range._minimum = std::min(range._minimum, focalLengthPixels / w);
            range._maximum = std::max(range._maximum, focalLengthPixels / w);
        }
    }

    return range;
}

std::vector<size_t> GraphRendererCore::gpuGraphDataRenderOrder() const
{
    std::vector<size_t> renderOrder;
    renderOrder.reserve(_gpuGraphData.size());

    for(size_t i = 0; i < _gpuGraphData.size(); i++)
        renderOrder.push_back(i);

    std::ranges::sort(renderOrder, [this](auto a, auto b)
    {
        const auto& ggda = _gpuGraphData.at(a);
        const auto& ggdb = _gpuGraphData.at(b);

        if(ggda._isOverlay != ggdb._isOverlay)
            return ggdb._isOverlay;

        if(ggda._componentAlpha == ggdb._componentAlpha)
            return ggda._unhighlightAlpha > ggdb._unhighlightAlpha;

        return ggda._componentAlpha > ggdb._componentAlpha;
    });

    // Filter out any invisible layers
    std::erase_if(renderOrder, [this](auto index)
    {
        return _gpuGraphData.at(index).invisible();
    });

    return renderOrder;
}

void GraphRendererCore::renderGraph()
{
    _elementShaders.setFrameUniforms(
    {
        ._viewportSize = QVector2D(static_cast<float>(_width), static_cast<float>(_height)),
        ._flatness = shading() == Shading::Flat ? 1.0f : 0.0f,
        ._outlineColor = Document::contrastingColorForBackground(),
        ._widestEdgeSize = std::ranges::max(_gpuGraphData | std::views::transform(
            [](const auto& gpuGraphData) { return gpuGraphData._extents._largestEdgeSize; })),
        ._thresholds = _detailThresholds,
    });

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glDisable(GL_BLEND);
    glDisable(GL_DITHER);

    for(auto& gpuGraphData : _gpuGraphData)
        gpuGraphData.clearFramebuffer();

    const auto pixelScale = pixelScaleRange();

    for(auto i : gpuGraphDataRenderOrder())
    {
        auto& gpuGraphData = _gpuGraphData.at(i);

        // Clear the depth buffer, but only when we're about to render graph elements
        // that are found, so that subsequent render passes of not highlighted elements
        // use the existing depth information
        if(gpuGraphData._unhighlightAlpha >= 1.0f)
            gpuGraphData.clearDepthbuffer();

        gpuGraphData.clearFramebuffer(GL_COLOR_BUFFER_BIT);
        renderToLayer(gpuGraphData);

        const auto passes = detailPasses(gpuGraphData, pixelScale);

        if(passes._tracedNodes)
            renderNodesWithShader(_elementShaders._nodes, gpuGraphData);

        renderEdges(gpuGraphData, passes);
        renderSplats(gpuGraphData, passes);
        renderText(gpuGraphData, pixelScale);
    }
}

void GraphRendererCore::render2D(QRect selectionRect)
{
    const auto& renderOrder = gpuGraphDataRenderOrder();
    auto index = !renderOrder.empty() ? renderOrder.front() : 0;
    auto& gpuGraphData = _gpuGraphData.at(index);

    if(gpuGraphData.unused())
        return;

    glBindFramebuffer(GL_FRAMEBUFFER, gpuGraphData._fbo);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glViewport(0, 0, _width, _height);

    QMatrix4x4 m;
    m.ortho(0, static_cast<float>(_width), 0, static_cast<float>(_height), -1.0f, 1.0f);

    if(!selectionRect.isNull())
    {
        const QColor color(Qt::white);

        QRect r;
        r.setLeft(selectionRect.left());
        r.setRight(selectionRect.right());
        r.setTop(_height - selectionRect.top());
        r.setBottom(_height - selectionRect.bottom());

        std::vector<GLfloat> quadData =
        {
            static_cast<GLfloat>(r.left()), static_cast<GLfloat>(r.bottom()),
            static_cast<GLfloat>(color.redF()), static_cast<GLfloat>(color.blueF()), static_cast<GLfloat>(color.greenF()),
            static_cast<GLfloat>(r.right()), static_cast<GLfloat>(r.bottom()),
            static_cast<GLfloat>(color.redF()), static_cast<GLfloat>(color.blueF()), static_cast<GLfloat>(color.greenF()),
            static_cast<GLfloat>(r.right()), static_cast<GLfloat>(r.top()),
            static_cast<GLfloat>(color.redF()), static_cast<GLfloat>(color.blueF()), static_cast<GLfloat>(color.greenF()),

            static_cast<GLfloat>(r.right()), static_cast<GLfloat>(r.top()),
            static_cast<GLfloat>(color.redF()), static_cast<GLfloat>(color.blueF()), static_cast<GLfloat>(color.greenF()),
            static_cast<GLfloat>(r.left()),  static_cast<GLfloat>(r.top()),
            static_cast<GLfloat>(color.redF()), static_cast<GLfloat>(color.blueF()), static_cast<GLfloat>(color.greenF()),
            static_cast<GLfloat>(r.left()),  static_cast<GLfloat>(r.bottom()),
            static_cast<GLfloat>(color.redF()), static_cast<GLfloat>(color.blueF()), static_cast<GLfloat>(color.greenF()),
        };

        GLenum drawBuffers[] = {GL_NONE, GL_NONE, GL_NONE, GL_COLOR_ATTACHMENT3};
        glDrawBuffers(4, static_cast<GLenum*>(drawBuffers));

        _selectionMarkerDataBuffer.bind();
        _selectionMarkerDataBuffer.allocate(quadData.data(), static_cast<int>(quadData.size() * sizeof(GLfloat)));

        _selectionMarkerShader.bind();
        _selectionMarkerShader.setUniformValue("projectionMatrix", m);

        _selectionMarkerDataVAO.bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        _selectionMarkerDataVAO.release();

        _selectionMarkerShader.release();
        _selectionMarkerDataBuffer.release();

        // Force rendering of the selection layer
        gpuGraphData._elementsSelected = true;
    }

    glEnable(GL_DEPTH_TEST);
}

void GraphRendererCore::compositeLayer(QOpenGLShaderProgram& shader, float alpha,
    std::initializer_list<GLuint> textures)
{
    shader.bind();
    shader.setUniformValue("alpha", alpha);
    _screenPass.draw(shader, textures);
}

void GraphRendererCore::compositeElements()
{
    _compositeElements.bindForDrawing();

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_BLEND);

    for(auto i : gpuGraphDataRenderOrder())
    {
        const auto& graphData = _gpuGraphData.at(i);

        if(graphData.hasGraphElements())
        {
            _screenPass.draw(_compositeElementShader,
                {graphData._elementTexture, graphData._subElementTexture});
        }
    }
}

void GraphRendererCore::compositeInto(GLuint framebuffer, Flags<Type> type)
{
    const bool antialiasing = u::pref(u"visuals/enableAntialiasing"_s).toBool();

    glDisable(GL_DEPTH_TEST);

    // The selection's outline is drawn from where the selection is within each
    // pixel, which SMAA estimates from the silhouettes
    if(antialiasing)
    {
        compositeElements();
        _smaaAntialiasing.findSilhouettes(_compositeElements.texture());
    }

    _composite.bindForDrawing();

    auto backgroundColor = u::pref(u"visuals/backgroundColor"_s).value<QColor>();

    glClearColor(static_cast<GLfloat>(backgroundColor.redF()),
        static_cast<GLfloat>(backgroundColor.greenF()),
        static_cast<GLfloat>(backgroundColor.blueF()), 1.0f);

    glClear(GL_COLOR_BUFFER_BIT);

    // Layers hold premultiplied colour, since what covers part of a pixel is blended into them
    glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
    glEnable(GL_BLEND);

    _selectionShader.bind();
    _selectionShader.setUniformValue("highlightColor",
        u::pref(u"visuals/highlightColor"_s).value<QColor>());
    _selectionShader.release();

    _outlineShader.bind();
    _outlineShader.setUniformValue("outlineColor",
        Document::contrastingColorForBackground());
    _outlineShader.release();

    for(auto i : gpuGraphDataRenderOrder())
    {
        const auto& graphData = _gpuGraphData.at(i);

        if(type.test(GraphRendererCore::Type::Color))
        {
            compositeLayer(_screenShader, graphData.alpha(), {graphData._colorTexture});

            if(_shading == Shading::Flat && graphData.hasGraphElements())
                compositeLayer(_outlineShader, graphData.alpha(), {graphData._elementTexture});
        }

        if(type.test(GraphRendererCore::Type::Selection) && graphData._elementsSelected)
        {
            auto selectionTexture = graphData._selectionTexture;

            if(antialiasing)
            {
                _smaaAntialiasing.blend(selectionTexture, _smoothedSelection.framebuffer());
                selectionTexture = _smoothedSelection.texture();

                _composite.bindForDrawing();
                glEnable(GL_BLEND);
            }

            compositeLayer(_selectionShader, graphData.selectionAlpha(), {selectionTexture});
        }
    }

    if(antialiasing)
        _smaaAntialiasing.blend(_composite.texture(), framebuffer);
    else
        _composite.copyTo(framebuffer);
}

QSize GraphRendererCore::renderSdfTexture(const GlyphMap& glyphMap, GLuint texture)
{
    if(glyphMap.images().empty())
        return {0, 0};

    const auto scaleFactor = 4;
    const auto sourceWidth = glyphMap.images().at(0).width();
    const auto sourceHeight = glyphMap.images().at(0).height();
    const auto renderWidth = glyphMap.images().at(0).width() / scaleFactor;
    const auto renderHeight = glyphMap.images().at(0).height() / scaleFactor;
    const auto numImages = glyphMap.images().size();

    glViewport(0, 0, renderWidth, renderHeight);

    QMatrix4x4 m;
    m.ortho(0, static_cast<float>(renderWidth), 0, static_cast<float>(renderHeight), -1.0f, 1.0f);

    _sdfShader.bind();
    _sdfShader.setUniformValue("tex", 0);
    _sdfShader.setUniformValue("projectionMatrix", m);
    _sdfShader.setUniformValue("scaleFactor", static_cast<GLfloat>(scaleFactor));
    _sdfShader.setUniformValue("texSize", QPoint(sourceWidth, sourceHeight));

    resizeScreenQuad(renderWidth, renderHeight);
    _screenQuadDataBuffer.bind();
    _screenQuadVAO.bind();

    glEnable(GL_BLEND);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);

    GLuint fbo = 0u;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA,
        renderWidth, renderHeight, static_cast<GLsizei>(numImages),
        0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLuint sourceTexture;
    glGenTextures(1, &sourceTexture);

    glBindTexture(GL_TEXTURE_2D, sourceTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // The SDF shader reads the coverage of individual source texels, so it must
    // not be given an interpolation of the texels surrounding the sample point
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Render an SDF texture for each source glyph layer
    for(size_t layer = 0; layer < numImages; layer++)
    {
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0, static_cast<GLint>(layer));

        GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, static_cast<GLenum*>(drawBuffers));

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            qWarning() << "FBO incomplete while rendering SDF textures";
            break;
        }

        // Buffer the glyph texture from the source image
        auto image = glyphMap.images().at(layer).flipped().convertToFormat(QImage::Format_RGBA8888);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(),
            0, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());

        glDrawArrays(GL_TRIANGLES, 0, 6);

        if(u::pref(u"debug/saveGlyphMaps"_s).toBool())
        {
            glFinish();

            std::vector<uchar> pixels(static_cast<size_t>(renderWidth * renderHeight * 4));
            glReadPixels(0, 0, renderWidth, renderHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
            QImage sdfImage(pixels.data(), renderWidth, renderHeight, QImage::Format_RGBA8888);
            sdfImage.flip();
            sdfImage.save(u"%1/graphia-SDF%2.png"_s.arg(QDir::tempPath(), QString::number(layer)));
        }
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glDeleteTextures(1, &sourceTexture);

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    _screenQuadVAO.release();
    _screenQuadDataBuffer.release();
    _sdfShader.release();

    glDeleteFramebuffers(1, &fbo);

    glDepthMask(GL_TRUE);

    return {renderWidth, renderHeight};
}
