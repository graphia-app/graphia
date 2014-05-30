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
      _width(0), _height(0),
      _colorTexture(0),
      _selectionTexture(0),
      _depthTexture(0),
      _visualFBO(0),
      _FBOcomplete(false),
      _trackFocusNode(true),
      _funcs(nullptr),
#if defined(Q_OS_MAC)
      _instanceFuncs( 0 ),
#endif
      _componentsViewData(nullptr),
      _aspectRatio(4.0f / 3.0f),
      _camera(nullptr),
      _sphere(nullptr),
      _cylinder(nullptr),
      _quad(nullptr),
      _graphModel(nullptr),
      _nodePositionData(0),
      _edgePositionData(0),
      _nodeVisualData(0),
      _edgeVisualData(0),
      _componentMarkerData(0),
      _debugLinesData(0)
{
    update(0.0f);
}

void GraphScene::initialise()
{
    // Resolve the OpenGL functions that we need for instanced rendering
#if !defined(Q_OS_MAC)
    _funcs = _context->versionFunctions<QOpenGLFunctions_3_3_Core>();
#else
    _instanceFuncs = new QOpenGLExtension_ARB_instanced_arrays;
    if ( !_instanceFuncs->initializeOpenGLFunctions() )
        qFatal( "Could not resolve GL_ARB_instanced_arrays functions" );

    _funcs = _context->versionFunctions<QOpenGLFunctions_3_2_Core>();
#endif
    if ( !_funcs )
        qFatal( "Could not obtain required OpenGL context version" );
    _funcs->initializeOpenGLFunctions();

    MaterialPtr nodeMaterial(new Material);
    nodeMaterial->setShaders(":/gl/shaders/instancednodes.vert", ":/gl/shaders/ads.frag" );
    loadShaderProgram(_nodesShader, ":/gl/shaders/instancednodes.vert", ":/gl/shaders/ads.frag");

    // Create a sphere
    _sphere = new Sphere( this );
    _sphere->setRadius(1.0f);
    _sphere->setRings(16);
    _sphere->setSlices(16);
    _sphere->setMaterial(nodeMaterial);
    _sphere->create();

    MaterialPtr edgeMaterial(new Material);
    edgeMaterial->setShaders(":/gl/shaders/instancededges.vert", ":/gl/shaders/ads.frag" );
    loadShaderProgram(_edgesShader, ":/gl/shaders/instancededges.vert", ":/gl/shaders/ads.frag");

    _cylinder = new Cylinder(this);
    _cylinder->setRadius(1.0f);
    _cylinder->setLength(1.0f);
    _cylinder->setSlices(8);
    _cylinder->setMaterial(edgeMaterial);
    _cylinder->create();

    MaterialPtr componentMarkerMaterial(new Material);
    componentMarkerMaterial->setShaders(":/gl/shaders/instancedmarkers.vert", ":/gl/shaders/marker.frag" );
    loadShaderProgram(_componentMarkerShader, ":/gl/shaders/instancedmarkers.vert", ":/gl/shaders/marker.frag");

    _quad = new Quad(this);
    _quad->setEdgeLength(1.0f);
    _quad->setMaterial(componentMarkerMaterial);
    _quad->create();

    _debugLinesDataVAO.create();
    loadShaderProgram(_debugLinesShader, ":/gl/shaders/debuglines.vert", ":/gl/shaders/debuglines.frag");

    _selectionMarkerDataVAO.create();
    loadShaderProgram(_selectionMarkerShader, ":/gl/shaders/2d.vert", ":/gl/shaders/selectionMarker.frag");

    // Create a pair of VBOs ready to hold our data
    prepareVertexBuffers();

    // Tell OpenGL how to pass the data VBOs to the shader program
    prepareNodeVAO();
    prepareEdgeVAO();
    prepareComponentMarkerVAO();
    prepareSelectionMarkerVAO();
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
    const ReadOnlyGraph& component = *_graphModel->graph().componentById(_focusComponentId);

    _nodeVisualData.resize(component.numNodes() * 8);
    _edgeVisualData.resize(component.numEdges() * 8);
    int i = 0;
    int j = 0;

    for(NodeId nodeId : component.nodeIds())
    {
        _nodeVisualData[i++] = nodeVisuals[nodeId]._size;
        _nodeVisualData[i++] = nodeVisuals[nodeId]._color.redF();
        _nodeVisualData[i++] = nodeVisuals[nodeId]._color.greenF();
        _nodeVisualData[i++] = nodeVisuals[nodeId]._color.blueF();
        _nodeVisualData[i++] = nodeVisuals[nodeId]._outlineColor.redF();
        _nodeVisualData[i++] = nodeVisuals[nodeId]._outlineColor.greenF();
        _nodeVisualData[i++] = nodeVisuals[nodeId]._outlineColor.blueF();
    }

    for(EdgeId edgeId : component.edgeIds())
    {
        _edgeVisualData[j++] = edgeVisuals[edgeId]._size;
        _edgeVisualData[j++] = edgeVisuals[edgeId]._color.redF();
        _edgeVisualData[j++] = edgeVisuals[edgeId]._color.greenF();
        _edgeVisualData[j++] = edgeVisuals[edgeId]._color.blueF();
        _edgeVisualData[j++] = edgeVisuals[edgeId]._outlineColor.redF();
        _edgeVisualData[j++] = edgeVisuals[edgeId]._outlineColor.greenF();
        _edgeVisualData[j++] = edgeVisuals[edgeId]._outlineColor.blueF();
    }
}

