#ifndef GRAPHCOMPONENTRENDERER_H
#define GRAPHCOMPONENTRENDERER_H

#include "camera.h"
#include "primitives/cylinder.h"
#include "primitives/sphere.h"
#include "transition.h"
#include "../maths/boundingbox.h"
#include "../graph/graph.h"
#include "../graph/grapharray.h"

#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QVector3D>
#include <QColor>
#include <QRect>

#include <memory>
#include <vector>

class GraphWidget;
class QOpenGLContext;
class Quad;
class GraphModel;
class SelectionManager;
class Camera;
class Octree;

class QOpenGLFunctions_3_3_Core;

class GraphComponentRendererShared
{
    friend class GraphComponentRenderer;

public:
    GraphComponentRendererShared(const QOpenGLContext& context);

private:
    const QOpenGLContext* _context;

    QOpenGLShaderProgram _screenShader;
    QOpenGLShaderProgram _selectionShader;

    QOpenGLShaderProgram _nodesShader;
    QOpenGLShaderProgram _edgesShader;

    QOpenGLShaderProgram _selectionMarkerShader;
    QOpenGLShaderProgram _debugLinesShader;
};

class GraphComponentRenderer
{
public:
    GraphComponentRenderer();

    static const int multisamples = 4;
    static constexpr float TRANSITION_DURATION = 1.0f;

    void initialise(std::shared_ptr<GraphModel> graphModel, ComponentId componentId,
                    GraphWidget& graphWidget,
                    std::shared_ptr<SelectionManager> selectionManager,
                    std::shared_ptr<GraphComponentRendererShared> shared);

    bool visible() { return _visible; }
    void setVisible(bool visible) { _visible = visible; }

    void cleanup();
    void update(float t);
    void render(int x, int y, int width = 0, int height = 0, float alpha = 1.0f);
    void resize(int width, int height);
    void resizeViewport(int width, int height);

    void moveFocusToNode(NodeId nodeId, Transition::Type transitionType);
    void moveFocusToCentreOfComponent(Transition::Type transitionType);
    void selectFocusNodeClosestToCameraVector(Transition::Type transitionType = Transition::Type::InversePower);

    ComponentId componentId() { return _componentId; }
    NodeId focusNodeId();
    QVector3D focusPosition();
    void enableFocusTracking() { _trackFocus = true; }
    void disableFocusTracking() { _trackFocus = false; }

    bool transitioning();
    bool trackingCentreOfComponent();
    bool autoZooming();

    void resetView(Transition::Type transitionType = Transition::Type::EaseInEaseOut);
    bool viewIsReset() { return trackingCentreOfComponent() && autoZooming(); }

    Camera* camera() { return &_viewData._camera; }
    void zoom(float delta);
    void zoomToDistance(float distance);

    void setSelectionRect(const QRect& rect) { _selectionRect = rect; }
    void clearSelectionRect() { _selectionRect = QRect(); }

    int width() const { return _width; }
    int height() const { return _height; }

    void updatePositionalData();
    enum class When
    {
        Later,
        Now
    };

    void updateVisualData(When when = When::Later);

    void cloneViewDataFrom(const GraphComponentRenderer& other);
    void saveViewData() { _savedViewData = _viewData; }
    void restoreViewData() { _viewData = _savedViewData; }

    bool initialised() { return _initialised; }

    void freeze();
    void thaw();

private:
    GraphWidget* _graphWidget;
    std::shared_ptr<GraphComponentRendererShared> _shared;
    Sphere _sphere;
    Cylinder _cylinder;

    bool _initialised;
    bool _visible;

    bool _frozen;
    bool _cleanupWhenThawed;
    bool _updateVisualDataWhenThawed;
    bool _updatePositionDataWhenThawed;

