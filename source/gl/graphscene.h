#ifndef INSTANCEDGEOMETRYSCENE_H
#define INSTANCEDGEOMETRYSCENE_H

#include "abstractscene.h"
#include "camera.h"
#include "transition.h"
#include "../maths/boundingbox.h"
#include "../graph/graph.h"
#include "../graph/grapharray.h"

#include <QList>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QVector3D>
#include <QColor>
#include <QMutex>
#include <QRect>

class Sphere;
class Cylinder;
class Quad;
class GraphModel;
class SelectionManager;

class QOpenGLFunctions_3_3_Core;

class ComponentViewData
{
public:
    ComponentViewData();
    ComponentViewData(const ComponentViewData& other);

    Camera _camera;
    float _zoomDistance;
    NodeId _focusNodeId;

    bool _initialised;
};

class GraphScene : public AbstractScene
{
    Q_OBJECT

public:
    GraphScene(QObject* parent = 0);

    static const int multisamples = 4;

    void initialise();
    void cleanup();
    void update( float t );
    void render();
    void resize( int w, int h );

    void moveFocusToNode(NodeId nodeId, Transition::Type transitionType);
    void selectFocusNodeClosestToCameraVector(Transition::Type transitionType = Transition::Type::InversePower);
    ComponentId focusComponentId() { return _focusComponentId; }
    NodeId focusNodeId()
    {
        ComponentViewData* viewData = focusComponentViewData();
        return viewData != nullptr ? viewData->_focusNodeId : NodeId();
    }
    void enableFocusNodeTracking() { _trackFocusNode = true; }
    void disableFocusNodeTracking() { _trackFocusNode = false; }

    void moveToNextComponent();
    void moveToPreviousComponent();
    void moveToComponent(ComponentId componentId);

    bool interactionAllowed();

    void setGraphModel(GraphModel* graphModel);

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
    void prepareComponentMarkerVAO();
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
    void renderComponentMarkers();
    void renderDebugLines();
    void render2D();

    void centreNodeInViewport(NodeId nodeId, Transition::Type transitionType, float cameraDistance = -1.0f);
    void updateVisualData();

private slots:
    void onGraphChanged(const Graph* graph);
    void onNodeWillBeRemoved(const Graph*, NodeId nodeId);
    void onComponentAdded(const Graph*, ComponentId);
    void onComponentWillBeRemoved(const Graph* graph, ComponentId componentId);
    void onComponentSplit(const Graph* graph, ComponentId oldComponentId, const QSet<ComponentId>& splitters);
    void onComponentsWillMerge(const Graph* graph, const QSet<ComponentId>& mergers, ComponentId merged);

public slots:
    void onSelectionChanged(const SelectionManager& selectionManager);

private:
    bool _trackFocusNode;
    float _targetZoomDistance;
    Transition _zoomTransition;
    QRect _selectionRect;

    QOpenGLFunctions_3_3_Core* _funcs;

    ComponentId _focusComponentId;
    ComponentId _lastSplitterFocusComponentId;
    ComponentArray<ComponentViewData>* _componentsViewData;
    ComponentViewData* focusComponentViewData() const;

    float _aspectRatio;
    Camera* _camera;
    Transition _panTransition;

    Sphere* _sphere;
    Cylinder* _cylinder;
    Quad* _quad;

    GraphModel* _graphModel;

    QOpenGLShaderProgram _nodesShader;
    QVector<GLfloat> _nodePositionData;
    QOpenGLBuffer _nodePositionBuffer;

    QOpenGLShaderProgram _edgesShader;
    QVector<GLfloat> _edgePositionData;
    QOpenGLBuffer _edgePositionBuffer;

    QVector<GLfloat> _nodeVisualData;
    QOpenGLBuffer _nodeVisualBuffer;

    QVector<GLfloat> _edgeVisualData;
    QOpenGLBuffer _edgeVisualBuffer;

    QOpenGLShaderProgram _componentMarkerShader;
    QVector<GLfloat> _componentMarkerData;
    QOpenGLBuffer _componentMarkerDataBuffer;

    QOpenGLShaderProgram _selectionMarkerShader;
    QOpenGLBuffer _selectionMarkerDataBuffer;
    QOpenGLVertexArrayObject _selectionMarkerDataVAO;

    QList<DebugLine> _debugLines;
    QMutex _debugLinesMutex;
    QVector<GLfloat> _debugLinesData;
    QOpenGLBuffer _debugLinesDataBuffer;
    QOpenGLVertexArrayObject _debugLinesDataVAO;
    QOpenGLShaderProgram _debugLinesShader;

signals:
    void userInteractionStarted();
    void userInteractionFinished();

public:
    void addDebugLine(const QVector3D& start, const QVector3D& end, const QColor color = QColor(Qt::GlobalColor::white))
    {
        DebugLine debugLine(start, end, color);
        _debugLines.append(debugLine);
    }
    void addDebugBoundingBox(const BoundingBox3D& boundingBox, const QColor color = QColor(Qt::GlobalColor::white));
    void clearDebugLines() { _debugLines.clear(); }
    void submitDebugLines();
};

#endif // GRAPHSCENE_H