void GraphScene::onGraphChanged(const Graph*)
{
    if(_focusComponentId.isNull())
    {
        // The component we were focused on has gone, we need to find a new one
        moveToNextComponent();
    }

    updateVisualData();
}

void GraphScene::onNodeWillBeRemoved(const Graph*, NodeId nodeId)
{
    ComponentViewData* currentComponentViewData = focusComponentViewData();
    if(currentComponentViewData->_focusNodeId == nodeId)
        currentComponentViewData->_focusNodeId.setToNull();
}

static void setupCamera(Camera& camera, float aspectRatio)
{
    camera.setPerspectiveProjection(60.0f, aspectRatio, 0.3f, 10000.0f);
}

void GraphScene::onComponentAdded(const Graph*, ComponentId componentId)
{
    ComponentViewData* componentViewData = &(*_componentsViewData)[componentId];

    if(!componentViewData->_initialised)
    {
        setupCamera(componentViewData->_camera, _aspectRatio);
        _targetZoomDistance = componentViewData->_zoomDistance;
        componentViewData->_focusNodeId.setToNull();
        componentViewData->_initialised = true;
    }
}

void GraphScene::onComponentWillBeRemoved(const Graph*, ComponentId componentId)
{
    ComponentViewData* componentViewData = &(*_componentsViewData)[componentId];
    componentViewData->_initialised = false;

    if(componentId == _lastSplitterFocusComponentId)
        _lastSplitterFocusComponentId.setToNull();

    if(componentId == _focusComponentId)
    {
        if(!_lastSplitterFocusComponentId.isNull())
        {
            ComponentViewData* currentComponentViewData = focusComponentViewData();
            ComponentViewData* lastSplitterComponentViewData = &(*_componentsViewData)[_lastSplitterFocusComponentId];
            *lastSplitterComponentViewData = *currentComponentViewData;
            lastSplitterComponentViewData->_focusNodeId.setToNull();
            _focusComponentId = _lastSplitterFocusComponentId;
        }
        else
            _focusComponentId.setToNull();
    }
}

void GraphScene::onComponentSplit(const Graph* graph, ComponentId oldComponentId, const QSet<ComponentId>& splitters)
{
    if(oldComponentId == _focusComponentId)
    {
        ComponentViewData currentComponentViewData = *focusComponentViewData();
        ComponentId newFocusComponentId = graph->componentIdOfNode(currentComponentViewData._focusNodeId);

        for(ComponentId splitter : splitters)
        {
            ComponentViewData* splitterComponentViewData = &(*_componentsViewData)[splitter];

            // Clone the current camera data to all splitters
            *splitterComponentViewData = currentComponentViewData;

            // Splitters that don't contain the current focus node will need to find a new one
            if(splitter != newFocusComponentId)
                splitterComponentViewData->_focusNodeId.setToNull();
        }

        _focusComponentId = newFocusComponentId;

        QSet<ComponentId> nonFocusSplitters(splitters);
        nonFocusSplitters.remove(_focusComponentId);
        _lastSplitterFocusComponentId = nonFocusSplitters.values().at(0);
    }
}

void GraphScene::onComponentsWillMerge(const Graph*, const QSet<ComponentId>& mergers, ComponentId merged)
{
    qDebug() << "onComponentsWillMerge" << mergers << merged; //FIXME leaving this in as it's untested for now
    for(ComponentId merger : mergers)
    {
        if(merger == _focusComponentId)
        {
            ComponentViewData* mergerComponentViewData = &(*_componentsViewData)[merger];
            ComponentViewData* mergedComponentViewData = &(*_componentsViewData)[merged];
            *mergedComponentViewData = *mergerComponentViewData;
            _focusComponentId = merged;
            break;
        }
    }
}

void GraphScene::onSelectionChanged(const SelectionManager& selectionManager)
{
    qDebug() << "Selection:" << selectionManager.selectedNodes();

    NodeVisuals& nodeVisuals = _graphModel->nodeVisuals();
    EdgeVisuals& edgeVisuals = _graphModel->edgeVisuals();

    for(NodeId nodeId : _graphModel->graph().nodeIds())
    {
        if(selectionManager.nodeIsSelected(nodeId))
            nodeVisuals[nodeId]._outlineColor = Qt::GlobalColor::white;
        else
            nodeVisuals[nodeId]._outlineColor = Qt::GlobalColor::black;
    }

    // Edges can't be selected at the moment
    for(EdgeId edgeId : _graphModel->graph().edgeIds())
        edgeVisuals[edgeId]._outlineColor = Qt::GlobalColor::black;

    updateVisualData();
}

