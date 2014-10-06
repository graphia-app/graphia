#ifndef GRAPHCOMPONENTSCENE_H
#define GRAPHCOMPONENTSCENE_H

#include "scene.h"
#include "graphcomponentviewdata.h"
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

class Quad;
class GraphModel;
class SelectionManager;
class Camera;
class Octree;

class QOpenGLFunctions_3_3_Core;

class GraphComponentScene : public Scene
{
    Q_OBJECT

public:
    GraphComponentScene(std::shared_ptr<ComponentArray<GraphComponentViewData>> componentsViewData,
                        QObject* parent = nullptr);

    static const int multisamples = 4;

    void initialise();
    void cleanup();
    void update(float t);
    void render();
    void resize(int w, int h);

    void moveFocusToNode(NodeId nodeId, Transition::Type transitionType);
    void selectFocusNodeClosestToCameraVector(Transition::Type transitionType = Transition::Type::InversePower);
    ComponentId focusComponentId() { return _focusComponentId; }
    NodeId focusNodeId()
    {
        GraphComponentViewData* viewData = focusComponentViewData();
        return viewData != nullptr ? viewData->_focusNodeId : NodeId();
    }
    void enableFocusNodeTracking() { _trackFocusNode = true; }
    void disableFocusNodeTracking() { _trackFocusNode = false; }

    void moveToNextComponent();
    void moveToPreviousComponent();
    void moveToComponent(ComponentId componentId);

    bool transitioning();

    void setGraphModel(std::shared_ptr<GraphModel> graphModel);
    void setSelectionManager(std::shared_ptr<SelectionManager> selectionManager);

    Camera* camera() { return _camera; }
    void zoom(float delta);

    void setSelectionRect(const QRect& rect) { _selectionRect = rect; }
    void clearSelectionRect() { _selectionRect = QRect(); }

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

    void prepareVertexBuffers();
    void prepareNodeVAO();
    void prepareEdgeVAO();
    void prepareSelectionMarkerVAO();
    void prepareDebugLinesVAO();
    void prepareTexture();

    QOpenGLShaderProgram _screenShader;
    QOpenGLShaderProgram _selectionShader;
    void prepareScreenQuad();

    bool loadShaderProgram(QOpenGLShaderProgram& program, const QString& vertexShader, const QString& fragmentShader);

    int _width;
    int _height;

    GLuint _colorTexture;
    GLuint _selectionTexture;
    GLuint _depthTexture;
    GLuint _visualFBO;
    bool _FBOcomplete;

    QOpenGLVertexArrayObject _screenQuadVAO;

    bool prepareRenderBuffers();

    void renderNodes();
    void renderEdges();
    void renderDebugLines();
    void render2D();

    void centreNodeInViewport(NodeId nodeId, Transition::Type transitionType, float cameraDistance = -1.0f);

    bool _positionalDataRequiresUpdate;
    void queuePositionalDataUpdate();
    void updatePositionalData();

    bool _visualDataRequiresUpdate;
    void queueVisualDataUpdate();
    void updateVisualData();

private slots:
    void onGraphChanged(const Graph* graph);
    void onNodeWillBeRemoved(const Graph*, NodeId nodeId);
    void onComponentAdded(const Graph*, ComponentId);
    void onComponentWillBeRemoved(const Graph* graph, ComponentId componentId);
    void onComponentSplit(const Graph* graph, ComponentId oldComponentId, const ElementIdSet<ComponentId>& splitters);
    void onComponentsWillMerge(const Graph* graph, const ElementIdSet<ComponentId>& mergers, ComponentId merged);

public slots:
    void onSelectionChanged(const SelectionManager*);

private:
    bool _trackFocusNode;
    float _targetZoomDistance;
    Transition _zoomTransition;
    QRect _selectionRect;

    QOpenGLFunctions_3_3_Core* _funcs;

    std::vector<ComponentId> _componentIdsCache;
    void refreshComponentIdsCache();

    ComponentId _focusComponentId;
    ComponentId _lastSplitterFocusComponentId;
    std::shared_ptr<ComponentArray<GraphComponentViewData>> _componentsViewData;
    GraphComponentViewData* focusComponentViewData() const;

    float _aspectRatio;
    Camera* _camera;
    Transition _panTransition;

    Sphere _sphere;
    Cylinder _cylinder;

    std::shared_ptr<GraphModel> _graphModel;
    std::shared_ptr<SelectionManager> _selectionManager;

    QMatrix4x4 _modelViewMatrix;
    QMatrix4x4 _projectionMatrix;

    QOpenGLShaderProgram _nodesShader;
    std::vector<GLfloat> _nodePositionData;
    int _numNodesInPositionData;
    QOpenGLBuffer _nodePositionBuffer;

    QOpenGLShaderProgram _edgesShader;
    std::vector<GLfloat> _edgePositionData;
    int _numEdgesInPositionData;
    QOpenGLBuffer _edgePositionBuffer;

    std::vector<GLfloat> _nodeVisualData;
    QOpenGLBuffer _nodeVisualBuffer;

    std::vector<GLfloat> _edgeVisualData;
    QOpenGLBuffer _edgeVisualBuffer;

    QOpenGLShaderProgram _selectionMarkerShader;
    QOpenGLBuffer _selectionMarkerDataBuffer;
    QOpenGLVertexArrayObject _selectionMarkerDataVAO;

    std::vector<DebugLine> _debugLines;
    std::mutex _debugLinesMutex;
    std::vector<GLfloat> _debugLinesData;
    QOpenGLBuffer _debugLinesDataBuffer;
    QOpenGLVertexArrayObject _debugLinesDataVAO;
    QOpenGLShaderProgram _debugLinesShader;

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

#endif // GRAPHCOMPONENTSCENE_H
