#include "graphscene.h"

#include "camera.h"
#include "sphere.h"
#include "cylinder.h"
#include "quad.h"
#include "material.h"

#include "../graph/graphmodel.h"
#include "../layout/layout.h"
#include "../layout/spatialoctree.h"
#include "../layout/collision.h"

#include "../maths/frustum.h"
#include "../maths/plane.h"
#include "../maths/boundingsphere.h"

#include "../ui/selectionmanager.h"

#include "../utils.h"

#include <QObject>
#include <QOpenGLContext>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLFunctions_3_2_Core>
#if defined(Q_OS_MAC)
#include <QOpenGLExtensions>
#endif

#include <QMutexLocker>

#include <QKeyEvent>
#include <QMouseEvent>

#include <QtMath>
#include <cmath>

GraphScene::GraphScene( QObject* parent )
    : AbstractScene( parent ),
      width(0), height(0),
      colorTexture(0),
      selectionTexture(0),
      depthTexture(0),
      visualFBO(0),
      FBOcomplete(false),
      m_rightMouseButtonHeld(false),
      m_leftMouseButtonHeld(false),
      m_controlKeyHeld(false),
      m_shiftKeyHeld(false),
      m_selecting(false),
      m_frustumSelecting(false),
      m_mouseMoving(false),
      m_funcs(nullptr),
#if defined(Q_OS_MAC)
      m_instanceFuncs( 0 ),
#endif
      componentsViewData(nullptr),
      aspectRatio(4.0f / 3.0f),
      m_camera(nullptr),
      m_sphere(nullptr),
      m_cylinder(nullptr),
      m_quad(nullptr),
      _graphModel(nullptr),
      m_nodePositionData(0),
      m_edgePositionData(0),
      m_nodeVisualData(0),
      m_edgeVisualData(0),
      m_componentMarkerData(0),
      debugLinesData(0)
{
    update(0.0f);
}

void GraphScene::initialise()
{
    // Resolve the OpenGL functions that we need for instanced rendering
#if !defined(Q_OS_MAC)
    m_funcs = m_context->versionFunctions<QOpenGLFunctions_3_3_Core>();
#else
    m_instanceFuncs = new QOpenGLExtension_ARB_instanced_arrays;
    if ( !m_instanceFuncs->initializeOpenGLFunctions() )
        qFatal( "Could not resolve GL_ARB_instanced_arrays functions" );

    m_funcs = m_context->versionFunctions<QOpenGLFunctions_3_2_Core>();
#endif
    if ( !m_funcs )
        qFatal( "Could not obtain required OpenGL context version" );
    m_funcs->initializeOpenGLFunctions();

    MaterialPtr nodeMaterial(new Material);
    nodeMaterial->setShaders(":/gl/shaders/instancednodes.vert", ":/gl/shaders/ads.frag" );
    loadShaderProgram(nodesShader, ":/gl/shaders/instancednodes.vert", ":/gl/shaders/ads.frag");

    // Create a sphere
    m_sphere = new Sphere( this );
    m_sphere->setRadius(1.0f);
    m_sphere->setRings(16);
    m_sphere->setSlices(16);
    m_sphere->setMaterial(nodeMaterial);
    m_sphere->create();

    MaterialPtr edgeMaterial(new Material);
    edgeMaterial->setShaders(":/gl/shaders/instancededges.vert", ":/gl/shaders/ads.frag" );
    loadShaderProgram(edgesShader, ":/gl/shaders/instancededges.vert", ":/gl/shaders/ads.frag");

    m_cylinder = new Cylinder(this);
    m_cylinder->setRadius(1.0f);
    m_cylinder->setLength(1.0f);
    m_cylinder->setSlices(8);
    m_cylinder->setMaterial(edgeMaterial);
    m_cylinder->create();

    MaterialPtr componentMarkerMaterial(new Material);
    componentMarkerMaterial->setShaders(":/gl/shaders/instancedmarkers.vert", ":/gl/shaders/marker.frag" );
    loadShaderProgram(componentMarkerShader, ":/gl/shaders/instancedmarkers.vert", ":/gl/shaders/marker.frag");

    m_quad = new Quad(this);
    m_quad->setEdgeLength(1.0f);
    m_quad->setMaterial(componentMarkerMaterial);
    m_quad->create();

    debugLinesDataVAO.create();
    loadShaderProgram(debugLinesShader, ":/gl/shaders/debuglines.vert", ":/gl/shaders/debuglines.frag");

    // Create a pair of VBOs ready to hold our data
    prepareVertexBuffers();

    // Tell OpenGL how to pass the data VBOs to the shader program
    prepareNodeVAO();
    prepareEdgeVAO();
    prepareComponentMarkerVAO();
    prepareDebugLinesVAO();
    prepareScreenQuad();

    // Enable depth testing to prevent artifacts
    glEnable( GL_DEPTH_TEST );

    // Cull back facing triangles to save the gpu some work
    glEnable(GL_CULL_FACE);
}

void GraphScene::cleanup()
{
    //FIXME
}

void GraphScene::updateVisualData()
{
    NodeVisuals& nodeVisuals = _graphModel->nodeVisuals();
    EdgeVisuals& edgeVisuals = _graphModel->edgeVisuals();
    const ReadOnlyGraph& component = *_graphModel->graph().componentById(focusComponentId);

    m_nodeVisualData.resize(component.numNodes() * 8);
    m_edgeVisualData.resize(component.numEdges() * 8);
    int i = 0;
    int j = 0;

    for(NodeId nodeId : component.nodeIds())
    {
        m_nodeVisualData[i++] = nodeVisuals[nodeId].size;
        m_nodeVisualData[i++] = nodeVisuals[nodeId].color.redF();
        m_nodeVisualData[i++] = nodeVisuals[nodeId].color.greenF();
        m_nodeVisualData[i++] = nodeVisuals[nodeId].color.blueF();
        m_nodeVisualData[i++] = nodeVisuals[nodeId].outlineColor.redF();
        m_nodeVisualData[i++] = nodeVisuals[nodeId].outlineColor.greenF();
        m_nodeVisualData[i++] = nodeVisuals[nodeId].outlineColor.blueF();
        m_nodeVisualData[i++] = nodeVisuals[nodeId].outlineColor.alphaF();
    }

    for(EdgeId edgeId : component.edgeIds())
    {
        m_edgeVisualData[j++] = edgeVisuals[edgeId].size;
        m_edgeVisualData[j++] = edgeVisuals[edgeId].color.redF();
        m_edgeVisualData[j++] = edgeVisuals[edgeId].color.greenF();
        m_edgeVisualData[j++] = edgeVisuals[edgeId].color.blueF();
        m_edgeVisualData[j++] = edgeVisuals[edgeId].outlineColor.redF();
        m_edgeVisualData[j++] = edgeVisuals[edgeId].outlineColor.greenF();
        m_edgeVisualData[j++] = edgeVisuals[edgeId].outlineColor.blueF();
        m_edgeVisualData[j++] = edgeVisuals[edgeId].outlineColor.alphaF();
    }
}

