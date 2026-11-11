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

#ifndef GRAPHRENDERERCORE_H
#define GRAPHRENDERERCORE_H

#include "approximateedgeshading.h"
#include "depthprepass.h"
#include "edgecullresults.h"
#include "openglfunctions.h"
#include "graphrenderertypes.h"
#include "elementshaders.h"
#include "nodegeometry.h"
#include "screenpass.h"
#include "screentexture.h"
#include "smaaantialiasing.h"
#include "splatcompositor.h"
#include "texelbuffer.h"

#include "primitives/impostorquads.h"
#include "primitives/rectangle.h"

#include "shared/utils/flags.h"
#include "shared/utils/namedbool.h"

#include <QColor>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QRect>
#include <QSize>
#include <QMatrix4x4>

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

class ScreenshotRenderer;
class GlyphMap;
class QOpenGLContext;

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

    bool prepareRenderBuffers(int width, int height, GLuint depthTexture);
    void copyState(const GPUGraphData &gpuGraphData, QOpenGLShaderProgram &nodesShader,
        QOpenGLShaderProgram &edgesShader, QOpenGLShaderProgram &textShader);

    void reset();
    void clearFramebuffer(GLbitfield buffers = GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    void clearDepthbuffer();
    void drawToFramebuffer();

    void upload();

    struct NodeData;
    struct EdgeData;
    struct GlyphData;

    void addNode(const NodeData& nodeData, float size);
    void addEdge(const EdgeData& edgeData);
    void addGlyph(const GlyphData& glyphData);

    size_t numNodes() const;
    size_t numEdges() const;
    size_t numGlyphs() const;

    Primitive::ImpostorQuads _nodeQuad{1};
    Primitive::ImpostorQuads _edgeQuads{2};
    EdgeCullResults _edgeCullResults;
    Primitive::Rectangle _rectangle;

    float alpha() const;
    float selectionAlpha() const;

    float componentAlpha() const;
    float unhighlightAlpha() const;

    bool unused() const;
    bool empty() const;
    bool invisible() const;

    bool hasGraphElements() const;

    // Eight bits a channel, as a QColor holds it and as it is drawn
    using Color = std::array<uint8_t, 4>;
    static Color colorFrom(const QColor& color);

    struct NodeData
    {
        int _nodeIndex = -1;
        int _component = -1;
        Color _outerColor = {};
        Color _innerColor = {};
        float _selected = 0.0f;
    };

    struct EdgeData
    {
        int _sourceIndex = -1;
        int _targetIndex = -1;
        int _edgeType = -1;
        int _component = -1;
        float _size = -1.0f;
        Color _outerColor = {};
        Color _innerColor = {};
        float _selected = 0.0f;
    };

    struct GlyphData
    {
        int _component = -1;
        float _textureCoord[2] = {0.0f, 0.0f};
        int _textureLayer = -1;
        int _anchorNodes[2] = {-1, -1};
        float _basePosition[3] = {0.0f, 0.0f, 0.0f};
        float _glyphOffset[2] = {0.0f, 0.0f};
        float _glyphSize[2] = {0.0f, 0.0f};
        float _glyphScale = 0.0f;
        float _textHeight = 0.0f;
        float _color[3] = {0.0f, 0.0f, 0.0f};
    };

    // There are two alpha values so that we can split the alpha blended layers
    // depending on their purpose. The rendering occurs in order based on _componentAlpha,
    // going from opaque to transparent, then resorting to _unhighlightAlpha in the same order,
    // when the values of _componentAlpha match
    float _componentAlpha = 0.0f;
    float _unhighlightAlpha = 0.0f;

    bool _isOverlay = false;

    struct ElementExtents
    {
        float _smallestNodeSize = std::numeric_limits<float>::max();
        float _largestNodeSize = 0.0f;

        bool _hasArrowHeads = false;
        float _smallestEdgeSize = std::numeric_limits<float>::max();
        float _largestEdgeSize = 0.0f;
    };

    ElementExtents _extents;

    std::vector<NodeData> _nodeData;
    QOpenGLBuffer _nodeVBO;

    std::vector<GlyphData> _glyphData;
    float _tallestTextHeight = 0.0f;
    QOpenGLBuffer _textVBO;

    std::vector<EdgeData> _edgeData;
    QOpenGLBuffer _edgeVBO;

    bool _elementsSelected = false;

    GLuint _fbo = 0;
    GLuint _colorTexture = 0;
    GLuint _elementTexture = 0;

    // Which part of its element each pixel shows
    GLuint _subElementTexture = 0;

    GLuint _selectionTexture = 0;
};

class GraphRendererCore : public OpenGLFunctions
{
    friend class ScreenshotRenderer;

public:
    explicit GraphRendererCore(DetailThresholds detailThresholds = {});
    ~GraphRendererCore() override;

private:
    QOpenGLContext* _context = nullptr;

    int _width = 0;
    int _height = 0;

    std::array<GPUGraphData, 7> _gpuGraphData;