ComponentViewData* GraphScene::focusComponentViewData() const
{
    if(_focusComponentId.isNull())
        return nullptr;

    return _componentsViewData != nullptr ? &(*_componentsViewData)[_focusComponentId] : nullptr;
}

void GraphScene::update(float t)
{
    if(_graphModel != nullptr)
    {
        NodePositions& nodePositions = _graphModel->nodePositions();
        QMutexLocker mutexLocker(&nodePositions.mutex());

        const ReadOnlyGraph* component = _graphModel->graph().componentById(_focusComponentId);

        ComponentViewData* componentViewData = focusComponentViewData();
        _camera = &componentViewData->_camera;
        if(componentViewData->_focusNodeId.isNull())
            selectFocusNodeClosestToCameraVector(Transition::Type::None);

        _nodePositionData.resize(component->numNodes() * 3);
        _edgePositionData.resize(component->numEdges() * 6);
        int i = 0;

        for(NodeId nodeId : component->nodeIds())
        {
            QVector3D nodePosition = nodePositions[nodeId];

            _nodePositionData[i++] = nodePosition.x();
            _nodePositionData[i++] = nodePosition.y();
            _nodePositionData[i++] = nodePosition.z();
        }

        i = 0;
        for(EdgeId edgeId : component->edgeIds())
        {
            const Edge& edge = _graphModel->graph().edgeById(edgeId);
            QVector3D sourcePosition = nodePositions[edge.sourceId()];
            QVector3D targetPosition = nodePositions[edge.targetId()];

            _edgePositionData[i++] = sourcePosition.x();
            _edgePositionData[i++] = sourcePosition.y();
            _edgePositionData[i++] = sourcePosition.z();
            _edgePositionData[i++] = targetPosition.x();
            _edgePositionData[i++] = targetPosition.y();
            _edgePositionData[i++] = targetPosition.z();
        }

#if 0
        //FIXME debug
        SpatialOctree octree(NodeLayout::boundingBox(*component, nodePositions), component->nodeIds(), nodePositions);
        octree.debugRenderOctree(this);
#endif

        _zoomTransition.update(t);

        if(!_panTransition.finished())
            _panTransition.update(t);
        else if(_trackFocusNode)
            centreNodeInViewport(componentViewData->_focusNodeId, Transition::Type::None, componentViewData->_zoomDistance);
    }

    submitDebugLines();
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

void GraphScene::renderNodes()
{
    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    _funcs->glDrawBuffers(2, drawBuffers);

    _nodesShader.bind();
    setShaderADSParameters(_nodesShader);

    const ReadOnlyGraph* component = _graphModel->graph().componentById(_focusComponentId);

    _nodePositionBuffer.bind();
    _nodePositionBuffer.allocate(_nodePositionData.data(), _nodePositionData.size() * sizeof(GLfloat) );
    _nodeVisualBuffer.bind();
    _nodeVisualBuffer.allocate(_nodeVisualData.data(), _nodeVisualData.size() * sizeof(GLfloat) );

    // Calculate needed matrices
    QMatrix4x4 modelViewMatrix = _camera->viewMatrix();
    QMatrix3x3 normalMatrix = modelViewMatrix.normalMatrix();
    _nodesShader.setUniformValue("modelViewMatrix", modelViewMatrix);
    _nodesShader.setUniformValue("normalMatrix", normalMatrix);
    _nodesShader.setUniformValue("projectionMatrix", _camera->projectionMatrix());

    // Draw the nodes
    _sphere->vertexArrayObject()->bind();
    _funcs->glDrawElementsInstanced(GL_TRIANGLES, _sphere->indexCount(),
                                     GL_UNSIGNED_INT, 0, component->numNodes());
    _sphere->vertexArrayObject()->release();

    _nodesShader.release();
}

void GraphScene::renderEdges()
{
    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    _funcs->glDrawBuffers(2, drawBuffers);

    _edgesShader.bind();
    setShaderADSParameters(_edgesShader);

    const ReadOnlyGraph* component = _graphModel->graph().componentById(_focusComponentId);

    _edgePositionBuffer.bind();
    _edgePositionBuffer.allocate(_edgePositionData.data(), _edgePositionData.size() * sizeof(GLfloat));
    _edgeVisualBuffer.bind();
    _edgeVisualBuffer.allocate(_edgeVisualData.data(), _edgeVisualData.size() * sizeof(GLfloat));

    _edgesShader.setUniformValue("viewMatrix", _camera->viewMatrix());
    _edgesShader.setUniformValue("projectionMatrix", _camera->projectionMatrix());

    // Draw the edges
    _cylinder->vertexArrayObject()->bind();
    _funcs->glDrawElementsInstanced(GL_TRIANGLES, _cylinder->indexCount(),
                                     GL_UNSIGNED_INT, 0, component->numEdges());
    _cylinder->vertexArrayObject()->release();

    _edgesShader.release();
}

void GraphScene::renderComponentMarkers()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    _componentMarkerDataBuffer.bind();
    _componentMarkerDataBuffer.allocate(_componentMarkerData.data(), _componentMarkerData.size() * sizeof(GLfloat) );

    // Bind the shader program
    _componentMarkerShader.bind();

    // Calculate needed matrices
    QMatrix4x4 modelViewMatrix = _camera->viewMatrix();
    _componentMarkerShader.setUniformValue( "modelViewMatrix", modelViewMatrix );
    _componentMarkerShader.setUniformValue( "projectionMatrix", _camera->projectionMatrix() );

    // Draw the edges
    _quad->vertexArrayObject()->bind();
    _funcs->glDrawElementsInstanced(GL_TRIANGLES, _quad->indexCount(),
                                     GL_UNSIGNED_INT, 0, _graphModel->graph().numComponents());
    _quad->vertexArrayObject()->release();
    _componentMarkerShader.release();

    glDisable(GL_BLEND);
}