void GraphScene::onGraphChanged(const Graph*)
{
    if(focusComponentId.isNull())
    {
        // The component we were focused on has gone, we need to find a new one
        moveToNextComponent();
    }

    updateVisualData();
}

void GraphScene::onNodeWillBeRemoved(const Graph*, NodeId nodeId)
{
    ComponentViewData* currentComponentViewData = focusComponentViewData();
    if(currentComponentViewData->focusNodeId == nodeId)
        currentComponentViewData->focusNodeId.setToNull();
}

static void setupCamera(Camera& camera, float aspectRatio)
{
    camera.setPerspectiveProjection(60.0f, aspectRatio, 0.3f, 10000.0f);
}

void GraphScene::onComponentAdded(const Graph*, ComponentId componentId)
{
    ComponentViewData* componentViewData = &(*componentsViewData)[componentId];

    if(!componentViewData->initialised)
    {
        setupCamera(componentViewData->camera, aspectRatio);
        targetZoomDistance = componentViewData->zoomDistance;
        componentViewData->focusNodeId.setToNull();
        componentViewData->initialised = true;
    }
}

void GraphScene::onComponentWillBeRemoved(const Graph*, ComponentId componentId)
{
    ComponentViewData* componentViewData = &(*componentsViewData)[componentId];
    componentViewData->initialised = false;

    if(componentId == lastSplitterFocusComponentId)
        lastSplitterFocusComponentId.setToNull();

    if(componentId == focusComponentId)
    {
        if(!lastSplitterFocusComponentId.isNull())
        {
            ComponentViewData* currentComponentViewData = focusComponentViewData();
            ComponentViewData* lastSplitterComponentViewData = &(*componentsViewData)[lastSplitterFocusComponentId];
            *lastSplitterComponentViewData = *currentComponentViewData;
            lastSplitterComponentViewData->focusNodeId.setToNull();
            focusComponentId = lastSplitterFocusComponentId;
        }
        else
            focusComponentId.setToNull();
    }
}

void GraphScene::onComponentSplit(const Graph* graph, ComponentId oldComponentId, const QSet<ComponentId>& splitters)
{
    if(oldComponentId == focusComponentId)
    {
        ComponentViewData currentComponentViewData = *focusComponentViewData();
        ComponentId newFocusComponentId = graph->componentIdOfNode(currentComponentViewData.focusNodeId);

        for(ComponentId splitter : splitters)
        {
            ComponentViewData* splitterComponentViewData = &(*componentsViewData)[splitter];

            // Clone the current camera data to all splitters
            *splitterComponentViewData = currentComponentViewData;

            // Splitters that don't contain the current focus node will need to find a new one
            if(splitter != newFocusComponentId)
                splitterComponentViewData->focusNodeId.setToNull();
        }

        focusComponentId = newFocusComponentId;

        QSet<ComponentId> nonFocusSplitters(splitters);
        nonFocusSplitters.remove(focusComponentId);
        lastSplitterFocusComponentId = nonFocusSplitters.values().at(0);
    }
}

void GraphScene::onComponentsWillMerge(const Graph*, const QSet<ComponentId>& mergers, ComponentId merged)
{
    qDebug() << "onComponentsWillMerge" << mergers << merged; //FIXME leaving this in as it's untested for now
    for(ComponentId merger : mergers)
    {
        if(merger == focusComponentId)
        {
            ComponentViewData* mergerComponentViewData = &(*componentsViewData)[merger];
            ComponentViewData* mergedComponentViewData = &(*componentsViewData)[merged];
            *mergedComponentViewData = *mergerComponentViewData;
            focusComponentId = merged;
            break;
        }
    }
}

void GraphScene::onSelectionChanged()
{
    qDebug() << "Selection:" << _selectionManager->selectedNodes();

    NodeVisuals& nodeVisuals = _graphModel->nodeVisuals();
    EdgeVisuals& edgeVisuals = _graphModel->edgeVisuals();

    for(NodeId nodeId : _graphModel->graph().nodeIds())
    {
        if(_selectionManager->nodeIsSelected(nodeId))
            nodeVisuals[nodeId].outlineColor = Qt::GlobalColor::white;
        else
            nodeVisuals[nodeId].outlineColor = Qt::GlobalColor::black;
    }

    // Edges can't be selected at the moment
    for(EdgeId edgeId : _graphModel->graph().edgeIds())
        edgeVisuals[edgeId].outlineColor = Qt::GlobalColor::black;

    updateVisualData();
}

ComponentViewData* GraphScene::focusComponentViewData() const
{
    if(focusComponentId.isNull())
        return nullptr;

    return componentsViewData != nullptr ? &(*componentsViewData)[focusComponentId] : nullptr;
}

void GraphScene::update(float t)
{
    if(_graphModel != nullptr)
    {
        NodePositions& nodePositions = _graphModel->nodePositions();
        QMutexLocker mutexLocker(&nodePositions.mutex());

        const ReadOnlyGraph* component = _graphModel->graph().componentById(focusComponentId);

        ComponentViewData* componentViewData = focusComponentViewData();
        m_camera = &componentViewData->camera;
        if(componentViewData->focusNodeId.isNull())
            selectFocusNode(focusComponentId, Transition::Type::None);

        m_nodePositionData.resize(component->numNodes() * 3);
        m_edgePositionData.resize(component->numEdges() * 6);
        int i = 0;

        for(NodeId nodeId : component->nodeIds())
        {
            QVector3D nodePosition = nodePositions[nodeId];

            m_nodePositionData[i++] = nodePosition.x();
            m_nodePositionData[i++] = nodePosition.y();
            m_nodePositionData[i++] = nodePosition.z();
        }

        i = 0;
        for(EdgeId edgeId : component->edgeIds())
        {
            const Edge& edge = _graphModel->graph().edgeById(edgeId);
            QVector3D sourcePosition = nodePositions[edge.sourceId()];
            QVector3D targetPosition = nodePositions[edge.targetId()];

            m_edgePositionData[i++] = sourcePosition.x();
            m_edgePositionData[i++] = sourcePosition.y();
            m_edgePositionData[i++] = sourcePosition.z();
            m_edgePositionData[i++] = targetPosition.x();
            m_edgePositionData[i++] = targetPosition.y();
            m_edgePositionData[i++] = targetPosition.z();
        }

#if 0
        //FIXME debug
        SpatialOctree octree(NodeLayout::boundingBox(*component, nodePositions), component->nodeIds(), nodePositions);
        octree.debugRenderOctree(this);
#endif

        zoomTransition.update(t);

        if(!panTransition.finished())
            panTransition.update(t);
        else if(!m_rightMouseButtonHeld)
            centreNodeInViewport(componentViewData->focusNodeId, Transition::Type::None, componentViewData->zoomDistance);
    }

    submitDebugLines();
}