    QOpenGLShaderProgram _sdfShader;

    ElementShaders _elementShaders;
    DetailThresholds _detailThresholds;
    QOpenGLShaderProgram _textShader;

    QOpenGLShaderProgram _screenShader;
    QOpenGLShaderProgram _outlineShader;
    QOpenGLShaderProgram _selectionShader;

    QOpenGLShaderProgram _compositeElementShader;
    ScreenPass _screenPass;

    ScreenTexture _composite{GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, GL_LINEAR};
    ScreenTexture _compositeElements{GL_RG32F, GL_RG, GL_FLOAT, GL_NEAREST};
    SmaaAntialiasing _smaaAntialiasing;

    ScreenTexture _smoothedSelection{GL_R8, GL_RED, GL_UNSIGNED_BYTE, GL_LINEAR};
    SplatCompositor _splatCompositor{ElementShaders::glUnit(ElementShaders::Unit::NearestNode)};

    GLuint _depthTexture = 0;

    DepthPrepass _depthPrepass;

    ApproximateEdgeShading _approximateEdgeShading;

    struct ComponentData
    {
        QMatrix4x4 _modelViewMatrix;
        QMatrix4x4 _projectionMatrix;
        float _distance = 0.0f;
        float _lightScale = 0.0f;
        float _radius = 0.0f;
    };

    std::vector<ComponentData> _componentData;
    TexelBuffer _componentDataTexels;

    NodeGeometry _nodeGeometry;

    Shading _shading = Shading::Smooth;

    QOpenGLShaderProgram _selectionMarkerShader;
    QOpenGLBuffer _selectionMarkerDataBuffer;
    QOpenGLVertexArrayObject _selectionMarkerDataVAO;

    QOpenGLVertexArrayObject _screenQuadVAO;
    QOpenGLBuffer _screenQuadDataBuffer;

    void prepareSelectionMarkerVAO();
    void prepareScreenQuad();

    std::vector<size_t> gpuGraphDataRenderOrder() const;

    void bindComponentDataTexture(GLenum textureUnit, QOpenGLShaderProgram& shader);
    void bindNodeGeometryTexture(GLenum textureUnit, QOpenGLShaderProgram& shader);

    struct PixelScaleRange
    {
        float _minimum = 0.0f;
        float _maximum = 0.0f;
    };

    PixelScaleRange pixelScaleRange() const;

    // Which levels of detail a layer may draw, from how narrow and wide its elements
    // can be on screen; each element picks its own level in the shader
    struct DetailPasses
    {
        bool _tracedNodes = false;
        bool _splattedNodes = false;
        bool _fastSplattedEdges = false;
        bool _splattedEdges = false;
        bool _approximateTracedEdges = false;
        bool _tracedEdges = false;

        bool anySplatted() const { return _splattedNodes || _fastSplattedEdges || _splattedEdges; }
    };

    DetailPasses detailPasses(const GPUGraphData& gpuGraphData, const PixelScaleRange& pixelScale) const;
    void renderEdges(GPUGraphData& gpuGraphData, const DetailPasses& passes);
    void renderSplats(GPUGraphData& gpuGraphData, const DetailPasses& passes);

    void renderToLayer(GPUGraphData& gpuGraphData);
    void renderNodesWithShader(QOpenGLShaderProgram& shader, GPUGraphData& gpuGraphData,
        NamedBool<"asPoints"> asPoints = "asPoints"_no);
    void bindEdgeParameters(GPUGraphData& gpuGraphData);
    void releaseEdgeParameters(GPUGraphData& gpuGraphData);

    enum class CullingResults : uint8_t { None, VisibleRanges, VisibleRangesAndPrepassHits };

    void renderEdgesWithShader(QOpenGLShaderProgram& shader, GPUGraphData& gpuGraphData,
        CullingResults cullingResults = CullingResults::None);
    bool cullHiddenEdgeStretches(GPUGraphData& gpuGraphData);
    void fitEdgeShading(GPUGraphData& gpuGraphData);

    void compositeElements();
    void compositeLayer(QOpenGLShaderProgram& shader, float alpha,
        std::initializer_list<GLuint> textures);

    void prepare();

protected:
    bool makeContextCurrent();

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
        float distance, float lightScale, float radius);
    void uploadGPUComponentData();

    NodeGeometry& nodeGeometry() { return _nodeGeometry; }

    Shading shading() const;
    void setShading(Shading shading);

    void renderText(GPUGraphData& gpuGraphData, const PixelScaleRange& pixelScale);
    void renderGraph();
    void render2D(QRect selectionRect = {});

    enum class Type : unsigned char
    {
        None        = 0x0,
        Color       = 0x1,
        Selection   = 0x2,
        All = Color | Selection
    };

    void compositeInto(GLuint framebuffer, Flags<Type> type = Type::All);

    QSize renderSdfTexture(const GlyphMap& glyphMap, GLuint texture);
    virtual GLuint sdfTexture() const = 0;
};

#endif // GRAPHRENDERERCORE_H
