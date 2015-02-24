#ifndef GRAPHRENDERER_H
#define GRAPHRENDERER_H

#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>

class GraphWidget;
class QOpenGLContext;
class QOpenGLFunctions_3_3_Core;

class GraphRenderer
{
    friend class GraphComponentRenderer;

public:
    GraphRenderer(GraphWidget* graphWidget, const QOpenGLContext& context);
    virtual ~GraphRenderer();

    static const int NUM_MULTISAMPLES = 4;

    void resize(int width, int height);

    void clear();
    void render();

private:
    GraphWidget* _graphWidget;
    const QOpenGLContext* _context;
    QOpenGLFunctions_3_3_Core* _funcs;

    QOpenGLShaderProgram _screenShader;
    QOpenGLShaderProgram _selectionShader;

    QOpenGLShaderProgram _nodesShader;
    QOpenGLShaderProgram _edgesShader;

    QOpenGLShaderProgram _selectionMarkerShader;
    QOpenGLShaderProgram _debugLinesShader;

    int _width;
    int _height;

    GLuint _colorTexture;
    GLuint _selectionTexture;
    GLuint _depthTexture;
    GLuint _visualFBO;
    bool _FBOcomplete;

    bool prepareRenderBuffers(int width, int height);

    QOpenGLVertexArrayObject _screenQuadVAO;
    QOpenGLBuffer _screenQuadDataBuffer;

    void prepareQuad();
};

#endif // GRAPHRENDERER_H