void GraphScene::renderNodes(QOpenGLShaderProgram& program)
{
    const ReadOnlyGraph* component = _graphModel->graph().componentById(focusComponentId);

    m_nodePositionBuffer.bind();
    m_nodePositionBuffer.allocate(m_nodePositionData.data(), m_nodePositionData.size() * sizeof(GLfloat) );
    m_nodeVisualBuffer.bind();
    m_nodeVisualBuffer.allocate(m_nodeVisualData.data(), m_nodeVisualData.size() * sizeof(GLfloat) );

    // Calculate needed matrices
    QMatrix4x4 modelViewMatrix = m_camera->viewMatrix();
    QMatrix3x3 normalMatrix = modelViewMatrix.normalMatrix();
    program.setUniformValue("modelViewMatrix", modelViewMatrix);
    program.setUniformValue("normalMatrix", normalMatrix);
    program.setUniformValue("projectionMatrix", m_camera->projectionMatrix());

    m_funcs->glBindFragDataLocation(program.programId(), 0, "outColor");
    m_funcs->glBindFragDataLocation(program.programId(), 1, "outSelection");

    // Draw the nodes
    m_sphere->vertexArrayObject()->bind();
    m_funcs->glDrawElementsInstanced(GL_TRIANGLES, m_sphere->indexCount(),
                                     GL_UNSIGNED_INT, 0, component->numNodes());
    m_sphere->vertexArrayObject()->release();
}

void GraphScene::renderEdges(QOpenGLShaderProgram& program)
{
    const ReadOnlyGraph* component = _graphModel->graph().componentById(focusComponentId);

    m_edgePositionBuffer.bind();
    m_edgePositionBuffer.allocate(m_edgePositionData.data(), m_edgePositionData.size() * sizeof(GLfloat));
    m_edgeVisualBuffer.bind();
    m_edgeVisualBuffer.allocate(m_edgeVisualData.data(), m_edgeVisualData.size() * sizeof(GLfloat));

    program.setUniformValue("viewMatrix", m_camera->viewMatrix());
    program.setUniformValue("projectionMatrix", m_camera->projectionMatrix());

    m_funcs->glBindFragDataLocation(program.programId(), 0, "outColor");
    m_funcs->glBindFragDataLocation(program.programId(), 1, "outSelection");

    // Draw the edges
    m_cylinder->vertexArrayObject()->bind();
    m_funcs->glDrawElementsInstanced(GL_TRIANGLES, m_cylinder->indexCount(),
                                     GL_UNSIGNED_INT, 0, component->numEdges());
    m_cylinder->vertexArrayObject()->release();
}

void GraphScene::renderComponentMarkers()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_componentMarkerDataBuffer.bind();
    m_componentMarkerDataBuffer.allocate(m_componentMarkerData.data(), m_componentMarkerData.size() * sizeof(GLfloat) );

    // Bind the shader program
    componentMarkerShader.bind();

    // Calculate needed matrices
    QMatrix4x4 modelViewMatrix = m_camera->viewMatrix();
    componentMarkerShader.setUniformValue( "modelViewMatrix", modelViewMatrix );
    componentMarkerShader.setUniformValue( "projectionMatrix", m_camera->projectionMatrix() );

    // Draw the edges
    m_quad->vertexArrayObject()->bind();
    m_funcs->glDrawElementsInstanced(GL_TRIANGLES, m_quad->indexCount(),
                                     GL_UNSIGNED_INT, 0, _graphModel->graph().numComponents());
    m_quad->vertexArrayObject()->release();
    componentMarkerShader.release();

    glDisable(GL_BLEND);
}

void GraphScene::renderDebugLines()
{
    QMutexLocker locker(&debugLinesMutex);

    debugLinesDataBuffer.bind();
    debugLinesDataBuffer.allocate(debugLinesData.data(), debugLinesData.size() * sizeof(GLfloat));

    debugLinesShader.bind();

    // Calculate needed matrices
    QMatrix4x4 modelViewMatrix = m_camera->viewMatrix();
    debugLinesShader.setUniformValue( "modelViewMatrix", modelViewMatrix );
    debugLinesShader.setUniformValue( "projectionMatrix", m_camera->projectionMatrix() );

    debugLinesDataVAO.bind();
    glDrawArrays(GL_LINES, 0, debugLines.size() * 2);
    debugLinesDataVAO.release();
    debugLinesShader.release();

    clearDebugLines();
}

void GraphScene::addDebugBoundingBox(const BoundingBox3D& boundingBox, const QColor color)
{
    const QVector3D& min = boundingBox.min();
    const QVector3D& max = boundingBox.max();

    const QVector3D _0 = QVector3D(min.x(), min.y(), min.z());
    const QVector3D _1 = QVector3D(max.x(), min.y(), min.z());
    const QVector3D _2 = QVector3D(min.x(), max.y(), min.z());
    const QVector3D _3 = QVector3D(max.x(), max.y(), min.z());
    const QVector3D _4 = QVector3D(min.x(), min.y(), max.z());
    const QVector3D _5 = QVector3D(max.x(), min.y(), max.z());
    const QVector3D _6 = QVector3D(min.x(), max.y(), max.z());
    const QVector3D _7 = QVector3D(max.x(), max.y(), max.z());

    addDebugLine(_0, _1, color);
    addDebugLine(_1, _3, color);
    addDebugLine(_3, _2, color);
    addDebugLine(_2, _0, color);

    addDebugLine(_4, _5, color);
    addDebugLine(_5, _7, color);
    addDebugLine(_7, _6, color);
    addDebugLine(_6, _4, color);

    addDebugLine(_0, _4, color);
    addDebugLine(_1, _5, color);
    addDebugLine(_3, _7, color);
    addDebugLine(_2, _6, color);
}

