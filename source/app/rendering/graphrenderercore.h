#ifndef GRAPHRENDERERCORE_H
#define GRAPHRENDERERCORE_H

#include "openglfunctions.h"

#include "primitives/arrow.h"
#include "primitives/sphere.h"
#include "primitives/rectangle.h"

#include <QRect>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>

#include <array>
#include <vector>

struct GPUGraphData : OpenGLFunctions
{
    GPUGraphData();
    virtual ~GPUGraphData();

    void initialise(QOpenGLShaderProgram& nodesShader,
                    QOpenGLShaderProgram& edgesShader,
                    QOpenGLShaderProgram& textShader);
    void prepareVertexBuffers();
    void prepareNodeVAO(QOpenGLShaderProgram& shader);
    void prepareEdgeVAO(QOpenGLShaderProgram& shader);
    void prepareTextVAO(QOpenGLShaderProgram &shader);

    bool prepareRenderBuffers(int width, int height, GLuint depthTexture);

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

    bool unused() const;

    struct NodeData
    {
        float _position[3];
        int _component = -1;
        float _size = -1.0f;
        float _outerColor[3];
        float _innerColor[3];
        float _outlineColor[3];
    };

    struct EdgeData
    {
        float _sourcePosition[3];
        float _targetPosition[3];
        float _sourceSize = 0.0f;
        float _targetSize = 0.0f;
        int _edgeType = -1;
        int _component = -1;
        float _size = -1.0f;
        float _outerColor[3];
        float _innerColor[3];
        float _outlineColor[3];
    };

    struct GlyphData
    {
        int _component = -1;
        float _textureCoord[2];
        int _textureLayer = -1;
        float _basePosition[3];
        float _glyphOffset[2];
        float _glyphSize[2];
        float _color[3];
    };

    // There are two alpha values so that we can split the alpha blended layers
    // depending on their purpose. The rendering occurs in order based on _alpha1,
    // going from opaque to transparent, then resorting to _alpha2 in the same order,
    // when the values of _alpha1 match
    float _alpha1 = 0.0f;
    float _alpha2 = 0.0f;

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

class GraphRendererCore :
    public OpenGLFunctions
{
public:
    GraphRendererCore();
    virtual ~GraphRendererCore();

    static const int NUM_MULTISAMPLES = 4;

private:
    int _width = 0;
    int _height = 0;

    std::array<GPUGraphData, 6> _gpuGraphData;

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

protected:
    int width() const { return _width; }
    int height() const { return _height; }

    bool resize(int width, int height);

    GPUGraphData* gpuGraphDataForAlpha(float alpha1, float alpha2);
    void resetGPUGraphData();
    void uploadGPUGraphData();

    GLuint componentDataTBO() const { return _componentDataTBO; }

    void renderNodes(GPUGraphData& gpuGraphData);
    void renderEdges(GPUGraphData& gpuGraphData);
    void renderText(GPUGraphData& gpuGraphData);
    void renderGraph();
    void render2D(QRect selectionRect = {});
    void renderToFramebuffer();

    virtual GLuint sdfTexture() const = 0;
};

#endif // GRAPHRENDERERCORE_H
