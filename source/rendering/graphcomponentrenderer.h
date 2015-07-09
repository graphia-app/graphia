#ifndef GRAPHCOMPONENTRENDERER_H
#define GRAPHCOMPONENTRENDERER_H

#include "openglfunctions.h"
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
#include <QQuaternion>
#include <QColor>
#include <QRect>

#include <memory>
#include <vector>

class GraphRenderer;
class Quad;
class GraphModel;
class SelectionManager;
class Camera;
class Octree;

class GraphComponentRenderer : protected OpenGLFunctions
{
public:
    GraphComponentRenderer();

    void initialise(std::shared_ptr<GraphModel> graphModel, ComponentId componentId,
                    std::shared_ptr<SelectionManager> selectionManager,
                    GraphRenderer* graphRenderer);

    bool visible() { return _visible; }
    void setVisible(bool visible);

    void cleanup();
    void update(float t);
    void render(int x, int y, int width = 0, int height = 0, float alpha = 1.0f);
    void setSize(int viewportWidth, int viewportHeight,
                 int width = 0, int height = 0);

    bool transitionActive();

    int viewportWidth() const { return _viewportWidth; }
    int viewportHeight() const { return _viewportHeight; }

    int width() const { return _width; }
    int height() const { return _height; }

    void moveFocusToNode(NodeId nodeId);
    void moveFocusToCentreOfComponent();
    void moveFocusToNodeClosestCameraVector();
    void moveFocusToPositionContainingNodes(const QVector3D& position,
                                            std::vector<NodeId> nodeIds,
                                            const QQuaternion& rotation);

    ComponentId componentId() { return _componentId; }
    NodeId focusNodeId();
    QVector3D focusPosition();
    void enableFocusTracking() { _trackFocus = true; }
    void disableFocusTracking() { _trackFocus = false; }

    bool trackingCentreOfComponent();
    bool autoZooming();

    void resetView();
    bool viewIsReset() { return _viewData.isReset(); }

    Camera* camera() { return &_viewData._camera; }
    const Camera* camera() const { return &_viewData._camera; }
    void zoom(float delta);
    void zoomToDistance(float distance);

    void updatePositionalData();
    enum class When
    {
        Later,
        Now
    };

    void updateVisualData(When when = When::Later);

    void cloneViewDataFrom(const GraphComponentRenderer& other);
    void saveViewData() { _savedViewData = _viewData; }
    bool savedViewIsReset() { return _savedViewData.isReset(); }
    void restoreViewData();

    bool initialised() { return _initialised; }

    void freeze();
    void thaw();

    void updateTransition(float f);

private:
    GraphRenderer* _graphRenderer;
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
        {}

        bool isReset() { return _focusNodeId.isNull() && _autoZooming; }

        Camera _camera;
        float _zoomDistance;
        bool _autoZooming;
        NodeId _focusNodeId;
        QVector3D _focusPosition;

        Camera _transitionStart;
        Camera _transitionEnd;
    };

    ViewData _viewData;
    ViewData _savedViewData;

    void prepareVertexBuffers();
    void prepareNodeVAO();
    void prepareEdgeVAO();
    void prepareDebugLinesVAO();

    int _viewportWidth;
    int _viewportHeight;
    int _width;
    int _height;

    void updateMatrices();

    void renderNodes(float alpha);
    void renderEdges(float alpha);
    void renderDebugLines();
    void render2D();

    void centreNodeInViewport(NodeId nodeId, float cameraDistance = -1.0f);
    void centrePositionInViewport(const QVector3D& focus,
                                  float cameraDistance = -1.0f,
                                  const QQuaternion rotation = QQuaternion());

    float _entireComponentZoomDistance;
    float zoomDistanceForNodeIds(const QVector3D& centre, std::vector<NodeId> nodeIds);
    void updateFocusPosition();
    void updateEntireComponentZoomDistance();

    bool _visualDataRequiresUpdate;
    void updateVisualDataIfRequired();

private:
    bool _trackFocus;
    float _targetZoomDistance;
    Transition _zoomTransition;

    ComponentId _componentId;

    float _fovx;
    float _fovy;

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