void GraphScene::submitDebugLines()
{
    QMutexLocker locker(&debugLinesMutex);

    debugLinesData.resize(debugLines.size() * 12);

    int i = 0;
    for(const DebugLine debugLine : debugLines)
    {
        debugLinesData[i++] = debugLine.start.x();
        debugLinesData[i++] = debugLine.start.y();
        debugLinesData[i++] = debugLine.start.z();
        debugLinesData[i++] = debugLine.color.redF();
        debugLinesData[i++] = debugLine.color.greenF();
        debugLinesData[i++] = debugLine.color.blueF();
        debugLinesData[i++] = debugLine.end.x();
        debugLinesData[i++] = debugLine.end.y();
        debugLinesData[i++] = debugLine.end.z();
        debugLinesData[i++] = debugLine.color.redF();
        debugLinesData[i++] = debugLine.color.greenF();
        debugLinesData[i++] = debugLine.color.blueF();
    }
}

static void setShaderADSParameters(QOpenGLShaderProgram& program)
{
    struct Light
    {
        Light() {}
        Light(const QVector4D& _position, const QVector3D& _intensity) :
            position(_position), intensity(_intensity)
        {}

        QVector4D position;
        QVector3D intensity;
    };

    QVector<Light> lights;
    lights.append(Light(QVector4D(-20.0f, 0.0f, 3.0f, 1.0f), QVector3D(0.6f, 0.6f, 0.6f)));
    lights.append(Light(QVector4D(0.0f, 0.0f, 0.0f, 1.0f), QVector3D(0.2f, 0.2f, 0.2f)));
    lights.append(Light(QVector4D(10.0f, -10.0f, -10.0f, 1.0f), QVector3D(0.4f, 0.4f, 0.4f)));

    int numberOfLights = lights.size();

    program.setUniformValue("numberOfLights", numberOfLights);

    for(int i = 0; i < numberOfLights; i++)
    {
        QByteArray positionId = QString("lights[%1].position").arg(i).toLatin1();
        program.setUniformValue(positionId.data(), lights[i].position);

        QByteArray intensityId = QString("lights[%1].intensity").arg(i).toLatin1();
        program.setUniformValue(intensityId.data(), lights[i].intensity);
    }

    program.setUniformValue("material.ks", QVector3D(1.0f, 1.0f, 1.0f));
    program.setUniformValue("material.ka", QVector3D(0.1f, 0.1f, 0.1f));
    program.setUniformValue("material.shininess", 50.0f);
}

void GraphScene::render()
{
    if(!FBOcomplete)
        return;

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    m_funcs->glBindFramebuffer(GL_FRAMEBUFFER, visualFBO);

    m_funcs->glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    m_funcs->glDrawBuffer(GL_COLOR_ATTACHMENT1);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    m_funcs->glDrawBuffers(2, drawBuffers);
    glClear(GL_DEPTH_BUFFER_BIT);

    nodesShader.bind();
    setShaderADSParameters(nodesShader);
    renderNodes(nodesShader);
    nodesShader.release();

    edgesShader.bind();
    setShaderADSParameters(edgesShader);
    renderEdges(edgesShader);
    edgesShader.release();

    //renderComponentMarkers();
    renderDebugLines();

    m_funcs->glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_funcs->glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    m_funcs->glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    m_funcs->glDisable(GL_DEPTH_TEST);

    screenQuadVAO.bind();
    m_funcs->glActiveTexture(GL_TEXTURE0);

    // Color texture
    m_funcs->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, colorTexture);

    screenShader.bind();
    m_funcs->glDrawArrays(GL_TRIANGLES, 0, 6);
    screenShader.release();

    // Selection texture
    m_funcs->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, selectionTexture);

    selectionShader.bind();
    m_funcs->glDrawArrays(GL_TRIANGLES, 0, 6);
    selectionShader.release();

    screenQuadVAO.release();

    m_funcs->glEnable(GL_DEPTH_TEST);

    glDisable(GL_BLEND);
}

void GraphScene::resize(int w, int h)
{
    width = w;
    height = h;

    qDebug() << "GraphScene::resize(" << w << h << ")";

    FBOcomplete = prepareRenderBuffers();

    glViewport(0, 0, w, h);

    aspectRatio = static_cast<float>(w) / static_cast<float>(h);

    if(_graphModel != nullptr)
    {
        for(ComponentId componentId : *_graphModel->graph().componentIds())
        {
            ComponentViewData* componentViewData = &(*componentsViewData)[componentId];
            setupCamera(componentViewData->camera, aspectRatio);
        }
    }
}

const float MINIMUM_CAMERA_DISTANCE = 2.5f;

void GraphScene::zoom(float direction)
{
    if(direction == 0.0f || !panTransition.finished())
        return;

    ComponentViewData* componentViewData = focusComponentViewData();
    float size = _graphModel->nodeVisuals()[componentViewData->focusNodeId].size;

    const float INTERSECTION_AVOIDANCE_OFFSET = 1.0f;
    const float ZOOM_STEP_FRACTION = 0.2f;
    float delta = (targetZoomDistance - size - INTERSECTION_AVOIDANCE_OFFSET) * ZOOM_STEP_FRACTION;

    targetZoomDistance -= delta * direction;

    if(targetZoomDistance < MINIMUM_CAMERA_DISTANCE)
        targetZoomDistance = MINIMUM_CAMERA_DISTANCE;

    float startZoomDistance = componentViewData->zoomDistance;
    zoomTransition.start(0.1f, Transition::Type::Linear,
    [=](float f)
    {
        componentViewData->zoomDistance = startZoomDistance + ((targetZoomDistance - startZoomDistance) * f);
    });
}

