#include "graphcomponentrenderer.h"

#include "camera.h"
#include "primitives/quad.h"
#include "material.h"

#include "../graph/graphmodel.h"
#include "../layout/layout.h"
#include "../layout/barneshuttree.h"
#include "../layout/collision.h"

#include "../maths/frustum.h"
#include "../maths/plane.h"
#include "../maths/boundingsphere.h"

#include "../ui/graphwidget.h"
#include "../ui/selectionmanager.h"

#include "../utils/utils.h"
#include "../utils/make_unique.h"

#include <QObject>
#include <QOpenGLContext>
#include <QOpenGLFunctions_3_3_Core>

#include <QKeyEvent>
#include <QMouseEvent>

#include <QtMath>
#include <cmath>
#include <mutex>

GraphComponentRenderer::GraphComponentRenderer()
    : _graphWidget(nullptr),
      _initialised(false),
      _visible(false),
      _frozen(false),
      _cleanupWhenThawed(false),
      _updateVisualDataWhenThawed(false),
      _updatePositionDataWhenThawed(false),
      _viewportWidth(0), _viewportHeight(0),
      _renderWidth(0), _renderHeight(0),
      _visualDataRequiresUpdate(false),
      _trackFocus(true),
      _targetZoomDistance(0.0f),
      _funcs(nullptr),
      _fovx(0.0f),
      _fovy(0.0f),
      _numNodesInPositionData(0),
      _numEdgesInPositionData(0)
{
}

void GraphComponentRenderer::initialise(std::shared_ptr<GraphModel> graphModel, ComponentId componentId,
                                        GraphWidget& graphWidget,
                                        std::shared_ptr<SelectionManager> selectionManager,
                                        std::shared_ptr<GraphRenderer> shared)
{
    _graphModel = graphModel;
    _componentId = componentId;
    _graphWidget = &graphWidget;
    _selectionManager = selectionManager;
    _shared = shared;

    _funcs = _shared->_context->versionFunctions<QOpenGLFunctions_3_3_Core>();
    if(!_funcs)
        qFatal("Could not obtain required OpenGL context version");
    _funcs->initializeOpenGLFunctions();

    //FIXME: eliminate Material, set shader directly?
    MaterialPtr nodeMaterial(new Material);
    nodeMaterial->setShaders(":/rendering/shaders/instancednodes.vert", ":/rendering/shaders/ads.frag");

    _sphere.setRadius(1.0f);
    _sphere.setRings(16);
    _sphere.setSlices(16);
    _sphere.setMaterial(nodeMaterial);
    _sphere.create();

    MaterialPtr edgeMaterial(new Material);
    edgeMaterial->setShaders(":/rendering/shaders/instancededges.vert", ":/rendering/shaders/ads.frag");

    _cylinder.setRadius(1.0f);
    _cylinder.setLength(1.0f);
    _cylinder.setSlices(8);
    _cylinder.setMaterial(edgeMaterial);
    _cylinder.create();

    if(!_debugLinesDataVAO.isCreated())
        _debugLinesDataVAO.create();

    prepareVertexBuffers();

    prepareNodeVAO();
    prepareEdgeVAO();
    prepareDebugLinesVAO();

    _targetZoomDistance = _viewData._zoomDistance;
    _viewData._focusNodeId.setToNull();

    _initialised = true;

    updatePositionalData();
    updateVisualData(When::Now);
}

void GraphComponentRenderer::setVisible(bool visible)
{
    if(visible && !_visible)
    {
        // We're about to display for the first time
        // so make sure the GPU data is up-to-date
        updatePositionalData();
        updateVisualData();
    }

    _visible = visible;
}

