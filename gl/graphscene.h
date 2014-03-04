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

    Camera camera;
    float zoomDistance;
    NodeId focusNodeId;

    bool initialised;
};

class GraphScene : public AbstractScene
{
    Q_OBJECT

public:
    GraphScene(QObject* parent = 0);

    void initialise();
    void update( float t );
    void render();
    void resize( int w, int h );

    void centreNodeInViewport(NodeId nodeId, Transition::Type transitionType, float cameraDistance = -1.0f);
    void selectFocusNode(ComponentId componentId, Transition::Type transitionType);

    void mousePressEvent(QMouseEvent* mouseEvent);
    void mouseReleaseEvent(QMouseEvent* mouseEvent);
    void mouseMoveEvent(QMouseEvent* mouseEvent);
    void mouseDoubleClickEvent(QMouseEvent* mouseEvent);
    void wheelEvent(QWheelEvent* wheelEvent);

    bool keyPressEvent(QKeyEvent* keyEvent);
    bool keyReleaseEvent(QKeyEvent* keyEvent);

    void setGraphModel(GraphModel* graphModel);
    void setSelectionManager(SelectionManager* selectionManager);

private:
    struct DebugLine
    {
        DebugLine(const QVector3D& start, const QVector3D& end, const QColor& color) :
            start(start), end(end), color(color)
        {}

        QVector3D start;
        QVector3D end;
        QColor color;
    };

    void prepareVertexBuffers();
    void prepareNodeVAO();
    void prepareEdgeVAO();
    void prepareComponentMarkerVAO();
    void prepareDebugLinesVAO();
    void prepareTexture();

    void renderNodes();
    void renderEdges();
    void renderComponentMarkers();
    void renderDebugLines();

    void zoom(float delta);

    void updateVisualData();

private slots:
    void onGraphChanged(const Graph* graph);
    void onNodeWillBeRemoved(const Graph*, NodeId nodeId);
    void onComponentAdded(const Graph*, ComponentId);
    void onComponentWillBeRemoved(const Graph* graph, ComponentId componentId);
    void onComponentSplit(const Graph* graph, ComponentId oldComponentId, const QSet<ComponentId>& splitters);
    void onComponentsWillMerge(const Graph* graph, const QSet<ComponentId>& mergers, ComponentId merged);

private:
    bool m_rightMouseButtonHeld;
    bool m_leftMouseButtonHeld;

    bool m_controlKeyHeld;
    bool m_selecting;
    bool m_frustumSelecting;
    QPoint m_frustumSelectStart;

    QPoint m_prevPos;
    QPoint m_pos;
    bool m_mouseMoving;
    NodeId clickedNodeId;

    float targetZoomDistance;
    Transition zoomTransition;

    QOpenGLFunctions_3_3_Core* m_funcs;

    ComponentId focusComponentId;
    ComponentId lastSplitterFocusComponentId;
    ComponentArray<ComponentViewData>* componentsViewData;
    ComponentViewData* focusComponentViewData() const;

    float aspectRatio;
    Camera* m_camera;
    Transition panTransition;

    Sphere* m_sphere;
    Cylinder* m_cylinder;
    Quad* m_quad;

    GraphModel* _graphModel;
    SelectionManager* _selectionManager;

    QVector<GLfloat> m_nodePositionData;
    QOpenGLBuffer m_nodePositionBuffer;

    QVector<GLfloat> m_edgePositionData;
    QOpenGLBuffer m_edgePositionBuffer;

    QVector<GLfloat> m_nodeVisualData;
    QOpenGLBuffer m_nodeVisualBuffer;

    QVector<GLfloat> m_edgeVisualData;
    QOpenGLBuffer m_edgeVisualBuffer;

    QVector<GLfloat> m_componentMarkerData;
    QOpenGLBuffer m_componentMarkerDataBuffer;

    QList<DebugLine> debugLines;
    QMutex debugLinesMutex;
    QVector<GLfloat> debugLinesData;
    QVector<QVector3D> debugLinesVertices;
    QOpenGLBuffer debugLinesDataBuffer;
    QOpenGLVertexArrayObject debugLinesDataVAO;
    QOpenGLShaderProgram debugLinesShader;

    void moveToNextComponent();
    void moveToPreviousComponent();
    void moveToComponent(ComponentId componentId);

public:
    void addDebugLine(const QVector3D& start, const QVector3D& end, const QColor color = QColor(Qt::GlobalColor::white))
    {
        DebugLine debugLine(start, end, color);
        debugLines.append(debugLine);
    }
    void addDebugBoundingBox(const BoundingBox3D& boundingBox, const QColor color = QColor(Qt::GlobalColor::white));
    void clearDebugLines() { debugLines.clear(); }
    void submitDebugLines();
};

#endif // INSTANCEDGEOMETRYSCENE_H