void GraphScene::centreNodeInViewport(NodeId nodeId, Transition::Type transitionType, float cameraDistance)
{
    if(nodeId.isNull())
        return;

    const QVector3D& nodePosition = _graphModel->nodePositions()[nodeId];
    Plane translationPlane(nodePosition, m_camera->viewVector().normalized());

    QVector3D curPoint = translationPlane.rayIntersection(
                Ray(m_camera->position(), m_camera->viewVector().normalized()));

    QVector3D translation = nodePosition - curPoint;
    QVector3D startPosition = m_camera->position();
    QVector3D startViewTarget = m_camera->viewTarget();
    QVector3D targetPosition = m_camera->position() + translation;

    if(cameraDistance >= 0.0f)
        targetPosition = nodePosition - (m_camera->viewVector().normalized() * cameraDistance);
    else
        focusComponentViewData()->zoomDistance = targetZoomDistance = translationPlane.distanceToPoint(targetPosition);

    if(targetPosition.distanceToPoint(nodePosition) < MINIMUM_CAMERA_DISTANCE)
        targetPosition = nodePosition - (m_camera->viewVector().normalized() * MINIMUM_CAMERA_DISTANCE);

    if(transitionType != Transition::Type::None)
    {
        emit userInteractionStarted();
        panTransition.start(0.3f, transitionType,
        [=](float f)
        {
            m_camera->setPosition(startPosition + ((targetPosition - startPosition) * f));
            m_camera->setViewTarget(startViewTarget + ((nodePosition - startViewTarget) * f));
        },
        [=]()
        {
            emit userInteractionFinished();
        });
    }
    else
    {
        m_camera->setPosition(targetPosition);
        m_camera->setViewTarget(nodePosition);
    }
}

void GraphScene::selectFocusNode(ComponentId componentId, Transition::Type transitionType)
{
    if(componentId.isNull())
        return;

    Collision collision(*_graphModel->graph().componentById(componentId),
                        _graphModel->nodeVisuals(), _graphModel->nodePositions());
    NodeId closestNodeId = collision.closestNodeToLine(m_camera->position(), m_camera->viewVector().normalized());
    if(!closestNodeId.isNull())
    {
        centreNodeInViewport(closestNodeId, transitionType);
        focusComponentViewData()->focusNodeId = closestNodeId;
        updateVisualData();
    }
}

void GraphScene::mousePressEvent(QMouseEvent* mouseEvent)
{
    switch(mouseEvent->button())
    {
    case Qt::LeftButton:
        m_leftMouseButtonHeld = true;
        m_selecting = true;
        break;

    case Qt::RightButton:
        m_rightMouseButtonHeld = true;
        break;

    default: break;
    }

    m_pos = m_prevPos = mouseEvent->pos();

    Ray ray = m_camera->rayForViewportCoordinates(m_pos.x(), m_pos.y());

    const ReadOnlyGraph& component = *_graphModel->graph().componentById(focusComponentId);

    Collision collision(component, _graphModel->nodeVisuals(), _graphModel->nodePositions());
    clickedNodeId = collision.nearestNodeIntersectingLine(ray.origin(), ray.dir());

    if(m_shiftKeyHeld)
        m_frustumSelectStart = m_pos;
}

void GraphScene::mouseReleaseEvent(QMouseEvent* mouseEvent)
{
    switch(mouseEvent->button())
    {
    case Qt::RightButton:
        if(!panTransition.finished())
        {
            m_rightMouseButtonHeld = false;
            return;
        }

        emit userInteractionFinished();

        if(m_rightMouseButtonHeld && m_mouseMoving)
            selectFocusNode(focusComponentId, Transition::Type::InversePower);

        m_rightMouseButtonHeld = false;
        clickedNodeId.setToNull();
        break;

    case Qt::LeftButton:
        m_leftMouseButtonHeld = false;

        if(!panTransition.finished())
            return;

        emit userInteractionFinished();

        if(m_selecting)
        {
            if(m_frustumSelecting)
            {
                if(!m_shiftKeyHeld)
                    _selectionManager->clearNodeSelection();

                QPoint frustumEndPoint = mouseEvent->pos();
                Frustum frustum = m_camera->frustumForViewportCoordinates(
                            m_frustumSelectStart.x(), m_frustumSelectStart.y(),
                            frustumEndPoint.x(), frustumEndPoint.y());

                QSet<NodeId> selection;

                const ReadOnlyGraph& component = *_graphModel->graph().componentById(focusComponentId);
                for(NodeId nodeId : component.nodeIds())
                {
                    const QVector3D& nodePosition = _graphModel->nodePositions()[nodeId];
                    if(frustum.containsPoint(nodePosition))
                        selection.insert(nodeId);
                }

                _selectionManager->selectNodes(selection);

                m_frustumSelecting = false;
            }
            else
            {
                if(!clickedNodeId.isNull())
                {
                    if(!m_shiftKeyHeld)
                        _selectionManager->clearNodeSelection();

                    _selectionManager->toggleNode(clickedNodeId);
                }
                else
                    _selectionManager->clearNodeSelection();
            }

            m_selecting = false;
        }

        clickedNodeId.setToNull();
        break;

    default: break;
    }

    if(m_mouseMoving && !m_rightMouseButtonHeld && !m_leftMouseButtonHeld)
        m_mouseMoving = false;
}