    struct ViewData
    {
        ViewData() :
            _zoomDistance(1.0f),
            _autoZooming(true)
        {
            _camera.setPosition(QVector3D(0.0f, 0.0f, 1.0f));
            _camera.setViewTarget(QVector3D(0.0f, 0.0f, 0.0f));
            _camera.setUpVector(QVector3D(0.0f, 1.0f, 0.0f));
        }

        Camera _camera;
        float _zoomDistance;
        bool _autoZooming;
        NodeId _focusNodeId;
        QVector3D _focusPosition;
    };

    ViewData _viewData;
    ViewData _savedViewData;

    void prepareVertexBuffers();
    void prepareNodeVAO();
    void prepareEdgeVAO();
    void prepareSelectionMarkerVAO();
    void prepareDebugLinesVAO();
    void prepareTexture();

    QOpenGLVertexArrayObject _screenQuadVAO;
    QOpenGLBuffer _screenQuadDataBuffer;
    void prepareQuad();

    int _width;
    int _height;
    int _viewportWidth;
    int _viewportHeight;

    GLuint _colorTexture;
    GLuint _selectionTexture;
    GLuint _depthTexture;
    GLuint _visualFBO;
    bool _FBOcomplete;

    bool prepareRenderBuffers();

    void renderNodes();
    void renderEdges();
    void renderDebugLines();
    void render2D();

    void centreNodeInViewport(NodeId nodeId, float cameraDistance = -1.0f,
                              Transition::Type transitionType = Transition::Type::None);
    void centrePositionInViewport(const QVector3D& viewTarget, float cameraDistance = -1.0f,
                                  Transition::Type transitionType = Transition::Type::None);

    float _entireComponentZoomDistance;
    void updateEntireComponentZoomDistance();

    bool _visualDataRequiresUpdate;
    void updateVisualDataIfRequired();

private:
    bool _trackFocus;
    float _targetZoomDistance;
    Transition _zoomTransition;
    QRect _selectionRect;

    QOpenGLFunctions_3_3_Core* _funcs;

    ComponentId _componentId;

    float _aspectRatio;
    float _fovx;
    float _fovy;
    Transition _panTransition;

    std::shared_ptr<GraphModel> _graphModel;
    std::shared_ptr<SelectionManager> _selectionManager;

    QMatrix4x4 _modelViewMatrix;
    QMatrix4x4 _projectionMatrix;

    std::vector<GLfloat> _nodePositionData;
    int _numNodesInPositionData;
    QOpenGLBuffer _nodePositionBuffer;

    std::vector<GLfloat> _edgePositionData;
    int _numEdgesInPositionData;
    QOpenGLBuffer _edgePositionBuffer;

    std::vector<GLfloat> _nodeVisualData;
    QOpenGLBuffer _nodeVisualBuffer;

    std::vector<GLfloat> _edgeVisualData;
    QOpenGLBuffer _edgeVisualBuffer;

    QOpenGLBuffer _selectionMarkerDataBuffer;
    QOpenGLVertexArrayObject _selectionMarkerDataVAO;

private:
    struct DebugLine
    {
        DebugLine(const QVector3D& start, const QVector3D& end, const QColor& color) :
            _start(start), _end(end), _color(color)
        {}

        QVector3D _start;
        QVector3D _end;
        QColor _color;
    };

    std::vector<DebugLine> _debugLines;
    std::mutex _debugLinesMutex;
    std::vector<GLfloat> _debugLinesData;
    QOpenGLBuffer _debugLinesDataBuffer;
    QOpenGLVertexArrayObject _debugLinesDataVAO;

public:
    void addDebugLine(const QVector3D& start, const QVector3D& end, const QColor color = QColor(Qt::GlobalColor::white))
    {
        DebugLine debugLine(start, end, color);
        _debugLines.push_back(debugLine);
    }
    void addDebugBoundingBox(const BoundingBox3D& boundingBox, const QColor color = QColor(Qt::GlobalColor::white));
    void clearDebugLines() { _debugLines.clear(); }
    void submitDebugLines();
};

#endif // GRAPHCOMPONENTRENDERER_H