void GraphComponentRenderer::cleanup()
{
    if(_frozen)
    {
        _cleanupWhenThawed = true;
        return;
    }

    _nodePositionData.clear();
    _numNodesInPositionData = 0;
    _edgePositionData.clear();
    _numEdgesInPositionData = 0;

    _nodeVisualData.clear();
    _edgeVisualData.clear();

    _graphModel = nullptr;
    _componentId.setToNull();
    _graphWidget = nullptr;
    _selectionManager = nullptr;
    _shared = nullptr;

    _funcs = nullptr;
    _initialised = false;
}

void GraphComponentRenderer::cloneViewDataFrom(const GraphComponentRenderer& other)
{
    _viewData = other._viewData;
}

void GraphComponentRenderer::restoreViewData()
{
    updateFocusPosition();
    _viewData._autoZooming = _savedViewData._autoZooming;
    _viewData._focusNodeId = _savedViewData._focusNodeId;
    zoomToDistance(_savedViewData._zoomDistance);

    if(_savedViewData._focusNodeId.isNull())
        centrePositionInViewport(_viewData._focusPosition, _viewData._zoomDistance);
    else
        centreNodeInViewport(_savedViewData._focusNodeId, _viewData._zoomDistance);
}

void GraphComponentRenderer::freeze()
{
    _frozen = true;
}

void GraphComponentRenderer::thaw()
{
    _frozen = false;

    if(_cleanupWhenThawed)
    {
        cleanup();
        _cleanupWhenThawed = false;
    }
    else
    {
        if(_updateVisualDataWhenThawed)
        {
            updateVisualData(When::Now);
            _updateVisualDataWhenThawed = false;
        }

        if(_updatePositionDataWhenThawed)
        {
            updatePositionalData();
            _updatePositionDataWhenThawed = false;
        }
    }
}

void GraphComponentRenderer::updatePositionalData()
{
    Q_ASSERT(_initialised);

    if(_frozen)
    {
        _updatePositionDataWhenThawed = true;
        return;
    }

    std::unique_lock<std::recursive_mutex> lock(_graphModel->nodePositions().mutex());

    auto component = _graphModel->graph().componentById(_componentId);

    NodePositions& nodePositions = _graphModel->nodePositions();

    _numNodesInPositionData = component->numNodes();
    _numEdgesInPositionData = component->numEdges();

    _nodePositionData.resize(_numNodesInPositionData * 3);
    _edgePositionData.resize(_numEdgesInPositionData * 6);
    int i = 0;

    NodeArray<QVector3D> scaledAndSmoothedNodePositions(_graphModel->graph());

    for(NodeId nodeId : component->nodeIds())
    {
        const QVector3D nodePosition = nodePositions.getScaledAndSmoothed(nodeId);
        scaledAndSmoothedNodePositions[nodeId] = nodePosition;

        _nodePositionData[i++] = nodePosition.x();
        _nodePositionData[i++] = nodePosition.y();
        _nodePositionData[i++] = nodePosition.z();
    }

    _nodePositionBuffer.bind();
    _nodePositionBuffer.allocate(_nodePositionData.data(), static_cast<int>(_nodePositionData.size()) * sizeof(GLfloat));
    _nodePositionBuffer.release();

    i = 0;
    for(EdgeId edgeId : component->edgeIds())
    {
        const Edge& edge = _graphModel->graph().edgeById(edgeId);
        const QVector3D sourcePosition = scaledAndSmoothedNodePositions[edge.sourceId()];
        const QVector3D targetPosition = scaledAndSmoothedNodePositions[edge.targetId()];

        _edgePositionData[i++] = sourcePosition.x();
        _edgePositionData[i++] = sourcePosition.y();
        _edgePositionData[i++] = sourcePosition.z();
        _edgePositionData[i++] = targetPosition.x();
        _edgePositionData[i++] = targetPosition.y();
        _edgePositionData[i++] = targetPosition.z();
    }

    _edgePositionBuffer.bind();
    _edgePositionBuffer.allocate(_edgePositionData.data(), static_cast<int>(_edgePositionData.size()) * sizeof(GLfloat));
    _edgePositionBuffer.release();

    updateFocusPosition();
    updateEntireComponentZoomDistance();
}