void GraphScene::renderDebugLines()
{
    QMutexLocker locker(&_debugLinesMutex);

    _funcs->glDrawBuffer(GL_COLOR_ATTACHMENT0);

    _debugLinesDataBuffer.bind();
    _debugLinesDataBuffer.allocate(_debugLinesData.data(), _debugLinesData.size() * sizeof(GLfloat));

    _debugLinesShader.bind();

    // Calculate needed matrices
    QMatrix4x4 modelViewMatrix = _camera->viewMatrix();
    _debugLinesShader.setUniformValue( "modelViewMatrix", modelViewMatrix );
    _debugLinesShader.setUniformValue( "projectionMatrix", _camera->projectionMatrix() );

    _debugLinesDataVAO.bind();
    _funcs->glDrawArrays(GL_LINES, 0, _debugLines.size() * 2);
    _debugLinesDataVAO.release();
    _debugLinesShader.release();

    clearDebugLines();
}

void GraphScene::render2D()
{
    _funcs->glDisable(GL_DEPTH_TEST);

    QMatrix4x4 m;
    m.ortho(0.0f, _width, 0.0f, _height, -1.0f, 1.0f);

    if(!_selectionRect.isNull())
    {
        const QColor color(Qt::GlobalColor::white);

        QRect r;
        r.setLeft(_selectionRect.left());
        r.setRight(_selectionRect.right());
        r.setTop(_height - _selectionRect.top());
        r.setBottom(_height - _selectionRect.bottom());

        QVector<GLfloat> data;

        data.append(r.left());  data.append(r.bottom());
        data.append(color.redF()); data.append(color.blueF()); data.append(color.greenF());
        data.append(r.right()); data.append(r.bottom());
        data.append(color.redF()); data.append(color.blueF()); data.append(color.greenF());
        data.append(r.right()); data.append(r.top());
        data.append(color.redF()); data.append(color.blueF()); data.append(color.greenF());

        data.append(r.right()); data.append(r.top());
        data.append(color.redF()); data.append(color.blueF()); data.append(color.greenF());
        data.append(r.left());  data.append(r.top());
        data.append(color.redF()); data.append(color.blueF()); data.append(color.greenF());
        data.append(r.left());  data.append(r.bottom());
        data.append(color.redF()); data.append(color.blueF()); data.append(color.greenF());

        _funcs->glDrawBuffer(GL_COLOR_ATTACHMENT1);

        _selectionMarkerDataBuffer.bind();
        _selectionMarkerDataBuffer.allocate(data.data(),
                                           data.size() * sizeof(GLfloat));

        _selectionMarkerShader.bind();
        _selectionMarkerShader.setUniformValue("projectionMatrix", m);

        _selectionMarkerDataVAO.bind();
        _funcs->glDrawArrays(GL_TRIANGLES, 0, 6);
        _selectionMarkerDataVAO.release();

        _selectionMarkerShader.release();
    }

    _funcs->glEnable(GL_DEPTH_TEST);
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
    QMutexLocker locker(&_debugLinesMutex);

    _debugLinesData.resize(_debugLines.size() * 12);

    int i = 0;
    for(const DebugLine debugLine : _debugLines)
    {
        _debugLinesData[i++] = debugLine._start.x();
        _debugLinesData[i++] = debugLine._start.y();
        _debugLinesData[i++] = debugLine._start.z();
        _debugLinesData[i++] = debugLine._color.redF();
        _debugLinesData[i++] = debugLine._color.greenF();
        _debugLinesData[i++] = debugLine._color.blueF();
        _debugLinesData[i++] = debugLine._end.x();
        _debugLinesData[i++] = debugLine._end.y();
        _debugLinesData[i++] = debugLine._end.z();
        _debugLinesData[i++] = debugLine._color.redF();
        _debugLinesData[i++] = debugLine._color.greenF();
        _debugLinesData[i++] = debugLine._color.blueF();
    }
}