void GraphScene::mouseMoveEvent(QMouseEvent* mouseEvent)
{
    if(!panTransition.finished())
        return;

    m_pos = mouseEvent->pos();

    if(m_shiftKeyHeld && m_leftMouseButtonHeld)
    {
        emit userInteractionStarted();
        m_selecting = true;
        m_frustumSelecting = true;
        clickedNodeId.setToNull();
    }
    else if(!clickedNodeId.isNull())
    {
        m_selecting = false;

        if(m_rightMouseButtonHeld)
        {
            emit userInteractionStarted();

            const QVector3D& clickedNodePosition = _graphModel->nodePositions()[clickedNodeId];

            Plane translationPlane(clickedNodePosition, m_camera->viewVector().normalized());

            QVector3D prevPoint = translationPlane.rayIntersection(
                        m_camera->rayForViewportCoordinates(m_prevPos.x(), m_prevPos.y()));
            QVector3D curPoint = translationPlane.rayIntersection(
                        m_camera->rayForViewportCoordinates(m_pos.x(), m_pos.y()));
            QVector3D translation = prevPoint - curPoint;

            m_camera->translateWorld(translation);
        }
        else if(m_leftMouseButtonHeld)
        {
            emit userInteractionStarted();

            ComponentViewData* componentViewData = focusComponentViewData();

            if(componentViewData->focusNodeId.isNull())
                selectFocusNode(focusComponentId, Transition::Type::InversePower);

            if(componentViewData->focusNodeId != clickedNodeId)
            {
                emit userInteractionStarted();

                const QVector3D& clickedNodePosition = _graphModel->nodePositions()[clickedNodeId];
                const QVector3D& rotationCentre = _graphModel->nodePositions()[componentViewData->focusNodeId];
                float radius = clickedNodePosition.distanceToPoint(rotationCentre);

                BoundingSphere boundingSphere(rotationCentre, radius);
                QVector3D cursorPoint;
                Ray cursorRay = m_camera->rayForViewportCoordinates(m_pos.x(), m_pos.y());

                Plane divisionPlane(rotationCentre, cursorRay.dir().normalized());

                QList<QVector3D> intersections = boundingSphere.rayIntersection(cursorRay);
                if(intersections.size() > 0)
                {
                    if(divisionPlane.sideForPoint(clickedNodePosition) == Plane::Side::Front && intersections.size() > 1)
                        cursorPoint = intersections.at(1);
                    else
                        cursorPoint = intersections.at(0);
                }
                else
                {
                    // When the ray misses the node completely we clamp the cursor point on the surface of the sphere
                    QVector3D cameraToCentre = rotationCentre - m_camera->position();
                    float cameraToCentreLengthSq = cameraToCentre.lengthSquared();
                    float radiusSq = radius * radius;
                    float cameraToClickedLengthSq = cameraToCentreLengthSq - radiusSq;

                    // Form a right angled triangle from the camera, the circle tangent and a point which lies on the
                    // camera to rotation centre vector
                    float adjacentLength = (cameraToClickedLengthSq - radiusSq + cameraToCentreLengthSq) /
                            (2.0f * cameraToCentre.length());
                    float oppositeLength = sqrt(cameraToClickedLengthSq - (adjacentLength * adjacentLength));
                    QVector3D corner = m_camera->position() + ((adjacentLength / cameraToCentre.length()) * cameraToCentre);

                    Plane p(m_camera->position(), rotationCentre, cursorRay.origin());
                    QVector3D oppositeDir = QVector3D::crossProduct(p.normal(), cameraToCentre).normalized();

                    cursorPoint = corner + (oppositeLength * oppositeDir);
                }

                QVector3D clickedLine = clickedNodePosition - rotationCentre;
                QVector3D cursorLine = cursorPoint - rotationCentre;

                QVector3D axis = QVector3D::crossProduct(clickedLine, cursorLine).normalized();
                float dot = QVector3D::dotProduct(clickedLine, cursorLine);
                float value = dot / (clickedLine.length() * cursorLine.length());
                value = Utils::clamp(-1.0f, 1.0f, value);
                float radians = std::acos(value);
                float angle = -qRadiansToDegrees(radians);

                QQuaternion rotation = QQuaternion::fromAxisAndAngle(axis, angle);
                m_camera->rotateAboutViewTarget(rotation);
            }
        }
    }

    m_prevPos = m_pos;

    if(m_rightMouseButtonHeld || m_leftMouseButtonHeld)
        m_mouseMoving = true;
}

void GraphScene::mouseDoubleClickEvent(QMouseEvent* mouseEvent)
{
    if(mouseEvent->button() == Qt::LeftButton)
    {
        if(!clickedNodeId.isNull() && !m_mouseMoving)
        {
            centreNodeInViewport(clickedNodeId, Transition::Type::EaseInEaseOut);
            focusComponentViewData()->focusNodeId = clickedNodeId;
            updateVisualData();
        }
    }
}

bool GraphScene::keyPressEvent(QKeyEvent* keyEvent)
{
    switch(keyEvent->key())
    {
    case Qt::Key_Shift:
        m_shiftKeyHeld = true;
        if(m_leftMouseButtonHeld)
            m_frustumSelectStart = m_pos;
        return true;

    case Qt::Key_Control:
        m_controlKeyHeld = true;
        return true;

    case Qt::Key_Delete:
        _graphModel->graph().removeNodes(_selectionManager->selectedNodes());
        _selectionManager->clearNodeSelection();
        return true;

    case Qt::Key_Left:
        moveToNextComponent();
        return true;

    case Qt::Key_Right:
        moveToPreviousComponent();
        return true;

    default:
        return false;
    }
}

bool GraphScene::keyReleaseEvent(QKeyEvent* keyEvent)
{
    switch(keyEvent->key())
    {
    case Qt::Key_Shift:
        m_shiftKeyHeld = false;
        return true;

    case Qt::Key_Control:
        m_controlKeyHeld = false;
        return true;

    default:
        return false;
    }
}

static ComponentId cycleThroughComponentIds(const QList<ComponentId>& componentIds,
                                            ComponentId currentComponentId, int amount)
{
    int numComponents = componentIds.size();

    for(int i = 0; i < numComponents; i++)
    {
        if(componentIds.at(i) == currentComponentId)
        {
            int nextIndex = (i + amount + numComponents) % numComponents;
            return componentIds.at(nextIndex);
        }
    }

    if(numComponents > 0)
        return componentIds.at(0);

    return ComponentId();
}

void GraphScene::moveToNextComponent()
{
    focusComponentId = cycleThroughComponentIds(*_graphModel->graph().componentIds(), focusComponentId, -1);

    if(!focusComponentId.isNull())
    {
        updateVisualData();
        targetZoomDistance = focusComponentViewData()->zoomDistance;
    }
}

void GraphScene::moveToPreviousComponent()
{
    focusComponentId = cycleThroughComponentIds(*_graphModel->graph().componentIds(), focusComponentId, 1);

    if(!focusComponentId.isNull())
    {
        updateVisualData();
        targetZoomDistance = focusComponentViewData()->zoomDistance;
    }
}

void GraphScene::moveToComponent(ComponentId componentId)
{
    focusComponentId = componentId;

    if(!focusComponentId.isNull())
    {
        updateVisualData();
        targetZoomDistance = focusComponentViewData()->zoomDistance;
    }
}

void GraphScene::wheelEvent(QWheelEvent* wheelEvent)
{
    if(wheelEvent->angleDelta().y() > 0.0f)
        zoom(1.0f);
    else
        zoom(-1.0f);
}

void GraphScene::setGraphModel(GraphModel* graphModel)
{
    this->_graphModel = graphModel;

    if(componentsViewData != nullptr)
        delete componentsViewData;

    componentsViewData = new ComponentArray<ComponentViewData>(_graphModel->graph());
    focusComponentId = _graphModel->graph().firstComponentId();

    for(ComponentId componentId : *_graphModel->graph().componentIds())
        onComponentAdded(&_graphModel->graph(), componentId);

    updateVisualData();
    connect(&_graphModel->graph(), &Graph::graphChanged, this, &GraphScene::onGraphChanged);
    connect(&_graphModel->graph(), &Graph::nodeWillBeRemoved, this, &GraphScene::onNodeWillBeRemoved);
    connect(&_graphModel->graph(), &Graph::componentAdded, this, &GraphScene::onComponentAdded);
    connect(&_graphModel->graph(), &Graph::componentWillBeRemoved, this, &GraphScene::onComponentWillBeRemoved);
    connect(&_graphModel->graph(), &Graph::componentSplit, this, &GraphScene::onComponentSplit);
    connect(&_graphModel->graph(), &Graph::componentsWillMerge, this, &GraphScene::onComponentsWillMerge);
}

