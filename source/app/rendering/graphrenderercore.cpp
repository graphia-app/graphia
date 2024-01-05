/* Copyright © 2013-2023 Graphia Technologies Ltd.
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
#include "shared/rendering/multisamples.h"

#include "glyphmap.h"
#include "shadertools.h"

#include "ui/document.h"

#include <QColor>
#include <QDir>

using namespace Qt::Literals::StringLiterals;

template<typename T>
void setupTexture(T t, GLuint& texture, int width, int height, GLint format, int numMultiSamples)
{
    if(texture == 0)
        t->glGenTextures(1, &texture);
    t->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
    t->glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
        numMultiSamples, format, width, height, GL_FALSE);
    t->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
}

GPUGraphData::GPUGraphData()
{
    resolveOpenGLFunctions();
}

void GPUGraphData::initialise(QOpenGLShaderProgram& nodesShader,
                              QOpenGLShaderProgram& edgesShader,
                              QOpenGLShaderProgram& textShader)
{
    _sphere.setRadius(1.0f);
    _sphere.setRings(16);
    _sphere.setSlices(16);
    _sphere.create(nodesShader);

    _arrow.setRadius(1.0f);
    _arrow.setLength(1.0f);
    _arrow.setSlices(8);
    _arrow.create(edgesShader);

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
    shader.enableAttributeArray("basePosition");
    shader.enableAttributeArray("glyphOffset");
    shader.enableAttributeArray("glyphSize");
    shader.enableAttributeArray("glyphScale");
    shader.enableAttributeArray("color");

    glVertexAttribIPointer(static_cast<GLuint>(shader.attributeLocation("component")),              1, GL_INT,  sizeof(GlyphData),
                             reinterpret_cast<const void*>(offsetof(GlyphData, _component))); // NOLINT
    shader.setAttributeBuffer("textureCoord",    GL_FLOAT, offsetof(GlyphData, _textureCoord),      2,          sizeof(GlyphData));
    glVertexAttribIPointer(static_cast<GLuint>(shader.attributeLocation("textureLayer")),           1, GL_INT,  sizeof(GlyphData),
                             reinterpret_cast<const void*>(offsetof(GlyphData, _textureLayer))); // NOLINT
    shader.setAttributeBuffer("basePosition",    GL_FLOAT, offsetof(GlyphData, _basePosition),      3,          sizeof(GlyphData));
    shader.setAttributeBuffer("glyphOffset",     GL_FLOAT, offsetof(GlyphData, _glyphOffset),       2,          sizeof(GlyphData));
    shader.setAttributeBuffer("glyphSize",       GL_FLOAT, offsetof(GlyphData, _glyphSize),         2,          sizeof(GlyphData));
    shader.setAttributeBuffer("glyphScale",      GL_FLOAT, offsetof(GlyphData, _glyphScale),        1,          sizeof(GlyphData));
    shader.setAttributeBuffer("color",           GL_FLOAT, offsetof(GlyphData, _color),             3,          sizeof(GlyphData));
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("component")),               1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("textureCoord")),            1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("textureLayer")),            1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("basePosition")),            1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("glyphOffset")),             1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("glyphSize")),               1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("glyphScale")),              1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("color")),                   1);

    _textVBO.release();
    shader.release();
    _rectangle.vertexArrayObject()->release();
}

void GPUGraphData::prepareNodeVAO(QOpenGLShaderProgram& shader)
{
    _sphere.vertexArrayObject()->bind();
    shader.bind();

    _nodeVBO.bind();
    shader.enableAttributeArray("nodePosition");
    shader.enableAttributeArray("component");
    shader.enableAttributeArray("size");
    shader.enableAttributeArray("outerColor");
    shader.enableAttributeArray("innerColor");
    shader.enableAttributeArray("selected");
    shader.setAttributeBuffer("nodePosition", GL_FLOAT, offsetof(NodeData, _position),      3,          sizeof(NodeData));
    glVertexAttribIPointer(static_cast<GLuint>(shader.attributeLocation("component")),      1, GL_INT,  sizeof(NodeData),
                          reinterpret_cast<const void*>(offsetof(NodeData, _component))); // NOLINT
    shader.setAttributeBuffer("size",         GL_FLOAT, offsetof(NodeData, _size),          1,          sizeof(NodeData));
    shader.setAttributeBuffer("outerColor",   GL_FLOAT, offsetof(NodeData, _outerColor),    3,          sizeof(NodeData));
    shader.setAttributeBuffer("innerColor",   GL_FLOAT, offsetof(NodeData, _innerColor),    3,          sizeof(NodeData));
    shader.setAttributeBuffer("selected",     GL_FLOAT, offsetof(NodeData, _selected),      1,          sizeof(NodeData));
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("nodePosition")),    1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("component")),       1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("size")),            1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("innerColor")),      1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("outerColor")),      1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("selected")),        1);
    _nodeVBO.release();

    shader.release();
    _sphere.vertexArrayObject()->release();
}

void GPUGraphData::prepareEdgeVAO(QOpenGLShaderProgram& shader)
{
    _arrow.vertexArrayObject()->bind();
    shader.bind();

    _edgeVBO.bind();
    shader.enableAttributeArray("sourcePosition");
    shader.enableAttributeArray("targetPosition");
    shader.enableAttributeArray("sourceSize");
    shader.enableAttributeArray("targetSize");
    shader.enableAttributeArray("edgeType");
    shader.enableAttributeArray("component");
    shader.enableAttributeArray("size");
    shader.enableAttributeArray("outerColor");
    shader.enableAttributeArray("innerColor");
    shader.enableAttributeArray("selected");
    shader.setAttributeBuffer("sourcePosition", GL_FLOAT, offsetof(EdgeData, _sourcePosition),  3,          sizeof(EdgeData));
    shader.setAttributeBuffer("targetPosition", GL_FLOAT, offsetof(EdgeData, _targetPosition),  3,          sizeof(EdgeData));
    shader.setAttributeBuffer("sourceSize",     GL_FLOAT, offsetof(EdgeData, _sourceSize),      1,          sizeof(EdgeData));
    shader.setAttributeBuffer("targetSize",     GL_FLOAT, offsetof(EdgeData, _targetSize),      1,          sizeof(EdgeData));
    glVertexAttribIPointer(static_cast<GLuint>(shader.attributeLocation("edgeType")),           1, GL_INT,  sizeof(EdgeData),
                            reinterpret_cast<const void*>(offsetof(EdgeData, _edgeType))); // NOLINT
    glVertexAttribIPointer(static_cast<GLuint>(shader.attributeLocation("component")),          1, GL_INT,  sizeof(EdgeData),
                           reinterpret_cast<const void*>(offsetof(EdgeData, _component))); // NOLINT
    shader.setAttributeBuffer("size",           GL_FLOAT, offsetof(EdgeData, _size),            1,          sizeof(EdgeData));
    shader.setAttributeBuffer("outerColor",     GL_FLOAT, offsetof(EdgeData, _outerColor),      3,          sizeof(EdgeData));
    shader.setAttributeBuffer("innerColor",     GL_FLOAT, offsetof(EdgeData, _innerColor),      3,          sizeof(EdgeData));
    shader.setAttributeBuffer("selected",       GL_FLOAT, offsetof(EdgeData, _selected),        1,          sizeof(EdgeData));
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("sourcePosition")),      1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("targetPosition")),      1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("sourceSize")),          1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("targetSize")),          1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("edgeType")),            1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("component")),           1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("size")),                1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("outerColor")),          1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("innerColor")),          1);
    glVertexAttribDivisor(static_cast<GLuint>(shader.attributeLocation("selected")),            1);
    _edgeVBO.release();

    shader.release();
    _arrow.vertexArrayObject()->release();
}

bool GPUGraphData::prepareRenderBuffers(int width, int height, GLuint depthTexture, GLint numMultiSamples)
{
    setupTexture(this, _colorTexture,     width, height, GL_RGBA,   numMultiSamples);
    setupTexture(this, _elementTexture,   width, height, GL_RG32F,  numMultiSamples);
    setupTexture(this, _selectionTexture, width, height, GL_RGBA,   numMultiSamples);

    if(_fbo == 0)
        glGenFramebuffers(1, &_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, _colorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, _elementTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D_MULTISAMPLE, _selectionTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D_MULTISAMPLE, depthTexture, 0);

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
    _nodeData.clear();
    _edgeData.clear();
    _glyphData.clear();
}

void GPUGraphData::clearFramebuffer(GLbitfield buffers)
{
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, static_cast<GLenum*>(drawBuffers));

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(buffers);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GPUGraphData::clearDepthbuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_NONE, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, static_cast<GLenum*>(drawBuffers));

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GPUGraphData::drawToFramebuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo);

    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, static_cast<GLenum*>(drawBuffers));
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

void GPUGraphData::copyState(const GPUGraphData& gpuGraphData,
    QOpenGLShaderProgram& nodesShader,
    QOpenGLShaderProgram& edgesShader,
    QOpenGLShaderProgram& textShader)
{
    _componentAlpha = gpuGraphData._componentAlpha;
    _unhighlightAlpha = gpuGraphData._unhighlightAlpha;
    _isOverlay = gpuGraphData._isOverlay;
    _nodeData = gpuGraphData._nodeData;
    _glyphData = gpuGraphData._glyphData;
    _edgeData = gpuGraphData._edgeData;
    _elementsSelected = gpuGraphData._elementsSelected;

    // Cause VBO to be recreated
    _fbo = 0;
    _colorTexture = 0;
    _elementTexture = 0;
    _selectionTexture = 0;

    _edgeVBO.destroy();
    _nodeVBO.destroy();
    _textVBO.destroy();

    initialise(nodesShader, edgesShader, textShader);
}

GraphRendererCore::GraphRendererCore() :
    _numMultiSamples(multisamples())
{
    resolveOpenGLFunctions();

    GLint maxSamples = 0;

    // We need to use the minimum of GL_MAX_COLOR_TEXTURE_SAMPLES,
    // and GL_MAX_DEPTH_TEXTURE_SAMPLES for our sample count as we
    // use the textures in an FBO, where the sample counts for
    // each attachment must be the same

    glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &maxSamples);
    _numMultiSamples = std::min(maxSamples, _numMultiSamples);
    glGetIntegerv(GL_MAX_DEPTH_TEXTURE_SAMPLES, &maxSamples);
    _numMultiSamples = std::min(maxSamples, _numMultiSamples);

    ShaderTools::loadShaderProgram(_screenShader, u":/shaders/screen.vert"_s, u":/shaders/screen.frag"_s);
    ShaderTools::loadShaderProgram(_outlineShader, u":/shaders/screen.vert"_s, u":/shaders/outline.frag"_s);
    ShaderTools::loadShaderProgram(_selectionShader, u":/shaders/screen.vert"_s, u":/shaders/selection.frag"_s);

    ShaderTools::loadShaderProgram(_nodesShader, u":/shaders/instancednodes.vert"_s, u":/shaders/nodecolorads.frag"_s);
    ShaderTools::loadShaderProgram(_edgesShader, u":/shaders/instancededges.vert"_s, u":/shaders/edgecolorads.frag"_s);

    ShaderTools::loadShaderProgram(_selectionMarkerShader, u":/shaders/2d.vert"_s, u":/shaders/selectionMarker.frag"_s);

    ShaderTools::loadShaderProgram(_sdfShader, u":/shaders/screen.vert"_s, u":/shaders/sdf.frag"_s);
    ShaderTools::loadShaderProgram(_textShader, u":/shaders/textrender.vert"_s, u":/shaders/textrender.frag"_s);

    for(auto& gpuGraphData : _gpuGraphData)
        gpuGraphData.initialise(_nodesShader, _edgesShader, _textShader);

    prepareComponentDataTexture();
    prepareSelectionMarkerVAO();
    prepareScreenQuad();
}

GraphRendererCore::~GraphRendererCore()
{
    if(_componentDataTexture != 0)
    {
        glDeleteTextures(1, &_componentDataTexture);
        _componentDataTexture = 0;
    }

    if(_depthTexture != 0)
    {
        glDeleteTextures(1, &_depthTexture);
        _depthTexture = 0;
    }
}

static void setShaderLightingParameters(QOpenGLShaderProgram& program)
{
    struct Light
    {
        Light() = default;
        Light(const QVector3D& _position, QColor _color) :
            position(_position), color(_color)
        {}

        QVector3D position;
        QColor color;
    };

    std::vector<Light> lights;
    lights.push_back({{-0.707f,  0.0f,    0.707f}, {100, 100, 100}});
    lights.push_back({{ 0.0f,    0.0f,    1.0f  }, {150, 150, 150}});
    lights.push_back({{ 0.707f, -0.707f,  0.0f  }, {100, 100, 100}});

    auto numberOfLights = lights.size();

    program.setUniformValue("numberOfLights", static_cast<GLuint>(numberOfLights));

    for(size_t i = 0; i < numberOfLights; i++)
    {
        QByteArray positionId = u"lights[%1].position"_s.arg(i).toLatin1();
        program.setUniformValue(positionId.data(), lights.at(i).position);

        QByteArray colorId = u"lights[%1].color"_s.arg(i).toLatin1();
        program.setUniformValue(colorId.data(), lights.at(i).color);
    }

    program.setUniformValue("material.ks", QVector3D(1.0f, 1.0f, 1.0f));
    program.setUniformValue("material.ka", QVector3D(0.02f, 0.02f, 0.02f));
    program.setUniformValue("material.shininess", 50.0f);
}

void GraphRendererCore::bindComponentDataTexture(GLenum textureUnit, QOpenGLShaderProgram& shader)
{
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, _componentDataTexture);
    shader.setUniformValue("componentDataElementSize",
        static_cast<int>(_componentDataElementSize));
    shader.setUniformValue("componentDataTextureMaxDimension",
        static_cast<int>(_componentDataMaxTextureSize));
    shader.setUniformValue("componentData", textureUnit - GL_TEXTURE0);
}

void GraphRendererCore::renderNodes(GPUGraphData& gpuGraphData)
{
    if(gpuGraphData.numNodes() == 0)
        return;

    _nodesShader.bind();
    setShaderLightingParameters(_nodesShader);

    gpuGraphData._nodeVBO.bind();

    bindComponentDataTexture(GL_TEXTURE0, _nodesShader);

    _nodesShader.setUniformValue("flatness", shading() == Shading::Flat ? 1.0f : 0.0f);

    gpuGraphData._sphere.vertexArrayObject()->bind();
    glDrawElementsInstanced(GL_TRIANGLES, gpuGraphData._sphere.glIndexCount(),
        GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(gpuGraphData.numNodes()));
    gpuGraphData._sphere.vertexArrayObject()->release();

    glBindTexture(GL_TEXTURE_2D, 0);
    gpuGraphData._nodeVBO.release();
    _nodesShader.release();
}

void GraphRendererCore::renderEdges(GPUGraphData& gpuGraphData)
{
    if(gpuGraphData.numEdges() == 0)
        return;

    _edgesShader.bind();
    setShaderLightingParameters(_edgesShader);

    gpuGraphData._edgeVBO.bind();

    bindComponentDataTexture(GL_TEXTURE0, _edgesShader);

    _edgesShader.setUniformValue("flatness", shading() == Shading::Flat ? 1.0f : 0.0f);

    gpuGraphData._arrow.vertexArrayObject()->bind();
    glDrawElementsInstanced(GL_TRIANGLES, gpuGraphData._arrow.glIndexCount(),
        GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(gpuGraphData.numEdges()));
    gpuGraphData._arrow.vertexArrayObject()->release();

    glBindTexture(GL_TEXTURE_2D, 0);
    gpuGraphData._edgeVBO.release();
    _edgesShader.release();
}

void GraphRendererCore::renderText(GPUGraphData& gpuGraphData)
{
    if(gpuGraphData.numGlyphs() == 0)
        return;

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

    gpuGraphData._rectangle.vertexArrayObject()->bind();
    glDrawElementsInstanced(GL_TRIANGLES, Primitive::Rectangle::glIndexCount(),
        GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(gpuGraphData.numGlyphs()));
    gpuGraphData._rectangle.vertexArrayObject()->release();

    glBindTexture(GL_TEXTURE_2D, 0);

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
        if(qFuzzyCompare(gpuGraphData._componentAlpha, componentAlpha) && qFuzzyCompare(gpuGraphData._unhighlightAlpha, unhighlightAlpha))
            return &gpuGraphData;
    }

    for(auto& gpuGraphData : _gpuGraphData)
    {
        if(gpuGraphData.unused())
        {
            gpuGraphData._componentAlpha = componentAlpha;
            gpuGraphData._unhighlightAlpha = unhighlightAlpha;
            return &gpuGraphData;
        }
    }

    qWarning() << "Not enough gpuGraphData instances for" << componentAlpha << unhighlightAlpha;
    for(auto& gpuGraphData : _gpuGraphData)
        qWarning() << "  " << gpuGraphData._componentAlpha << gpuGraphData._unhighlightAlpha;

    return nullptr;
}

GPUGraphData* GraphRendererCore::gpuGraphDataForOverlay(float alpha)
{
    for(auto& gpuGraphData : _gpuGraphData)
    {
        if(gpuGraphData._isOverlay && qFuzzyCompare(gpuGraphData._componentAlpha, alpha))
            return &gpuGraphData;
    }

    for(auto& gpuGraphData : _gpuGraphData)
    {
        if(gpuGraphData.unused())
        {
            gpuGraphData._componentAlpha = alpha;
            gpuGraphData._unhighlightAlpha = 1.0f;
            gpuGraphData._isOverlay = true;
            return &gpuGraphData;
        }
    }

    qWarning() << "Not enough gpuGraphData instances for overlay" << alpha;
    for(auto& gpuGraphData : _gpuGraphData)
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
    _componentDataElementSize = 0;
}

void GraphRendererCore::appendGPUComponentData(const QMatrix4x4& modelViewMatrix,
    const QMatrix4x4& projectionMatrix, float distance, float lightScale)
{
    std::vector<double> componentDataElement;
    componentDataElement.reserve(34);

    for(int i = 0; i < 16; i++)
        componentDataElement.push_back(modelViewMatrix.data()[i]);

    for(int i = 0; i < 16; i++)
        componentDataElement.push_back(projectionMatrix.data()[i]);

    componentDataElement.push_back(distance);
    componentDataElement.push_back(lightScale);

    _componentData.insert(_componentData.end(), componentDataElement.begin(), componentDataElement.end());

    Q_ASSERT(_componentDataElementSize == 0 || _componentDataElementSize == componentDataElement.size());
    _componentDataElementSize = componentDataElement.size();
}

void GraphRendererCore::uploadGPUComponentData()
{
    glBindTexture(GL_TEXTURE_2D, _componentDataTexture);
    Q_ASSERT(_componentData.size() <= static_cast<size_t>(_componentDataMaxTextureSize * _componentDataMaxTextureSize));
    const GLint textureHeight = (static_cast<GLint>(_componentData.size()) / _componentDataMaxTextureSize) + 1;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F,
        _componentDataMaxTextureSize, textureHeight, 0, GL_RED, GL_FLOAT, _componentData.data());
    glBindTexture(GL_TEXTURE_2D, 0);
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
        setupTexture(this, _depthTexture, width, height, GL_DEPTH_COMPONENT32, _numMultiSamples);

        if(!_gpuGraphData.empty())
        {
            FBOcomplete = true;
            for(auto& gpuGraphData : _gpuGraphData)
            {
                FBOcomplete = FBOcomplete && gpuGraphData.prepareRenderBuffers(width, height,
                    _depthTexture, _numMultiSamples);
            }
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

    _screenQuadDataBuffer.create();
    _screenQuadDataBuffer.bind();
    _screenQuadDataBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);

    for(auto* shader : {&_screenShader, &_outlineShader, &_selectionShader, &_sdfShader})
    {
        shader->bind();

        shader->enableAttributeArray("position");
        shader->setAttributeBuffer("position", GL_FLOAT, 0, 2, 2 * sizeof(GLfloat));

        if(shader->uniformLocation("frameBufferTexture") >= 0)
            shader->setUniformValue("frameBufferTexture", 0);

        if(shader->uniformLocation("multisamples") >= 0)
            shader->setUniformValue("multisamples", _numMultiSamples);

        shader->release();
    }

    _screenQuadDataBuffer.release();
    _screenQuadVAO.release();
}

void GraphRendererCore::prepareComponentDataTexture()
{
    if(_componentDataTexture == 0)
        glGenTextures(1, &_componentDataTexture);

    glBindTexture(GL_TEXTURE_2D, _componentDataTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Effectively disable mipmaps
    glBindTexture(GL_TEXTURE_2D, 0);

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &_componentDataMaxTextureSize);
}

std::vector<size_t> GraphRendererCore::gpuGraphDataRenderOrder() const
{
    std::vector<size_t> renderOrder;
    renderOrder.reserve(_gpuGraphData.size());

    for(size_t i = 0; i < _gpuGraphData.size(); i++)
        renderOrder.push_back(i);

    std::sort(renderOrder.begin(), renderOrder.end(), [this](auto a, auto b)
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
    renderOrder.erase(std::remove_if(renderOrder.begin(), renderOrder.end(),
    [this](auto index)
    {
        return _gpuGraphData.at(index).invisible();
    }), renderOrder.end());

    return renderOrder;
}

void GraphRendererCore::renderGraph()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);
    glDisable(GL_BLEND);
    glDisable(GL_DITHER);

    if(hasSampleShading())
    {
        // Enable per-sample shading, this makes small text look nice
        glEnable(GL_SAMPLE_SHADING_ARB);
    }

    for(auto& gpuGraphData : _gpuGraphData)
        gpuGraphData.clearFramebuffer();

    for(auto i : gpuGraphDataRenderOrder())
    {
        auto& gpuGraphData = _gpuGraphData.at(i);

        // Clear the depth buffer, but only when we're about to render graph elements
        // that are found, so that subsequent render passes of not highlighted elements
        // use the existing depth information
        if(gpuGraphData._unhighlightAlpha >= 1.0f)
            gpuGraphData.clearDepthbuffer();

        if(hasSampleShading())
        {
            // Shade all samples in multi-sampling
            glMinSampleShading(1.0f);
        }

        gpuGraphData.clearFramebuffer(GL_COLOR_BUFFER_BIT);
        gpuGraphData.drawToFramebuffer();

        renderNodes(gpuGraphData);

        if(u::pref(u"visuals/showEdges"_s).toBool())
            renderEdges(gpuGraphData);

        renderText(gpuGraphData);
    }

    glDisable(GL_SAMPLE_SHADING_ARB);
    glDisable(GL_MULTISAMPLE);
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
    glViewport(0, 0, _width, _height);

    QMatrix4x4 m;
    m.ortho(0.0f, static_cast<float>(_width), 0.0f, static_cast<float>(_height), -1.0f, 1.0f);

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

        GLenum drawBuffers[] = {GL_NONE, GL_NONE, GL_COLOR_ATTACHMENT2};
        glDrawBuffers(3, static_cast<GLenum*>(drawBuffers));

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

static void render2DComposite(OpenGLFunctions& f, QOpenGLShaderProgram& shader, GLuint texture,
    float alpha, bool disableAlphaBlending = false)
{
    shader.bind();
    shader.setUniformValue("alpha", alpha);
    shader.setUniformValue("disableAlphaBlending", disableAlphaBlending ? 1 : 0);
    f.glActiveTexture(GL_TEXTURE0);
    f.glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
    f.glDrawArrays(GL_TRIANGLES, 0, 6);
    shader.release();
}

void GraphRendererCore::renderToScreen(Flags<Type> type)
{
    glViewport(0, 0, _width, _height);

    auto backgroundColor = u::pref(u"visuals/backgroundColor"_s).value<QColor>();

    glClearColor(static_cast<GLfloat>(backgroundColor.redF()),
        static_cast<GLfloat>(backgroundColor.greenF()),
        static_cast<GLfloat>(backgroundColor.blueF()), 1.0f);

    glClear(GL_COLOR_BUFFER_BIT);

    glDisable(GL_DEPTH_TEST);

    QMatrix4x4 m;
    m.ortho(0, static_cast<float>(_width), 0, static_cast<float>(_height), -1.0f, 1.0f);

    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
    glEnable(GL_BLEND);

    for(auto* shader : {&_screenShader, &_outlineShader, &_selectionShader})
    {
        shader->bind();
        shader->setUniformValue("projectionMatrix", m);
        shader->setUniformValue("width", _width);
        shader->setUniformValue("height", _height);
        shader->release();
    }

    _selectionShader.bind();
    _selectionShader.setUniformValue("highlightColor",
        u::pref(u"visuals/highlightColor"_s).value<QColor>());
    _selectionShader.release();

    _outlineShader.bind();
    _outlineShader.setUniformValue("outlineColor",
        Document::contrastingColorForBackground());
    _outlineShader.release();

    resizeScreenQuad(_width, _height);
    _screenQuadDataBuffer.bind();
    _screenQuadVAO.bind();

    for(auto i : gpuGraphDataRenderOrder())
    {
        const auto& graphData = _gpuGraphData.at(i);

        if(type.test(GraphRendererCore::Type::Color))
        {
            const bool disableAlphaBlending =
                _shading == Shading::Flat && !graphData._isOverlay;

            render2DComposite(*this, _screenShader,
                graphData._colorTexture,
                graphData.alpha(),
                disableAlphaBlending);

            if(_shading == Shading::Flat && graphData.hasGraphElements())
            {
                render2DComposite(*this, _outlineShader,
                    graphData._elementTexture,
                    graphData.alpha());
            }
        }

        if(type.test(GraphRendererCore::Type::Selection) && graphData._elementsSelected)
        {
            render2DComposite(*this, _selectionShader,
                graphData._selectionTexture,
                graphData.selectionAlpha());
        }
    }

    _screenQuadDataBuffer.release();
    _screenQuadVAO.release();
}

void GraphRendererCore::renderSdfTexture(const GlyphMap& glyphMap, GLuint texture)
{
    if(glyphMap.images().empty())
        return;

    const auto scaleFactor = 4;
    const auto sourceWidth = glyphMap.images().at(0).width();
    const auto sourceHeight = glyphMap.images().at(0).height();
    const auto renderWidth = glyphMap.images().at(0).width() / scaleFactor;
    const auto renderHeight = glyphMap.images().at(0).height() / scaleFactor;
    const auto numImages = glyphMap.images().size();

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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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
        auto image = glyphMap.images().at(layer).mirrored().convertToFormat(QImage::Format_RGBA8888);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(),
            0, GL_RGBA, GL_UNSIGNED_BYTE, image.bits());

        glDrawArrays(GL_TRIANGLES, 0, 6);

        if(u::pref(u"debug/saveGlyphMaps"_s).toBool())
        {
            glFinish();

            std::vector<uchar> pixels(static_cast<size_t>(renderWidth * renderHeight * 4));
            glReadPixels(0, 0, renderWidth, renderHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
            QImage sdfImage(pixels.data(), renderWidth, renderHeight, QImage::Format_RGBA8888);
            sdfImage.mirror();
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
}