void GraphScene::render()
{
    if(!_FBOcomplete)
        return;

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    _funcs->glBindFramebuffer(GL_FRAMEBUFFER, _visualFBO);

    _funcs->glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    _funcs->glDrawBuffer(GL_COLOR_ATTACHMENT1);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    _funcs->glDrawBuffers(2, drawBuffers);
    glClear(GL_DEPTH_BUFFER_BIT);

    renderNodes();
    renderEdges();
    render2D();

    //renderComponentMarkers();
    renderDebugLines();

    _funcs->glBindFramebuffer(GL_FRAMEBUFFER, 0);

    _funcs->glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
    _funcs->glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    _funcs->glDisable(GL_DEPTH_TEST);

    _screenQuadVAO.bind();
    _funcs->glActiveTexture(GL_TEXTURE0);

    // Color texture
    _funcs->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _colorTexture);

    _screenShader.bind();
    _funcs->glDrawArrays(GL_TRIANGLES, 0, 6);
    _screenShader.release();

    // Selection texture
    _funcs->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _selectionTexture);

    _selectionShader.bind();
    _funcs->glDrawArrays(GL_TRIANGLES, 0, 6);
    _selectionShader.release();

    _screenQuadVAO.release();

    _funcs->glEnable(GL_DEPTH_TEST);

    glDisable(GL_BLEND);
}

void GraphScene::resize(int w, int h)
{
    _width = w;
    _height = h;

    qDebug() << "GraphScene::resize(" << w << h << ")";

    _FBOcomplete = prepareRenderBuffers();

    glViewport(0, 0, w, h);

    _aspectRatio = static_cast<float>(w) / static_cast<float>(h);

    if(_graphModel != nullptr)
    {
        for(ComponentId componentId : *_graphModel->graph().componentIds())
        {
            ComponentViewData* componentViewData = &(*_componentsViewData)[componentId];
            setupCamera(componentViewData->_camera, _aspectRatio);
        }
    }
}

const float MINIMU_CAMERA_DISTANCE = 2.5f;

void GraphScene::zoom(float direction)
{
    if(direction == 0.0f || !_panTransition.finished())
        return;

    ComponentViewData* componentViewData = focusComponentViewData();
    float size = _graphModel->nodeVisuals()[componentViewData->_focusNodeId]._size;

    const float INTERSECTION_AVOIDANCE_OFFSET = 1.0f;
    const float ZOO_STEP_FRACTION = 0.2f;
    float delta = (_targetZoomDistance - size - INTERSECTION_AVOIDANCE_OFFSET) * ZOO_STEP_FRACTION;

    _targetZoomDistance -= delta * direction;

    if(_targetZoomDistance < MINIMU_CAMERA_DISTANCE)
        _targetZoomDistance = MINIMU_CAMERA_DISTANCE;

    float startZoomDistance = componentViewData->_zoomDistance;
    _zoomTransition.start(0.1f, Transition::Type::Linear,
    [=](float f)
    {
        componentViewData->_zoomDistance = startZoomDistance + ((_targetZoomDistance - startZoomDistance) * f);
    });
}

void GraphScene::centreNodeInViewport(NodeId nodeId, Transition::Type transitionType, float cameraDistance)
{
    if(nodeId.isNull())
        return;

    const QVector3D& nodePosition = _graphModel->nodePositions()[nodeId];
    Plane translationPlane(nodePosition, _camera->viewVector().normalized());

    QVector3D curPoint = translationPlane.rayIntersection(
                Ray(_camera->position(), _camera->viewVector().normalized()));

    QVector3D translation = nodePosition - curPoint;
    QVector3D startPosition = _camera->position();
    QVector3D startViewTarget = _camera->viewTarget();
    QVector3D targetPosition = _camera->position() + translation;

    if(cameraDistance >= 0.0f)
        targetPosition = nodePosition - (_camera->viewVector().normalized() * cameraDistance);
    else
        focusComponentViewData()->_zoomDistance = _targetZoomDistance = translationPlane.distanceToPoint(targetPosition);

    if(targetPosition.distanceToPoint(nodePosition) < MINIMU_CAMERA_DISTANCE)
        targetPosition = nodePosition - (_camera->viewVector().normalized() * MINIMU_CAMERA_DISTANCE);

    if(transitionType != Transition::Type::None)
    {
        emit userInteractionStarted();
        _panTransition.start(0.3f, transitionType,
        [=](float f)
        {
            _camera->setPosition(startPosition + ((targetPosition - startPosition) * f));
            _camera->setViewTarget(startViewTarget + ((nodePosition - startViewTarget) * f));
        },
        [=]()
        {
            emit userInteractionFinished();
        });
    }
    else
    {
        _camera->setPosition(targetPosition);
        _camera->setViewTarget(nodePosition);
    }
}