void GraphScene::setSelectionManager(SelectionManager* selectionManager)
{
    this->_selectionManager = selectionManager;
    connect(selectionManager, &SelectionManager::selectionChanged, this, &GraphScene::onSelectionChanged);
}

void GraphScene::prepareVertexBuffers()
{
    // Populate the data buffer object
    m_nodePositionBuffer.create();
    m_nodePositionBuffer.setUsagePattern( QOpenGLBuffer::DynamicDraw );
    m_nodePositionBuffer.bind();
    m_nodePositionBuffer.allocate( m_nodePositionData.data(), m_nodePositionData.size() * sizeof(GLfloat) );

    m_edgePositionBuffer.create();
    m_edgePositionBuffer.setUsagePattern( QOpenGLBuffer::DynamicDraw );
    m_edgePositionBuffer.bind();
    m_edgePositionBuffer.allocate( m_edgePositionData.data(), m_edgePositionData.size() * sizeof(GLfloat) );

    m_nodeVisualBuffer.create();
    m_nodeVisualBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    m_nodeVisualBuffer.bind();
    m_nodeVisualBuffer.allocate( m_nodeVisualData.data(), m_nodeVisualData.size() * sizeof(GLfloat) );

    m_edgeVisualBuffer.create();
    m_edgeVisualBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    m_edgeVisualBuffer.bind();
    m_edgeVisualBuffer.allocate( m_edgeVisualData.data(), m_edgeVisualData.size() * sizeof(GLfloat) );

    m_componentMarkerDataBuffer.create();
    m_componentMarkerDataBuffer.setUsagePattern( QOpenGLBuffer::DynamicDraw );
    m_componentMarkerDataBuffer.bind();
    m_componentMarkerDataBuffer.allocate( m_componentMarkerData.data(), m_componentMarkerData.size() * sizeof(GLfloat) );

    debugLinesDataBuffer.create();
    debugLinesDataBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    debugLinesDataBuffer.bind();
    debugLinesDataBuffer.allocate(debugLinesData.data(), debugLinesData.size() * sizeof(GLfloat));
}

void GraphScene::prepareNodeVAO()
{
    // Bind the marker's VAO
    m_sphere->vertexArrayObject()->bind();

    // Enable the data buffer and add it to the marker's VAO
    QOpenGLShaderProgramPtr shader = m_sphere->material()->shader();
    shader->bind();
    m_nodePositionBuffer.bind();
    shader->enableAttributeArray("point");
    shader->setAttributeBuffer("point", GL_FLOAT, 0, 3);

    m_nodeVisualBuffer.bind();
    shader->enableAttributeArray("size");
    shader->enableAttributeArray("color");
    shader->enableAttributeArray("outlineColor");
    shader->setAttributeBuffer("size", GL_FLOAT, 0, 1, 8 * sizeof(GLfloat));
    shader->setAttributeBuffer("color", GL_FLOAT, 1 * sizeof(GLfloat), 3, 8 * sizeof(GLfloat));
    shader->setAttributeBuffer("outlineColor", GL_FLOAT, 4 * sizeof(GLfloat), 4, 8 * sizeof(GLfloat));

    // We only vary the point attribute once per instance
    GLuint pointLocation = shader->attributeLocation("point");
    GLuint sizeLocation = shader->attributeLocation("size");
    GLuint colorLocation = shader->attributeLocation("color");
    GLuint outlineColorLocation = shader->attributeLocation("outlineColor");
#if !defined(Q_OS_MAC)
    m_funcs->glVertexAttribDivisor(pointLocation, 1);
    m_funcs->glVertexAttribDivisor(sizeLocation, 1);
    m_funcs->glVertexAttribDivisor(colorLocation, 1);
    m_funcs->glVertexAttribDivisor(outlineColorLocation, 1);
#else
    m_instanceFuncs->glVertexAttribDivisorARB(pointLocation, 1);
    m_instanceFuncs->glVertexAttribDivisorARB(sizeLocation, 1);
    m_instanceFuncs->glVertexAttribDivisorARB(colorLocation, 1);
    m_instanceFuncs->glVertexAttribDivisorARB(outlineColorLocation, 1);
#endif
    m_sphere->vertexArrayObject()->release();
    shader->release();
}

void GraphScene::prepareEdgeVAO()
{
    // Bind the marker's VAO
    m_cylinder->vertexArrayObject()->bind();

    // Enable the data buffer and add it to the marker's VAO
    QOpenGLShaderProgramPtr shader = m_cylinder->material()->shader();
    shader->bind();
    m_edgePositionBuffer.bind();
    shader->enableAttributeArray("source");
    shader->enableAttributeArray("target");
    shader->setAttributeBuffer("source", GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));
    shader->setAttributeBuffer("target", GL_FLOAT, 3 * sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

    m_edgeVisualBuffer.bind();
    shader->enableAttributeArray("size");
    shader->enableAttributeArray("color");
    shader->enableAttributeArray("outlineColor");
    shader->setAttributeBuffer("size", GL_FLOAT, 0, 1, 8 * sizeof(GLfloat));
    shader->setAttributeBuffer("color", GL_FLOAT, 1 * sizeof(GLfloat), 3, 8 * sizeof(GLfloat));
    shader->setAttributeBuffer("outlineColor", GL_FLOAT, 4 * sizeof(GLfloat), 4, 8 * sizeof(GLfloat));

    // We only vary the point attribute once per instance
    GLuint sourcePointLocation = shader->attributeLocation("source");
    GLuint targetPointLocation = shader->attributeLocation("target");
    GLuint sizeLocation = shader->attributeLocation("size");
    GLuint colorLocation = shader->attributeLocation("color");
    GLuint outlineColorLocation = shader->attributeLocation("outlineColor");
#if !defined(Q_OS_MAC)
    m_funcs->glVertexAttribDivisor(sourcePointLocation, 1);
    m_funcs->glVertexAttribDivisor(targetPointLocation, 1);
    m_funcs->glVertexAttribDivisor(sizeLocation, 1);
    m_funcs->glVertexAttribDivisor(colorLocation, 1);
    m_funcs->glVertexAttribDivisor(outlineColorLocation, 1);
#else
    m_instanceFuncs->glVertexAttribDivisorARB(sourcePointLocation, 1);
    m_instanceFuncs->glVertexAttribDivisorARB(targetPointLocation, 1);
    m_instanceFuncs->glVertexAttribDivisorARB(sizeLocation, 1);
    m_instanceFuncs->glVertexAttribDivisorARB(colorLocation, 1);
    m_instanceFuncs->glVertexAttribDivisorARB(outlineColorLocation, 1);
#endif
    m_cylinder->vertexArrayObject()->release();
    shader->release();
}