float GraphComponentRenderer::zoomDistanceForNodeIds(const QVector3D& centre, std::vector<NodeId> nodeIds)
{
    float minHalfFov = qDegreesToRadians(std::min(_fovx, _fovy) * 0.5f);

    if(minHalfFov > 0.0f)
    {
        float maxDistance = std::numeric_limits<float>::min();
        for(auto nodeId : nodeIds)
        {
            QVector3D nodePosition = _graphModel->nodePositions().getScaledAndSmoothed(nodeId);
            auto& nodeVisual = _graphModel->nodeVisuals().at(nodeId);
            float distance = (centre - nodePosition).length() + nodeVisual._size;

            if(distance > maxDistance)
                maxDistance = distance;
        }

        return maxDistance / std::sin(minHalfFov);
    }
    else
    {
        qWarning() << "zoomDistanceForNodes returning default value";
        return 1.0f;
    }
}

void GraphComponentRenderer::updateFocusPosition()
{
    auto component = _graphModel->graph().componentById(_componentId);
    _viewData._focusPosition = NodePositions::centreOfMassScaledAndSmoothed(_graphModel->nodePositions(),
                                                                            component->nodeIds());
}

void GraphComponentRenderer::updateEntireComponentZoomDistance()
{
    auto component = _graphModel->graph().componentById(_componentId);
    _entireComponentZoomDistance = zoomDistanceForNodeIds(focusPosition(), component->nodeIds());
}

void GraphComponentRenderer::updateVisualData(When when)
{
    _visualDataRequiresUpdate = true;

    if(when == When::Now)
        updateVisualDataIfRequired();
}

void GraphComponentRenderer::updateVisualDataIfRequired()
{
    Q_ASSERT(_initialised);

    if(!_visualDataRequiresUpdate)
        return;

    if(_frozen)
    {
        _updateVisualDataWhenThawed = true;
        return;
    }

    auto component = _graphModel->graph().componentById(_componentId);

    _visualDataRequiresUpdate = false;

    auto& nodeVisuals = _graphModel->nodeVisuals();
    auto& edgeVisuals = _graphModel->edgeVisuals();

    _nodeVisualData.resize(component->numNodes() * 7);
    _edgeVisualData.resize(component->numEdges() * 7);

    const QColor selectedOutLineColor = Qt::GlobalColor::white;
    const QColor deselectedOutLineColor = Qt::GlobalColor::black;

    int i = 0;
    for(NodeId nodeId : component->nodeIds())
    {
        _nodeVisualData[i++] = nodeVisuals[nodeId]._size;
        _nodeVisualData[i++] = nodeVisuals[nodeId]._color.redF();
        _nodeVisualData[i++] = nodeVisuals[nodeId]._color.greenF();
        _nodeVisualData[i++] = nodeVisuals[nodeId]._color.blueF();

        QColor outlineColor = _selectionManager && _selectionManager->nodeIsSelected(nodeId) ?
            outlineColor = selectedOutLineColor :
            outlineColor = deselectedOutLineColor;

        _nodeVisualData[i++] = outlineColor.redF();
        _nodeVisualData[i++] = outlineColor.greenF();
        _nodeVisualData[i++] = outlineColor.blueF();
    }

    _nodeVisualBuffer.bind();
    _nodeVisualBuffer.allocate(_nodeVisualData.data(), static_cast<int>(_nodeVisualData.size()) * sizeof(GLfloat));
    _nodeVisualBuffer.release();

    i = 0;
    for(EdgeId edgeId : component->edgeIds())
    {
        _edgeVisualData[i++] = edgeVisuals[edgeId]._size;
        _edgeVisualData[i++] = edgeVisuals[edgeId]._color.redF();
        _edgeVisualData[i++] = edgeVisuals[edgeId]._color.greenF();
        _edgeVisualData[i++] = edgeVisuals[edgeId]._color.blueF();
        _edgeVisualData[i++] = deselectedOutLineColor.redF();
        _edgeVisualData[i++] = deselectedOutLineColor.greenF();
        _edgeVisualData[i++] = deselectedOutLineColor.blueF();
    }

    _edgeVisualBuffer.bind();
    _edgeVisualBuffer.allocate(_edgeVisualData.data(), static_cast<int>(_edgeVisualData.size()) * sizeof(GLfloat));
    _edgeVisualBuffer.release();
}

