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
        return viewData != nullptr ? viewData->focusNodeId : NodeId();
    }
    void enableFocusNodeTracking() { trackFocusNode = true; }
    void disableFocusNodeTracking() { trackFocusNode = false; }

    void moveToNextComponent();
    void moveToPreviousComponent();
    void moveToComponent(ComponentId componentId);

    bool interactionAllowed();

    void setGraphModel(GraphModel* graphModel);

    Camera* camera() { return m_camera; }
    void zoom(float delta);

    void setSelectionRect(const QRect& rect) { selectionRect = rect; }
    void clearSelectionRect() { selectionRect = QRect(); }

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
    void prepareSelectionMarkerVAO();
    void prepareDebugLinesVAO();
    void prepareTexture();

    QOpenGLShaderProgram screenShader;
    QOpenGLShaderProgram selectionShader;
    void prepareScreenQuad();

    bool loadShaderProgram(QOpenGLShaderProgram& program, const QString& vertexShader, const QString& fragmentShader);

    int width;
    int height;

    GLuint colorTexture;
    GLuint selectionTexture;
    GLuint depthTexture;
    GLuint visualFBO;
    bool FBOcomplete;

    QOpenGLVertexArrayObject screenQuadVAO;

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
    bool trackFocusNode;
    float targetZoomDistance;
    Transition zoomTransition;
    QRect selectionRect;

    QOpenGLFunctions_3_3_Core* m_funcs;

    ComponentId _focusComponentId;
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

    QOpenGLShaderProgram nodesShader;
    QVector<GLfloat> m_nodePositionData;
    QOpenGLBuffer m_nodePositionBuffer;

    QOpenGLShaderProgram edgesShader;
    QVector<GLfloat> m_edgePositionData;
    QOpenGLBuffer m_edgePositionBuffer;

    QVector<GLfloat> m_nodeVisualData;
    QOpenGLBuffer m_nodeVisualBuffer;

    QVector<GLfloat> m_edgeVisualData;
    QOpenGLBuffer m_edgeVisualBuffer;

    QOpenGLShaderProgram componentMarkerShader;
    QVector<GLfloat> m_componentMarkerData;
    QOpenGLBuffer m_componentMarkerDataBuffer;

    QOpenGLShaderProgram selectionMarkerShader;
    QOpenGLBuffer selectionMarkerDataBuffer;
    QOpenGLVertexArrayObject selectionMarkerDataVAO;

    QList<DebugLine> debugLines;
    QMutex debugLinesMutex;
    QVector<GLfloat> debugLinesData;
    QOpenGLBuffer debugLinesDataBuffer;
    QOpenGLVertexArrayObject debugLinesDataVAO;
    QOpenGLShaderProgram debugLinesShader;

signals:
    void userInteractionStarted();
    void userInteractionFinished();

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

#endif // GRAPHSCENE_H