void GraphScene::prepareComponentMarkerVAO()
{
    // Bind the marker's VAO
    m_quad->vertexArrayObject()->bind();

    // Enable the data buffer and add it to the marker's VAO
    QOpenGLShaderProgramPtr shader = m_quad->material()->shader();
    shader->bind();
    m_componentMarkerDataBuffer.bind();
    shader->enableAttributeArray("point");
    shader->enableAttributeArray("scale");
    shader->setAttributeBuffer("point", GL_FLOAT, 0, 2, 3 * sizeof(GLfloat));
    shader->setAttributeBuffer("scale", GL_FLOAT, 2 * sizeof(GLfloat), 1, 3 * sizeof(GLfloat));

    // We only vary the point attribute once per instance
    GLuint pointLocation = shader->attributeLocation("point");
    GLuint scaleLocation = shader->attributeLocation("scale");
#if !defined(Q_OS_MAC)
    m_funcs->glVertexAttribDivisor(pointLocation, 1);
    m_funcs->glVertexAttribDivisor(scaleLocation, 1);
#else
    m_instanceFuncs->glVertexAttribDivisorARB(pointLocation, 1);
    m_instanceFuncs->glVertexAttribDivisorARB(scaleLocation, 1);
#endif
    m_quad->vertexArrayObject()->release();
    shader->release();
}

void GraphScene::prepareDebugLinesVAO()
{
    debugLinesDataVAO.bind();
    debugLinesShader.bind();
    debugLinesDataBuffer.bind();

    debugLinesShader.enableAttributeArray("position");
    debugLinesShader.enableAttributeArray("color");
    debugLinesShader.setAttributeBuffer("position", GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));
    debugLinesShader.setAttributeBuffer("color", GL_FLOAT, 3 * sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

    debugLinesDataVAO.release();
    debugLinesShader.release();
}

void GraphScene::prepareScreenQuad()
{
    GLfloat quadVerts[] =
    {
         1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f,  1.0f,

        -1.0f,  1.0f,
        -1.0f, -1.0f,
         1.0f, -1.0f,
    };
    int quadVertsSize = sizeof(quadVerts);
    QOpenGLBuffer quadBuffer;

    screenQuadVAO.create();
    screenQuadVAO.bind();

    quadBuffer.create();
    quadBuffer.bind();
    quadBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    quadBuffer.allocate(quadVerts, quadVertsSize);

    loadShaderProgram(screenShader, ":/gl/shaders/screen.vert", ":/gl/shaders/screen.frag");
    screenShader.bind();
    screenShader.enableAttributeArray("vertexPosition");
    screenShader.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 2, 2 * sizeof(GLfloat));
    screenShader.setUniformValue("frameBufferTexture", 0);
    screenShader.release();

    loadShaderProgram(selectionShader, ":/gl/shaders/screen.vert", ":/gl/shaders/selection.frag");
    selectionShader.bind();
    selectionShader.enableAttributeArray("vertexPosition");
    selectionShader.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 2, 2 * sizeof(GLfloat));
    selectionShader.setUniformValue("frameBufferTexture", 0);
    selectionShader.release();

    quadBuffer.release();
    screenQuadVAO.release();
}

bool GraphScene::loadShaderProgram(QOpenGLShaderProgram& program, const QString& vertexShader, const QString& fragmentShader)
{
    if(!program.addShaderFromSourceFile(QOpenGLShader::Vertex, vertexShader))
    {
        qCritical() << QObject::tr("Could not compile vertex shader. Log:") << program.log();
        return false;
    }

    if(!program.addShaderFromSourceFile(QOpenGLShader::Fragment, fragmentShader))
    {
        qCritical() << QObject::tr("Could not compile fragment shader. Log:") << program.log();
        return false;
    }

    if(!program.link())
    {
        qCritical() << QObject::tr("Could not link shader program. Log:") << program.log();
        return false;
    }

    return true;
}

bool GraphScene::prepareRenderBuffers()
{
    bool valid;

    // Color texture
    if(colorTexture == 0)
        m_funcs->glGenTextures(1, &colorTexture);
    m_funcs->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, colorTexture);
    m_funcs->glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, multisamples, GL_RGBA, width, height, GL_FALSE);
    m_funcs->glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
    m_funcs->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

    // Selection texture
    if(selectionTexture == 0)
        m_funcs->glGenTextures(1, &selectionTexture);
    m_funcs->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, selectionTexture);
    m_funcs->glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, multisamples, GL_RGBA, width, height, GL_FALSE);
    m_funcs->glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
    m_funcs->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

    // Depth texture
    if(depthTexture == 0)
        m_funcs->glGenTextures(1, &depthTexture);
    m_funcs->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, depthTexture);
    m_funcs->glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, multisamples, GL_DEPTH_COMPONENT, width, height, GL_FALSE);
    m_funcs->glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
    m_funcs->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

    // Visual FBO
    if(visualFBO == 0)
        m_funcs->glGenFramebuffers(1, &visualFBO);
    m_funcs->glBindFramebuffer(GL_FRAMEBUFFER, visualFBO);
    m_funcs->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, colorTexture, 0);
    m_funcs->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, selectionTexture, 0);
    m_funcs->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, depthTexture, 0);

    GLenum status = m_funcs->glCheckFramebufferStatus(GL_FRAMEBUFFER);
    valid = (status == GL_FRAMEBUFFER_COMPLETE);

    m_funcs->glBindFramebuffer(GL_FRAMEBUFFER, 0);

    Q_ASSERT(valid);
    return valid;
}

ComponentViewData::ComponentViewData() :
    zoomDistance(50.0f),
    initialised(false)
{
    camera.setPosition(QVector3D(0.0f, 0.0f, 50.0f));
    camera.setViewTarget(QVector3D(0.0f, 0.0f, 0.0f));
    camera.setUpVector(QVector3D(0.0f, 1.0f, 0.0f));
}

ComponentViewData::ComponentViewData(const ComponentViewData& other) :
    camera(other.camera),
    zoomDistance(other.zoomDistance),
    focusNodeId(other.focusNodeId),
    initialised(other.initialised)
{
}
