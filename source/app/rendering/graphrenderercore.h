#ifndef GRAPHRENDERERCORE_H
#define GRAPHRENDERERCORE_H

#include "openglfunctions.h"

#include "primitives/arrow.h"
#include "primitives/rectangle.h"
#include "primitives/sphere.h"

#include "shared/utils/flags.h"

#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QRect>

#include <array>
#include <vector>

class ScreenshotRenderer;

struct GPUGraphData : OpenGLFunctions
{
    GPUGraphData();
    ~GPUGraphData() override;

    void initialise(QOpenGLShaderProgram& nodesShader, QOpenGLShaderProgram& edgesShader,
                    QOpenGLShaderProgram& textShader);
    void prepareVertexBuffers();
    void prepareNodeVAO(QOpenGLShaderProgram& shader);
    void prepareEdgeVAO(QOpenGLShaderProgram& shader);
    void prepareTextVAO(QOpenGLShaderProgram& shader);

    bool prepareRenderBuffers(int width, int height, GLuint depthTexture, GLint numMultiSamples);
    void copyState(const GPUGraphData &gpuGraphData, QOpenGLShaderProgram &nodesShader, QOpenGLShaderProgram &edgesShader, QOpenGLShaderProgram &textShader);

    void reset();
    void clearFramebuffer();
    void clearDepthbuffer();

    void upload();

    int numNodes() const;
    int numEdges() const;

    Primitive::Sphere _sphere;
    Primitive::Arrow _arrow;
    Primitive::Rectangle _rectangle;

    float alpha() const;
    float componentAlpha() const;
    float unhighlightAlpha() const;

    bool unused() const;

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
        float _color[3] = {0.0f, 0.0f, 0.0f};
    };

    // There are two alpha values so that we can split the alpha blended layers
    // depending on their purpose. The rendering occurs in order based on _componentAlpha,
    // going from opaque to transparent, then resorting to _unhighlightAlpha in the same order,
    // when the values of _componentAlpha match
    float _componentAlpha = 0.0f;
    float _unhighlightAlpha = 0.0f;

    bool _alwaysDrawnLast = false;

    std::vector<NodeData> _nodeData;
    QOpenGLBuffer _nodeVBO;

    std::vector<GlyphData> _glyphData;
    QOpenGLBuffer _textVBO;

    std::vector<EdgeData> _edgeData;
    QOpenGLBuffer _edgeVBO;

    GLuint _fbo = 0;
    GLuint _colorTexture = 0;
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

    QOpenGLShaderProgram _nodesShader;
    QOpenGLShaderProgram _edgesShader;
    QOpenGLShaderProgram _textShader;

    QOpenGLShaderProgram _screenShader;
    QOpenGLShaderProgram _selectionShader;

    GLuint _depthTexture = 0;

    GLuint _componentDataTBO = 0;
    GLuint _componentDataTexture = 0;

    QOpenGLShaderProgram _selectionMarkerShader;
    QOpenGLBuffer _selectionMarkerDataBuffer;
    QOpenGLVertexArrayObject _selectionMarkerDataVAO;

    QOpenGLVertexArrayObject _screenQuadVAO;
    QOpenGLBuffer _screenQuadDataBuffer;

    void prepareComponentDataTexture();
    void prepareSelectionMarkerVAO();
    void prepareQuad();

    std::vector<int> gpuGraphDataRenderOrder() const;

    void prepare();

protected:
    int width() const { return _width; }
    int height() const { return _height; }

    bool resize(int width, int height);

    GPUGraphData* gpuGraphDataForAlpha(float componentAlpha, float unhighlightAlpha);
    GPUGraphData* gpuGraphDataForOverlay(float alpha);
    void resetGPUGraphData();
    void uploadGPUGraphData();

    GLuint componentDataTBO() const { return _componentDataTBO; }

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

    void renderToFramebuffer(Flags<Type> type = Type::All);

    virtual GLuint sdfTexture() const = 0;
};

#endif // GRAPHRENDERERCORE_H