void GraphComponentRenderer::update(float t)
{
    Q_ASSERT(_initialised);

    if(_graphModel)
    {
        updateVisualDataIfRequired();

        _zoomTransition.update(t);

        if(_graphWidget->transition().finished() && _trackFocus)
        {
            if(trackingCentreOfComponent())
            {
                if(_zoomTransition.finished() && _viewData._autoZooming)
                    zoomToDistance(_entireComponentZoomDistance);

                centrePositionInViewport(_viewData._focusPosition, _viewData._zoomDistance);
            }
            else
                centreNodeInViewport(_viewData._focusNodeId, _viewData._zoomDistance);
        }

        _modelViewMatrix = _viewData._camera.viewMatrix();
        _projectionMatrix = _viewData._camera.projectionMatrix();
    }

    submitDebugLines();
}

static void setShaderADSParameters(QOpenGLShaderProgram& program, float alpha)
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

    std::vector<Light> lights;
    lights.emplace_back(QVector4D(-20.0f, 0.0f, 3.0f, 1.0f), QVector3D(0.6f, 0.6f, 0.6f));
    lights.emplace_back(QVector4D(0.0f, 0.0f, 0.0f, 1.0f), QVector3D(0.2f, 0.2f, 0.2f));
    lights.emplace_back(QVector4D(10.0f, -10.0f, -10.0f, 1.0f), QVector3D(0.4f, 0.4f, 0.4f));

    int numberOfLights = static_cast<int>(lights.size());

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

    program.setUniformValue("alpha", alpha);
}

void GraphComponentRenderer::renderNodes(float alpha)
{
    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    _funcs->glDrawBuffers(2, drawBuffers);

    _shared->_nodesShader.bind();
    setShaderADSParameters(_shared->_nodesShader, alpha);

    _nodePositionBuffer.bind();
    _nodeVisualBuffer.bind();

    QMatrix3x3 normalMatrix = _modelViewMatrix.normalMatrix();
    _shared->_nodesShader.setUniformValue("modelViewMatrix", _modelViewMatrix);
    _shared->_nodesShader.setUniformValue("normalMatrix", normalMatrix);
    _shared->_nodesShader.setUniformValue("projectionMatrix", _projectionMatrix);

    // Draw the nodes
    _sphere.vertexArrayObject()->bind();
    _funcs->glDrawElementsInstanced(GL_TRIANGLES, _sphere.indexCount(),
                                    GL_UNSIGNED_INT, 0, _numNodesInPositionData);
    _sphere.vertexArrayObject()->release();

    _nodeVisualBuffer.release();
    _nodePositionBuffer.release();
    _shared->_nodesShader.release();
}

void GraphComponentRenderer::renderEdges(float alpha)
{
    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    _funcs->glDrawBuffers(2, drawBuffers);

    _shared->_edgesShader.bind();
    setShaderADSParameters(_shared->_edgesShader, alpha);

    _edgePositionBuffer.bind();
    _edgeVisualBuffer.bind();

    _shared->_edgesShader.setUniformValue("viewMatrix", _modelViewMatrix);
    _shared->_edgesShader.setUniformValue("projectionMatrix", _projectionMatrix);

    // Draw the edges
    _cylinder.vertexArrayObject()->bind();
    _funcs->glDrawElementsInstanced(GL_TRIANGLES, _cylinder.indexCount(),
                                    GL_UNSIGNED_INT, 0, _numEdgesInPositionData);
    _cylinder.vertexArrayObject()->release();

    _edgeVisualBuffer.release();
    _edgePositionBuffer.release();
    _shared->_edgesShader.release();
}