void GraphScene::moveFocusToNode(NodeId nodeId, Transition::Type transitionType)
{
    if(_focusComponentId.isNull())
        return;

    centreNodeInViewport(nodeId, transitionType);
    focusComponentViewData()->_focusNodeId = nodeId;
    updateVisualData();
}

void GraphScene::selectFocusNodeClosestToCameraVector(Transition::Type transitionType)
{
    if(_focusComponentId.isNull())
        return;

    Collision collision(*_graphModel->graph().componentById(_focusComponentId),
                        _graphModel->nodeVisuals(), _graphModel->nodePositions());
    NodeId closestNodeId = collision.closestNodeToLine(_camera->position(), _camera->viewVector().normalized());
    if(!closestNodeId.isNull())
        moveFocusToNode(closestNodeId, transitionType);
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
    _focusComponentId = cycleThroughComponentIds(*_graphModel->graph().componentIds(), _focusComponentId, -1);

    if(!_focusComponentId.isNull())
    {
        updateVisualData();
        _targetZoomDistance = focusComponentViewData()->_zoomDistance;
    }
}

void GraphScene::moveToPreviousComponent()
{
    _focusComponentId = cycleThroughComponentIds(*_graphModel->graph().componentIds(), _focusComponentId, 1);

    if(!_focusComponentId.isNull())
    {
        updateVisualData();
        _targetZoomDistance = focusComponentViewData()->_zoomDistance;
    }
}

void GraphScene::moveToComponent(ComponentId componentId)
{
    _focusComponentId = componentId;

    if(!_focusComponentId.isNull())
    {
        updateVisualData();
        _targetZoomDistance = focusComponentViewData()->_zoomDistance;
    }
}

bool GraphScene::interactionAllowed()
{
    return _panTransition.finished();
}

