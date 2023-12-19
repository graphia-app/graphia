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

#ifndef GRAPHRENDERERCORE_H
#define GRAPHRENDERERCORE_H

#include "openglfunctions.h"
#include "shading.h"

#include "primitives/arrow.h"
#include "primitives/rectangle.h"
#include "primitives/sphere.h"

#include "shared/utils/flags.h"

#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QRect>
#include <QMatrix4x4>

#include <array>
#include <vector>

class ScreenshotRenderer;
class GlyphMap;

struct GPUGraphData : OpenGLFunctions
{
    GPUGraphData();
    ~GPUGraphData() override;

    void initialise(QOpenGLShaderProgram& nodesShader,
        QOpenGLShaderProgram& edgesShader,
        QOpenGLShaderProgram& textShader);
    void prepareVertexBuffers();
    void prepareNodeVAO(QOpenGLShaderProgram& shader);
    void prepareEdgeVAO(QOpenGLShaderProgram& shader);
    void prepareTextVAO(QOpenGLShaderProgram& shader);

    bool prepareRenderBuffers(int width, int height, GLuint depthTexture, GLint numMultiSamples);
    void copyState(const GPUGraphData &gpuGraphData, QOpenGLShaderProgram &nodesShader,
        QOpenGLShaderProgram &edgesShader, QOpenGLShaderProgram &textShader);

    void reset();
    void clearFramebuffer(GLbitfield buffers = GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    void clearDepthbuffer();
    void drawToFramebuffer();

    void upload();

    size_t numNodes() const;
    size_t numEdges() const;
    size_t numGlyphs() const;

    Primitive::Sphere _sphere;
    Primitive::Arrow _arrow;
    Primitive::Rectangle _rectangle;

    float alpha() const;
    float selectionAlpha() const;

    float componentAlpha() const;
    float unhighlightAlpha() const;

    bool unused() const;
    bool empty() const;
    bool invisible() const;

    bool hasGraphElements() const;

    struct NodeData
    {
        float _position[3] = {0.0f, 0.0f, 0.0f};
        int _component = -1;
        float _size = -1.0f;
        float _outerColor[3] = {0.0f, 0.0f, 0.0f};
        float _innerColor[3] = {0.0f, 0.0f, 0.0f};
        float _selected = 0.0f;
    };

    struct EdgeData
    {
        float _sourcePosition[3] = {0.0f, 0.0f, 0.0f};
        float _targetPosition[3] = {0.0f, 0.0f, 0.0f};
        float _sourceSize = 0.0f;
        float _targetSize = 0.0f;
        int _edgeType = -1;
        int _component = -1;
        float _size = -1.0f;
        float _outerColor[3] = {0.0f, 0.0f, 0.0f};
        float _innerColor[3] = {0.0f, 0.0f, 0.0f};
        float _selected = 0.0f;
    };

    struct GlyphData
    {
        int _component = -1;
        float _textureCoord[2] = {0.0f, 0.0f};
        int _textureLayer = -1;
        float _basePosition[3] = {0.0f, 0.0f, 0.0f};
        float _glyphOffset[2] = {0.0f, 0.0f};
        float _glyphSize[2] = {0.0f, 0.0f};
        float _glyphScale = 0.0f;
        float _color[3] = {0.0f, 0.0f, 0.0f};
    };

    // There are two alpha values so that we can split the alpha blended layers
    // depending on their purpose. The rendering occurs in order based on _componentAlpha,
    // going from opaque to transparent, then resorting to _unhighlightAlpha in the same order,
    // when the values of _componentAlpha match
    float _componentAlpha = 0.0f;
    float _unhighlightAlpha = 0.0f;

    bool _isOverlay = false;

    std::vector<NodeData> _nodeData;
    QOpenGLBuffer _nodeVBO;

    std::vector<GlyphData> _glyphData;
    QOpenGLBuffer _textVBO;

    std::vector<EdgeData> _edgeData;
    QOpenGLBuffer _edgeVBO;

    bool _elementsSelected = false;

    GLuint _fbo = 0;
    GLuint _colorTexture = 0;
    GLuint _elementTexture = 0;
    GLuint _selectionTexture = 0;
};

class GraphRendererCore : public OpenGLFunctions
{
    friend class ScreenshotRenderer;

public:
    GraphRendererCore();
    ~GraphRendererCore() override;

private:
    GLint _numMultiSamples = 0;

    int _width = 0;
    int _height = 0;

    std::array<GPUGraphData, 7> _gpuGraphData;

    QOpenGLShaderProgram _sdfShader;

    QOpenGLShaderProgram _nodesShader;
    QOpenGLShaderProgram _edgesShader;
    QOpenGLShaderProgram _textShader;

    QOpenGLShaderProgram _screenShader;
    QOpenGLShaderProgram _outlineShader;
    QOpenGLShaderProgram _selectionShader;

    GLuint _depthTexture = 0;

    GLuint _componentDataTexture = 0;
    GLint _componentDataMaxTextureSize = 0;
    std::vector<GLfloat> _componentData;
    size_t _componentDataElementSize = 0;

    Shading _shading = Shading::Smooth;

    QOpenGLShaderProgram _selectionMarkerShader;
    QOpenGLBuffer _selectionMarkerDataBuffer;
    QOpenGLVertexArrayObject _selectionMarkerDataVAO;

    QOpenGLVertexArrayObject _screenQuadVAO;
    QOpenGLBuffer _screenQuadDataBuffer;

    void prepareComponentDataTexture();
    void prepareSelectionMarkerVAO();
    void prepareScreenQuad();

    std::vector<size_t> gpuGraphDataRenderOrder() const;

    void bindComponentDataTexture(GLenum textureUnit, QOpenGLShaderProgram& shader);

    void prepare();

protected:
    int width() const { return _width; }
    int height() const { return _height; }

    void resizeScreenQuad(int width, int height);
    bool resize(int width, int height);

    GPUGraphData* gpuGraphDataForAlpha(float componentAlpha, float unhighlightAlpha);
    GPUGraphData* gpuGraphDataForOverlay(float alpha);
    void resetGPUGraphData();
    void uploadGPUGraphData();

    void resetGPUComponentData();
    void appendGPUComponentData(const QMatrix4x4& modelViewMatrix,
        const QMatrix4x4& projectionMatrix,
        float distance, float lightScale);
    void uploadGPUComponentData();

    Shading shading() const;
    void setShading(Shading shading);

    void renderNodes(GPUGraphData& gpuGraphData);
    void renderEdges(GPUGraphData& gpuGraphData);
    void renderText(GPUGraphData& gpuGraphData);
    void renderGraph();
    void render2D(QRect selectionRect = {});

    enum class Type
    {
        None        = 0x0,
        Color       = 0x1,
        Selection   = 0x2,
        All = Color | Selection
    };

    void renderToScreen(Flags<Type> type = Type::All);

    void renderSdfTexture(const GlyphMap& glyphMap, GLuint texture);
    virtual GLuint sdfTexture() const = 0;
};

#endif // GRAPHRENDERERCORE_H