void GraphComponentRenderer::renderDebugLines()
{
    std::unique_lock<std::mutex> lock(_debugLinesMutex);

    _funcs->glDrawBuffer(GL_COLOR_ATTACHMENT0);

    _debugLinesDataBuffer.bind();
    _debugLinesDataBuffer.allocate(_debugLinesData.data(), static_cast<int>(_debugLinesData.size()) * sizeof(GLfloat));

    _shared->_debugLinesShader.bind();

    // Calculate needed matrices
    _shared->_debugLinesShader.setUniformValue("modelViewMatrix", _modelViewMatrix);
    _shared->_debugLinesShader.setUniformValue("projectionMatrix", _projectionMatrix);

    _debugLinesDataVAO.bind();
    _funcs->glDrawArrays(GL_LINES, 0, static_cast<int>(_debugLines.size()) * 2);
    _debugLinesDataVAO.release();
    _shared->_debugLinesShader.release();
    _debugLinesDataBuffer.release();

    clearDebugLines();
}

void GraphComponentRenderer::render2D()
{
    _funcs->glDisable(GL_DEPTH_TEST);

    QMatrix4x4 m;
    m.ortho(0.0f, _viewportWidth, 0.0f, _viewportHeight, -1.0f, 1.0f);

    _funcs->glEnable(GL_DEPTH_TEST);
}

void GraphComponentRenderer::addDebugBoundingBox(const BoundingBox3D& boundingBox, const QColor color)
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