void GraphScene::setGraphModel(GraphModel* graphModel)
{
    this->_graphModel = graphModel;

    if(_componentsViewData != nullptr)
        delete _componentsViewData;

    _componentsViewData = new ComponentArray<ComponentViewData>(_graphModel->graph());
    _focusComponentId = _graphModel->graph().firstComponentId();

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

void GraphScene::prepareVertexBuffers()
{
    // Populate the data buffer object
    _nodePositionBuffer.create();
    _nodePositionBuffer.setUsagePattern( QOpenGLBuffer::DynamicDraw );
    _nodePositionBuffer.bind();
    _nodePositionBuffer.allocate( _nodePositionData.data(), _nodePositionData.size() * sizeof(GLfloat) );

    _edgePositionBuffer.create();
    _edgePositionBuffer.setUsagePattern( QOpenGLBuffer::DynamicDraw );
    _edgePositionBuffer.bind();
    _edgePositionBuffer.allocate( _edgePositionData.data(), _edgePositionData.size() * sizeof(GLfloat) );

    _nodeVisualBuffer.create();
    _nodeVisualBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    _nodeVisualBuffer.bind();
    _nodeVisualBuffer.allocate( _nodeVisualData.data(), _nodeVisualData.size() * sizeof(GLfloat) );

    _edgeVisualBuffer.create();
    _edgeVisualBuffer.setUsagePattern( QOpenGLBuffer::StaticDraw );
    _edgeVisualBuffer.bind();
    _edgeVisualBuffer.allocate( _edgeVisualData.data(), _edgeVisualData.size() * sizeof(GLfloat) );

    _componentMarkerDataBuffer.create();
    _componentMarkerDataBuffer.setUsagePattern( QOpenGLBuffer::DynamicDraw );
    _componentMarkerDataBuffer.bind();
    _componentMarkerDataBuffer.allocate( _componentMarkerData.data(), _componentMarkerData.size() * sizeof(GLfloat) );

    _selectionMarkerDataBuffer.create();
    _selectionMarkerDataBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    _selectionMarkerDataBuffer.bind();

    _debugLinesDataBuffer.create();
    _debugLinesDataBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    _debugLinesDataBuffer.bind();
    _debugLinesDataBuffer.allocate(_debugLinesData.data(), _debugLinesData.size() * sizeof(GLfloat));
}

void GraphScene::prepareNodeVAO()
{
    // Bind the marker's VAO
    _sphere->vertexArrayObject()->bind();

    // Enable the data buffer and add it to the marker's VAO
    QOpenGLShaderProgramPtr shader = _sphere->material()->shader();
    shader->bind();
    _nodePositionBuffer.bind();
    shader->enableAttributeArray("point");
    shader->setAttributeBuffer("point", GL_FLOAT, 0, 3);

    _nodeVisualBuffer.bind();
    shader->enableAttributeArray("size");
    shader->enableAttributeArray("color");
    shader->enableAttributeArray("outlineColor");
    shader->setAttributeBuffer("size", GL_FLOAT, 0, 1, 7 * sizeof(GLfloat));
    shader->setAttributeBuffer("color", GL_FLOAT, 1 * sizeof(GLfloat), 3, 7 * sizeof(GLfloat));
    shader->setAttributeBuffer("outlineColor", GL_FLOAT, 4 * sizeof(GLfloat), 3, 7 * sizeof(GLfloat));

    // We only vary the point attribute once per instance
    GLuint pointLocation = shader->attributeLocation("point");
    GLuint sizeLocation = shader->attributeLocation("size");
    GLuint colorLocation = shader->attributeLocation("color");
    GLuint outlineColorLocation = shader->attributeLocation("outlineColor");
#if !defined(Q_OS_MAC)
    _funcs->glVertexAttribDivisor(pointLocation, 1);
    _funcs->glVertexAttribDivisor(sizeLocation, 1);
    _funcs->glVertexAttribDivisor(colorLocation, 1);
    _funcs->glVertexAttribDivisor(outlineColorLocation, 1);
#else
    _instanceFuncs->glVertexAttribDivisorARB(pointLocation, 1);
    _instanceFuncs->glVertexAttribDivisorARB(sizeLocation, 1);
    _instanceFuncs->glVertexAttribDivisorARB(colorLocation, 1);
    _instanceFuncs->glVertexAttribDivisorARB(outlineColorLocation, 1);
#endif
    _sphere->vertexArrayObject()->release();
    shader->release();
}

void GraphScene::prepareEdgeVAO()
{
    // Bind the marker's VAO
    _cylinder->vertexArrayObject()->bind();

    // Enable the data buffer and add it to the marker's VAO
    QOpenGLShaderProgramPtr shader = _cylinder->material()->shader();
    shader->bind();
    _edgePositionBuffer.bind();
    shader->enableAttributeArray("source");
    shader->enableAttributeArray("target");
    shader->setAttributeBuffer("source", GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));
    shader->setAttributeBuffer("target", GL_FLOAT, 3 * sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

    _edgeVisualBuffer.bind();
    shader->enableAttributeArray("size");
    shader->enableAttributeArray("color");
    shader->enableAttributeArray("outlineColor");
    shader->setAttributeBuffer("size", GL_FLOAT, 0, 1, 7 * sizeof(GLfloat));
    shader->setAttributeBuffer("color", GL_FLOAT, 1 * sizeof(GLfloat), 3, 7 * sizeof(GLfloat));
    shader->setAttributeBuffer("outlineColor", GL_FLOAT, 4 * sizeof(GLfloat), 3, 7 * sizeof(GLfloat));

    // We only vary the point attribute once per instance
    GLuint sourcePointLocation = shader->attributeLocation("source");
    GLuint targetPointLocation = shader->attributeLocation("target");
    GLuint sizeLocation = shader->attributeLocation("size");
    GLuint colorLocation = shader->attributeLocation("color");
    GLuint outlineColorLocation = shader->attributeLocation("outlineColor");
#if !defined(Q_OS_MAC)
    _funcs->glVertexAttribDivisor(sourcePointLocation, 1);
    _funcs->glVertexAttribDivisor(targetPointLocation, 1);
    _funcs->glVertexAttribDivisor(sizeLocation, 1);
    _funcs->glVertexAttribDivisor(colorLocation, 1);
    _funcs->glVertexAttribDivisor(outlineColorLocation, 1);
#else
    _instanceFuncs->glVertexAttribDivisorARB(sourcePointLocation, 1);
    _instanceFuncs->glVertexAttribDivisorARB(targetPointLocation, 1);
    _instanceFuncs->glVertexAttribDivisorARB(sizeLocation, 1);
    _instanceFuncs->glVertexAttribDivisorARB(colorLocation, 1);
    _instanceFuncs->glVertexAttribDivisorARB(outlineColorLocation, 1);
#endif
    _cylinder->vertexArrayObject()->release();
    shader->release();
}

void GraphScene::prepareComponentMarkerVAO()
{
    // Bind the marker's VAO
    _quad->vertexArrayObject()->bind();

    // Enable the data buffer and add it to the marker's VAO
    QOpenGLShaderProgramPtr shader = _quad->material()->shader();
    shader->bind();
    _componentMarkerDataBuffer.bind();
    shader->enableAttributeArray("point");
    shader->enableAttributeArray("scale");
    shader->setAttributeBuffer("point", GL_FLOAT, 0, 2, 3 * sizeof(GLfloat));
    shader->setAttributeBuffer("scale", GL_FLOAT, 2 * sizeof(GLfloat), 1, 3 * sizeof(GLfloat));

    // We only vary the point attribute once per instance
    GLuint pointLocation = shader->attributeLocation("point");
    GLuint scaleLocation = shader->attributeLocation("scale");
#if !defined(Q_OS_MAC)
    _funcs->glVertexAttribDivisor(pointLocation, 1);
    _funcs->glVertexAttribDivisor(scaleLocation, 1);
#else
    _instanceFuncs->glVertexAttribDivisorARB(pointLocation, 1);
    _instanceFuncs->glVertexAttribDivisorARB(scaleLocation, 1);
#endif
    _quad->vertexArrayObject()->release();
    shader->release();
}

void GraphScene::prepareSelectionMarkerVAO()
{
    _selectionMarkerDataVAO.bind();
    _selectionMarkerShader.bind();
    _selectionMarkerDataBuffer.bind();

    _selectionMarkerShader.enableAttributeArray("position");
    _selectionMarkerShader.enableAttributeArray("color");
    _selectionMarkerShader.disableAttributeArray("texCoord");
    _selectionMarkerShader.setAttributeBuffer("position", GL_FLOAT, 0, 2, 5 * sizeof(GLfloat));
    _selectionMarkerShader.setAttributeBuffer("color", GL_FLOAT, 2 * sizeof(GLfloat), 3, 5 * sizeof(GLfloat));

    _selectionMarkerDataVAO.release();
    _selectionMarkerShader.release();
}

void GraphScene::prepareDebugLinesVAO()
{
    _debugLinesDataVAO.bind();
    _debugLinesShader.bind();
    _debugLinesDataBuffer.bind();

    _debugLinesShader.enableAttributeArray("position");
    _debugLinesShader.enableAttributeArray("color");
    _debugLinesShader.setAttributeBuffer("position", GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));
    _debugLinesShader.setAttributeBuffer("color", GL_FLOAT, 3 * sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

    _debugLinesDataVAO.release();
    _debugLinesShader.release();
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

    _screenQuadVAO.create();
    _screenQuadVAO.bind();

    quadBuffer.create();
    quadBuffer.bind();
    quadBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    quadBuffer.allocate(quadVerts, quadVertsSize);

    loadShaderProgram(_screenShader, ":/gl/shaders/screen.vert", ":/gl/shaders/screen.frag");
    _screenShader.bind();
    _screenShader.enableAttributeArray("vertexPosition");
    _screenShader.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 2, 2 * sizeof(GLfloat));
    _screenShader.setUniformValue("frameBufferTexture", 0);
    _screenShader.release();

    loadShaderProgram(_selectionShader, ":/gl/shaders/screen.vert", ":/gl/shaders/selection.frag");
    _selectionShader.bind();
    _selectionShader.enableAttributeArray("vertexPosition");
    _selectionShader.setAttributeBuffer("vertexPosition", GL_FLOAT, 0, 2, 2 * sizeof(GLfloat));
    _selectionShader.setUniformValue("frameBufferTexture", 0);
    _selectionShader.release();

    quadBuffer.release();
    _screenQuadVAO.release();
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
    if(_colorTexture == 0)
        _funcs->glGenTextures(1, &_colorTexture);
    _funcs->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _colorTexture);
    _funcs->glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, multisamples, GL_RGBA, _width, _height, GL_FALSE);
    _funcs->glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
    _funcs->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

    // Selection texture
    if(_selectionTexture == 0)
        _funcs->glGenTextures(1, &_selectionTexture);
    _funcs->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _selectionTexture);
    _funcs->glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, multisamples, GL_RGBA, _width, _height, GL_FALSE);
    _funcs->glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
    _funcs->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

    // Depth texture
    if(_depthTexture == 0)
        _funcs->glGenTextures(1, &_depthTexture);
    _funcs->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, _depthTexture);
    _funcs->glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, multisamples, GL_DEPTH_COMPONENT, _width, _height, GL_FALSE);
    _funcs->glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAX_LEVEL, 0);
    _funcs->glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

    // Visual FBO
    if(_visualFBO == 0)
        _funcs->glGenFramebuffers(1, &_visualFBO);
    _funcs->glBindFramebuffer(GL_FRAMEBUFFER, _visualFBO);
    _funcs->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, _colorTexture, 0);
    _funcs->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D_MULTISAMPLE, _selectionTexture, 0);
    _funcs->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, _depthTexture, 0);

    GLenum status = _funcs->glCheckFramebufferStatus(GL_FRAMEBUFFER);
    valid = (status == GL_FRAMEBUFFER_COMPLETE);

    _funcs->glBindFramebuffer(GL_FRAMEBUFFER, 0);

    Q_ASSERT(valid);
    return valid;
}

ComponentViewData::ComponentViewData() :
    _zoomDistance(50.0f),
    _initialised(false)
{
    _camera.setPosition(QVector3D(0.0f, 0.0f, 50.0f));
    _camera.setViewTarget(QVector3D(0.0f, 0.0f, 0.0f));
    _camera.setUpVector(QVector3D(0.0f, 1.0f, 0.0f));
}

ComponentViewData::ComponentViewData(const ComponentViewData& other) :
    _camera(other._camera),
    _zoomDistance(other._zoomDistance),
    _focusNodeId(other._focusNodeId),
    _initialised(other._initialised)
{
}