void GraphComponentRenderer::submitDebugLines()
{
    std::unique_lock<std::mutex> lock(_debugLinesMutex);

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

void GraphComponentRenderer::render(int x, int y, int width, int height, float alpha)
{
    if(!_initialised)
        return;

    if(!_shared->_FBOcomplete)
    {
        qWarning() << "Attempting to render component" <<
                      _componentId << "without a complete FBO";
        return;
    }

    if(width <= 0)
        width = _viewportWidth;

    if(height <= 0)
        height = _viewportHeight;

    _funcs->glEnable(GL_DEPTH_TEST);
    _funcs->glEnable(GL_CULL_FACE);
    _funcs->glDisable(GL_BLEND);

    _funcs->glViewport(x, _viewportHeight - height - y, width, height);
    _funcs->glBindFramebuffer(GL_FRAMEBUFFER, _shared->_visualFBO);

    GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    _funcs->glDrawBuffers(2, drawBuffers);
    _funcs->glClear(GL_DEPTH_BUFFER_BIT);

    renderNodes(alpha);
    renderEdges(alpha);
    render2D();

    renderDebugLines();
}

void GraphComponentRenderer::resize(int viewportWidth, int viewportHeight,
                                    int renderWidth, int renderHeight)
{
    if(_initialised && viewportWidth > 0 && viewportHeight > 0)
    {
        _viewportWidth = viewportWidth;
        _viewportHeight = viewportHeight;

        if(renderWidth <= 0)
            renderWidth = _viewportWidth;

        if(renderHeight <= 0)
            renderHeight = _viewportHeight;

        _renderWidth = renderWidth;
        _renderHeight = renderHeight;

        float aspectRatio = static_cast<float>(renderWidth) / static_cast<float>(renderHeight);
        _fovy = 60.0f;
        _fovx = _fovy * aspectRatio;

        _viewData._camera.setPerspectiveProjection(_fovy, aspectRatio, 0.3f, 50000.0f);
        _viewData._camera.setViewportWidth(renderWidth);
        _viewData._camera.setViewportHeight(renderHeight);
    }
    else
    {
        qWarning() << "GraphComponentRenderer::resize(" << viewportWidth << "," << viewportHeight <<
                    ") failed _initialised:" << _initialised;
    }
}

const float MINIMUM_ZOOM_DISTANCE = 2.5f;

void GraphComponentRenderer::zoom(float direction)
{
    if(direction == 0.0f || !_graphWidget->transition().finished())
        return;

    // Don't allow zooming out if autozooming
    if(direction < 0.0f && _viewData._autoZooming)
        return;

    float size = 0.0f;

    _viewData._autoZooming = false;

    if(!_viewData._focusNodeId.isNull())
        size = _graphModel->nodeVisuals().at(_viewData._focusNodeId)._size;

    const float INTERSECTION_AVOIDANCE_OFFSET = 1.0f;
    const float ZOOM_STEP_FRACTION = 0.2f;
    float delta = (_targetZoomDistance - size - INTERSECTION_AVOIDANCE_OFFSET) * ZOOM_STEP_FRACTION;

    _targetZoomDistance -= delta * direction;
    _targetZoomDistance = std::max(_targetZoomDistance, MINIMUM_ZOOM_DISTANCE);

    if(_targetZoomDistance > _entireComponentZoomDistance)
    {
        _targetZoomDistance = _entireComponentZoomDistance;

        // If we zoom out all the way then use autozoom mode
        if(_viewData._focusNodeId.isNull())
            _viewData._autoZooming = true;
    }

    float startZoomDistance = _viewData._zoomDistance;

    if(visible())
    {
        if(_zoomTransition.finished())
            _graphWidget->rendererStartedTransition();

        _zoomTransition.start(0.1f, Transition::Type::Linear,
            [=](float f)
            {
                _viewData._zoomDistance = startZoomDistance + ((_targetZoomDistance - startZoomDistance) * f);
            },
            [this]
            {
                _graphWidget->rendererFinishedTransition();
            });
    }
    else
        _viewData._zoomDistance = _targetZoomDistance;
}

void GraphComponentRenderer::zoomToDistance(float distance)
{
    distance = std::max(distance, MINIMUM_ZOOM_DISTANCE);
    distance = std::min(distance, _entireComponentZoomDistance);
    _viewData._zoomDistance = _targetZoomDistance = distance;
}

void GraphComponentRenderer::centreNodeInViewport(NodeId nodeId, float cameraDistance)
{
    if(nodeId.isNull())
        return;

    centrePositionInViewport(_graphModel->nodePositions().getScaledAndSmoothed(nodeId),
                             cameraDistance);
}

void GraphComponentRenderer::centrePositionInViewport(const QVector3D& viewTarget,
                                                      float cameraDistance,
                                                      const QVector3D /*viewVector*/)
{
    QVector3D position;

    if(cameraDistance < 0.0f)
    {
        Plane translationPlane(viewTarget, _viewData._camera.viewVector().normalized());

        if(translationPlane.sideForPoint(_viewData._camera.position()) == Plane::Side::Back)
        {
            // We're behind the translation plane, so move along it
            QVector3D cameraPlaneIntersection = translationPlane.rayIntersection(
                        Ray(_viewData._camera.position(), _viewData._camera.viewVector().normalized()));
            QVector3D translation = viewTarget - cameraPlaneIntersection;

            position = _viewData._camera.position() + translation;
        }
        else
        {
            // We're in front of the translation plane, so move directly to the target
            position = viewTarget + (_viewData._camera.position() - _viewData._camera.viewTarget());
        }

        float distanceToTarget = position.distanceToPoint(viewTarget);
        zoomToDistance(distanceToTarget);
    }
    else
    {
        // Given a specific camera distance
        position = viewTarget - (_viewData._camera.viewVector().normalized() * cameraDistance);
    }

    // Enforce minimum camera distance
    if(position.distanceToPoint(viewTarget) < MINIMUM_ZOOM_DISTANCE)
        position = viewTarget - (_viewData._camera.viewVector().normalized() * MINIMUM_ZOOM_DISTANCE);

    // Enforce maximum camera distance
    if(position.distanceToPoint(viewTarget) > _entireComponentZoomDistance)
        position = viewTarget - (_viewData._camera.viewVector().normalized() * _entireComponentZoomDistance);

    if(_graphWidget->transition().finished())
    {
        _viewData._camera.setPosition(position);
        _viewData._camera.setViewTarget(viewTarget);

        _viewData._transitionStart = _viewData._transitionEnd = _viewData._camera;
    }
    else
    {
        _viewData._transitionStart = _viewData._camera;

        _viewData._transitionEnd = _viewData._camera;
        _viewData._transitionEnd.setPosition(position);
        _viewData._transitionEnd.setViewTarget(viewTarget);
    }

    _viewData._camera.setRotation(_viewData._camera.rotation());
}

void GraphComponentRenderer::moveFocusToNode(NodeId nodeId)
{
    if(_componentId.isNull())
        return;

    _viewData._focusNodeId = nodeId;
    _viewData._autoZooming = false;
    updateEntireComponentZoomDistance();

    centreNodeInViewport(nodeId, -1.0f);
}

void GraphComponentRenderer::resetView()
{
    if(_componentId.isNull())
        return;

    _viewData._autoZooming = true;

    moveFocusToCentreOfComponent();
}

void GraphComponentRenderer::moveFocusToCentreOfComponent()
{
    if(_componentId.isNull())
        return;

    _viewData._focusNodeId.setToNull();
    updateFocusPosition();
    updateEntireComponentZoomDistance();

    if(_viewData._autoZooming)
        zoomToDistance(_entireComponentZoomDistance);
    else
        _viewData._zoomDistance = -1.0f;

    centrePositionInViewport(_viewData._focusPosition, _viewData._zoomDistance);
}

void GraphComponentRenderer::moveFocusToNodeClosestCameraVector()
{
    if(_componentId.isNull())
        return;

    Collision collision(*_graphModel, _componentId);
    //FIXME closestNodeToCylinder/Cone?
    NodeId closestNodeId = collision.nodeClosestToLine(_viewData._camera.position(), _viewData._camera.viewVector().normalized());
    if(!closestNodeId.isNull())
        moveFocusToNode(closestNodeId);
}

void GraphComponentRenderer::moveFocusToPositionContainingNodes(const QVector3D& position,
                                                                std::vector<NodeId> nodeIds,
                                                                const QVector3D& viewVector)
{
    if(_componentId.isNull())
        return;

    _viewData._focusNodeId.setToNull();
    _viewData._focusPosition = position;
    _entireComponentZoomDistance = zoomDistanceForNodeIds(position, nodeIds);
    zoomToDistance(_entireComponentZoomDistance);

    centrePositionInViewport(_viewData._focusPosition, _viewData._zoomDistance, viewVector);
}

void GraphComponentRenderer::updateTransition(float f)
{
    _viewData._camera.setPosition(Utils::interpolate(_viewData._transitionStart.position(),
                                                     _viewData._transitionEnd.position(), f));
    _viewData._camera.setViewTarget(Utils::interpolate(_viewData._transitionStart.viewTarget(),
                                                       _viewData._transitionEnd.viewTarget(), f));
}

NodeId GraphComponentRenderer::focusNodeId()
{
    return _viewData._focusNodeId;
}

QVector3D GraphComponentRenderer::focusPosition()
{
    if(_viewData._focusNodeId.isNull())
        return _viewData._focusPosition;
    else
        return _graphModel->nodePositions().getScaledAndSmoothed(_viewData._focusNodeId);
}

bool GraphComponentRenderer::trackingCentreOfComponent()
{
    return _viewData._focusNodeId.isNull();
}

bool GraphComponentRenderer::autoZooming()
{
    return _viewData._autoZooming;
}

void GraphComponentRenderer::prepareVertexBuffers()
{
    // Populate the data buffer object
    _nodePositionBuffer.create();
    _nodePositionBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    _nodePositionBuffer.bind();
    _nodePositionBuffer.allocate(_nodePositionData.data(), static_cast<int>(_nodePositionData.size()) * sizeof(GLfloat));
    _nodePositionBuffer.release();

    _edgePositionBuffer.create();
    _edgePositionBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    _edgePositionBuffer.bind();
    _edgePositionBuffer.allocate(_edgePositionData.data(), static_cast<int>(_edgePositionData.size()) * sizeof(GLfloat));
    _edgePositionBuffer.release();

    _nodeVisualBuffer.create();
    _nodeVisualBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    _nodeVisualBuffer.bind();
    _nodeVisualBuffer.allocate(_nodeVisualData.data(), static_cast<int>(_nodeVisualData.size()) * sizeof(GLfloat));
    _nodeVisualBuffer.release();

    _edgeVisualBuffer.create();
    _edgeVisualBuffer.setUsagePattern(QOpenGLBuffer::StaticDraw);
    _edgeVisualBuffer.bind();
    _edgeVisualBuffer.allocate(_edgeVisualData.data(), static_cast<int>(_edgeVisualData.size()) * sizeof(GLfloat));
    _edgeVisualBuffer.release();

    _debugLinesDataBuffer.create();
    _debugLinesDataBuffer.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    _debugLinesDataBuffer.bind();
    _debugLinesDataBuffer.allocate(_debugLinesData.data(), static_cast<int>(_debugLinesData.size()) * sizeof(GLfloat));
    _debugLinesDataBuffer.release();
}

void GraphComponentRenderer::prepareNodeVAO()
{
    _sphere.vertexArrayObject()->bind();

    QOpenGLShaderProgramPtr shader = _sphere.material()->shader();
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

    _funcs->glVertexAttribDivisor(shader->attributeLocation("point"), 1);
    _funcs->glVertexAttribDivisor(shader->attributeLocation("size"), 1);
    _funcs->glVertexAttribDivisor(shader->attributeLocation("color"), 1);
    _funcs->glVertexAttribDivisor(shader->attributeLocation("outlineColor"), 1);

    _nodeVisualBuffer.release();
    _nodePositionBuffer.release();
    shader->release();
    _sphere.vertexArrayObject()->release();
}

void GraphComponentRenderer::prepareEdgeVAO()
{
    _cylinder.vertexArrayObject()->bind();

    QOpenGLShaderProgramPtr shader = _cylinder.material()->shader();
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

    _funcs->glVertexAttribDivisor(shader->attributeLocation("source"), 1);
    _funcs->glVertexAttribDivisor(shader->attributeLocation("target"), 1);
    _funcs->glVertexAttribDivisor(shader->attributeLocation("size"), 1);
    _funcs->glVertexAttribDivisor(shader->attributeLocation("color"), 1);
    _funcs->glVertexAttribDivisor(shader->attributeLocation("outlineColor"), 1);

    _edgeVisualBuffer.release();
    _edgePositionBuffer.release();
    shader->release();
    _cylinder.vertexArrayObject()->release();
}

void GraphComponentRenderer::prepareDebugLinesVAO()
{
    _debugLinesDataVAO.bind();
    _shared->_debugLinesShader.bind();
    _debugLinesDataBuffer.bind();

    _shared->_debugLinesShader.enableAttributeArray("position");
    _shared->_debugLinesShader.enableAttributeArray("color");
    _shared->_debugLinesShader.setAttributeBuffer("position", GL_FLOAT, 0, 3, 6 * sizeof(GLfloat));
    _shared->_debugLinesShader.setAttributeBuffer("color", GL_FLOAT, 3 * sizeof(GLfloat), 3, 6 * sizeof(GLfloat));

    _debugLinesDataBuffer.release();
    _debugLinesDataVAO.release();
    _shared->_debugLinesShader.release();
}
