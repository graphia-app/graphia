#include "graphcomponentrenderer.h"
#include "graphrenderer.h"

#include "camera.h"

#include "graph/graph.h"
#include "graph/graphmodel.h"
#include "graph/componentmanager.h"

#include "layout/layout.h"
#include "layout/collision.h"

#include "maths/boundingsphere.h"
#include "maths/frustum.h"
#include "maths/plane.h"

#include "ui/graphquickitem.h"
#include "ui/selectionmanager.h"
#include "ui/visualisations/elementvisual.h"

#include "shared/graph/elementid_debug.h"

#include <QObject>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QtMath>

#include <cmath>
#include <mutex>
#include <algorithm>

// This value should be larger than the maximum node size
const float GraphComponentRenderer::MINIMUM_ZOOM_DISTANCE = 3.5f;
const float GraphComponentRenderer::COMFORTABLE_ZOOM_RADIUS = MINIMUM_ZOOM_DISTANCE * 2.0f;

void GraphComponentRenderer::initialise(GraphModel* graphModel, ComponentId componentId,
                                        SelectionManager* selectionManager,
                                        GraphRenderer* graphRenderer)
{
    if(_initialised)
    {
        synchronise();
        return;
    }

    _graphModel = graphModel;
    _componentId = componentId;
    _selectionManager = selectionManager;
    _graphRenderer = graphRenderer;

    _viewData = {};
    _targetZoomDistance = _viewData._zoomDistance;
    _moveFocusToCentreOfComponentLater = true;

    synchronise();

    QObject::connect(&_zoomTransition, &Transition::started, graphRenderer,
                     &GraphRenderer::rendererStartedTransition, Qt::DirectConnection);
    QObject::connect(&_zoomTransition, &Transition::finished, graphRenderer,
                     &GraphRenderer::rendererFinishedTransition, Qt::DirectConnection);

    _initialised = true;
    _visible = false;
}

void GraphComponentRenderer::setVisible(bool visible)
{
    _visible = visible;
}

void GraphComponentRenderer::cleanup()
{
    if(!_initialised)
        return;

    if(_frozen)
    {
        _cleanupWhenThawed = true;
        return;
    }

    _zoomTransition.disconnect();

    _graphRenderer->onComponentCleanup(_componentId);

    _nodeIds.clear();
    _edges.clear();

    _graphModel = nullptr;
    _componentId.setToNull();
    _selectionManager = nullptr;
    _graphRenderer = nullptr;

    _initialised = false;
    _visible = false;
}

void GraphComponentRenderer::synchronise()
{
    if(_frozen)
    {
        _synchroniseWhenThawed = true;
        return;
    }

    _nodeIds.clear();
    _edges.clear();

    auto component = _graphModel->graph().componentById(_componentId);
    Q_ASSERT(component != nullptr);

    _nodeIds = component->nodeIds();
    for(auto edgeId : component->edgeIds())
        _edges.emplace_back(&_graphModel->graph().edgeById(edgeId));
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
    if(!_frozen)
        return;

    _frozen = false;

    if(_synchroniseWhenThawed && !_cleanupWhenThawed)
    {
        synchronise();
        _synchroniseWhenThawed = false;
    }

    if(_cleanupWhenThawed)
    {
        cleanup();
        _cleanupWhenThawed = false;
    }
}

float GraphComponentRenderer::maxNodeDistanceFromPoint(const GraphModel& graphModel,
    const QVector3D& centre, const std::vector<NodeId>& nodeIds)
{
    float maxDistance = std::numeric_limits<float>::lowest();
    for(auto nodeId : nodeIds)
    {
        QVector3D nodePosition = graphModel.nodePositions().getScaledAndSmoothed(nodeId);
        auto& nodeVisual = graphModel.nodeVisual(nodeId);
        float distance = (centre - nodePosition).length() + nodeVisual._size;

        if(distance > maxDistance)
            maxDistance = distance;
    }

    return maxDistance;
}

QRectF GraphComponentRenderer::dimensions() const
{
    return _dimensions;
}

float GraphComponentRenderer::zoomDistanceForRadius(float radius) const
{
    float minHalfFov = qDegreesToRadians(std::min(_fovx, _fovy) * 0.5f);

    if(minHalfFov > 0.0f)
        return std::max(radius / std::sin(minHalfFov), MINIMUM_ZOOM_DISTANCE);

    qDebug() << "WARNING: ComponentId" << _componentId << "GraphComponentRenderer fov not set";
    return MINIMUM_ZOOM_DISTANCE;
}

void GraphComponentRenderer::updateFocusPosition()
{
    if(!componentIsValid())
        return;

    auto component = _graphModel->graph().componentById(_componentId);
    Q_ASSERT(component != nullptr);
    _viewData._focusPosition = NodePositions::centreOfMassScaledAndSmoothed(_graphModel->nodePositions(),
                                                                            component->nodeIds());
}

float GraphComponentRenderer::entireComponentZoomDistanceFor(NodeId nodeId)
{
    // If we're frozen then it's probably because the component is going away, and if
    // this is the case then we won't be able to get at its nodeIds, so don't update
    if(!componentIsValid())
        return -1.0f;

    QVector3D position;

    if(!nodeId.isNull())
    {
        position = _graphModel->nodePositions().getScaledAndSmoothed(nodeId);
    }
    else
    {
        updateFocusPosition();
        position = _viewData._focusPosition;
    }

    auto component = _graphModel->graph().componentById(_componentId);
    auto maxDistance = maxNodeDistanceFromPoint(*_graphModel, position, component->nodeIds());

    return zoomDistanceForRadius(maxDistance);
}

void GraphComponentRenderer::updateEntireComponentZoomDistance()
{
    auto distance = entireComponentZoomDistanceFor(_viewData._focusNodeId);
    if(distance < 0.0f)
        return;

    _entireComponentZoomDistance = distance;
}

void GraphComponentRenderer::update(float t)
{
    Q_ASSERT(_initialised);

    if(_graphModel != nullptr)
    {
        Q_ASSERT(_graphRenderer != nullptr);

        if(!_frozen)
        {
            if(_graphRenderer->layoutChanged())
            {
                updateEntireComponentZoomDistance();
            }
            else if(_moveFocusToCentreOfComponentLater)
            {
                moveFocusToCentreOfComponent();
                _moveFocusToCentreOfComponentLater = false;
            }
        }

        _zoomTransition.update(t);

        if(!_graphRenderer->transition().active() && _trackFocus)
        {
            if(trackingCentreOfComponent())
            {
                if(!_zoomTransition.active() && _viewData._autoZooming)
                    zoomToDistance(_entireComponentZoomDistance);

                centrePositionInViewport(_viewData._focusPosition, _viewData._zoomDistance);
            }
            else
                centreNodeInViewport(_viewData._focusNodeId, _viewData._zoomDistance);
        }
    }
}

QMatrix4x4 GraphComponentRenderer::subViewportMatrix() const
{
    QMatrix4x4 m;

    float xTranslation = (static_cast<float>(_dimensions.x() * 2.0 + _dimensions.width()) /
        static_cast<float>(_viewportWidth)) - 1.0f;
    float yTranslation = (static_cast<float>(_dimensions.y() * 2.0 + _dimensions.height()) /
        static_cast<float>(_viewportHeight)) - 1.0f;
    m.translate(xTranslation, -yTranslation);

    float xScale = static_cast<float>(_dimensions.width()) / static_cast<float>(_viewportWidth);
    float yScale = static_cast<float>(_dimensions.height()) / static_cast<float>(_viewportHeight);
    m.scale(xScale, yScale);

    return m;
}

QMatrix4x4 GraphComponentRenderer::modelViewMatrix() const
{
    return _viewData._camera.viewMatrix();
}

QMatrix4x4 GraphComponentRenderer::projectionMatrix() const
{
    return subViewportMatrix() * _viewData._camera.projectionMatrix();
}

void GraphComponentRenderer::setViewportSize(int viewportWidth, int viewportHeight)
{
    if(_initialised && viewportWidth > 0 && viewportHeight > 0)
    {
        _viewportWidth = viewportWidth;
        _viewportHeight = viewportHeight;
    }
    else
    {
        qWarning() << "GraphComponentRenderer::resize(" << viewportWidth << "," << viewportHeight <<
                    ") failed _initialised:" << _initialised;
    }
}

void GraphComponentRenderer::setDimensions(const QRectF& dimensions)
{
    _dimensions = dimensions;

    auto aspectRatio = static_cast<float>(_dimensions.width() / _dimensions.height());
    _fovy = 60.0f;
    _fovx = _fovy * aspectRatio;

    _viewData._camera.setPerspectiveProjection(_fovy, aspectRatio, 0.3f, 50000.0f);
    _viewData._camera.setViewportWidth(_dimensions.width());
    _viewData._camera.setViewportHeight(_dimensions.height());
}

bool GraphComponentRenderer::transitionActive()
{
    return _graphRenderer->transition().active() || _zoomTransition.active();
}

void GraphComponentRenderer::setAlpha(float alpha)
{
    if(_alpha != alpha)
    {
        _alpha = alpha;

        Q_ASSERT(_graphRenderer != nullptr);
        _graphRenderer->onComponentAlphaChanged(_componentId);
    }
}

void GraphComponentRenderer::zoom(float delta, bool doTransition)
{
    Q_ASSERT(_graphRenderer != nullptr);

    if(delta == 0.0f || _graphRenderer->transition().active())
        return;

    // Don't allow zooming out if autozooming
    if(delta < 0.0f && _viewData._autoZooming)
        return;

    float size = 0.0f;

    _viewData._autoZooming = false;

    if(!_viewData._focusNodeId.isNull())
        size = _graphModel->nodeVisual(_viewData._focusNodeId)._size;

    const float INTERSECTION_AVOIDANCE_OFFSET = 1.0f;
    const float MIN_SEPARATION = 2.0f;
    auto separation = std::max(((_targetZoomDistance - size) - INTERSECTION_AVOIDANCE_OFFSET), MIN_SEPARATION);
    auto step = delta * separation;

    _targetZoomDistance -= step;
    _targetZoomDistance = std::max(_targetZoomDistance, MINIMUM_ZOOM_DISTANCE);

    if(_targetZoomDistance > _entireComponentZoomDistance)
    {
        _targetZoomDistance = _entireComponentZoomDistance;

        // If we zoom out all the way then use autozoom mode
        if(_viewData._focusNodeId.isNull())
            _viewData._autoZooming = true;
    }

    float startZoomDistance = _viewData._zoomDistance;

    // Don't do anything if there would be no change
    if(qFuzzyCompare(startZoomDistance, _targetZoomDistance))
        return;

    if(visible())
    {
        if(doTransition)
        {
            _zoomTransition.start(0.1f, Transition::Type::Linear,
            [this, startZoomDistance](float f)
            {
                _viewData._zoomDistance = startZoomDistance + ((_targetZoomDistance - startZoomDistance) * f);
            });
        }
        else
            _viewData._zoomDistance = _targetZoomDistance;
    }
    else
        _viewData._zoomDistance = _targetZoomDistance;
}

void GraphComponentRenderer::zoomToDistance(float distance)
{
    distance = std::clamp(distance, MINIMUM_ZOOM_DISTANCE, _entireComponentZoomDistance);
    _viewData._zoomDistance = _targetZoomDistance = distance;
}

void GraphComponentRenderer::centreNodeInViewport(NodeId nodeId, float zoomDistance)
{
    if(nodeId.isNull())
        return;

    centrePositionInViewport(_graphModel->nodePositions().getScaledAndSmoothed(nodeId),
                             zoomDistance);
}

void GraphComponentRenderer::centrePositionInViewport(const QVector3D& focus,
                                                      float zoomDistance,
                                                      QQuaternion rotation)
{
    if(zoomDistance < 0.0f)
    {
        const QVector3D& oldPosition = _viewData._camera.position();
        QVector3D newPosition;
        Plane translationPlane(focus, _viewData._camera.viewVector());

        if(translationPlane.sideForPoint(oldPosition) == Plane::Side::Back)
        {
            // We're behind the translation plane, so move along it
            QVector3D cameraPlaneIntersection = translationPlane.rayIntersection(
                        Ray(oldPosition, _viewData._camera.viewVector()));
            QVector3D translation = focus - cameraPlaneIntersection;

            newPosition = oldPosition + translation;
        }
        else
        {
            // We're in front of the translation plane, so move directly to the target
            newPosition = focus + (oldPosition - _viewData._camera.focus());
        }

        zoomDistance = newPosition.distanceToPoint(focus);
    }

    zoomDistance = std::clamp(zoomDistance, MINIMUM_ZOOM_DISTANCE, _entireComponentZoomDistance);

    if(!_zoomTransition.active())
        zoomToDistance(zoomDistance);

    Q_ASSERT(_graphRenderer != nullptr);

    if(!_graphRenderer->transition().active() || !_viewData._camera.valid())
    {
        _viewData._camera.setDistance(zoomDistance);
        _viewData._camera.setFocus(focus);
        if(!rotation.isNull())
            _viewData._camera.setRotation(rotation);

        _viewData._transitionStart = _viewData._transitionEnd = _viewData._camera;
    }
    else
    {
        _viewData._transitionStart = _viewData._camera;

        _viewData._transitionEnd = _viewData._camera;
        _viewData._transitionEnd.setDistance(zoomDistance);
        _viewData._transitionEnd.setFocus(focus);
        if(!rotation.isNull())
            _viewData._transitionEnd.setRotation(rotation);
    }
}

void GraphComponentRenderer::moveFocusToNode(NodeId nodeId, float radius)
{
    if(!componentIsValid())
        return;

    _viewData._focusNodeId = nodeId;
    _viewData._autoZooming = false;
    updateEntireComponentZoomDistance();

    float zoomDistance = -1.0f;
    if(radius >= 0.0f)
        zoomDistance = zoomDistanceForRadius(radius);

    centreNodeInViewport(nodeId, zoomDistance);
}

void GraphComponentRenderer::moveSavedFocusToNode(NodeId nodeId, float radius)
{
    if(!componentIsValid())
        return;

    _savedViewData._focusNodeId = nodeId;
    _savedViewData._autoZooming = false;

    float zoomDistance = entireComponentZoomDistanceFor(_savedViewData._focusNodeId);
    if(radius >= 0.0f)
        zoomDistance = zoomDistanceForRadius(radius);

    _savedViewData._zoomDistance = _targetZoomDistance = zoomDistance;
}

void GraphComponentRenderer::resetView()
{
    if(!componentIsValid())
        return;

    _viewData._autoZooming = true;

    moveFocusToCentreOfComponent();
}

void GraphComponentRenderer::moveFocusToCentreOfComponent()
{
    if(!componentIsValid())
        return;

    _viewData._focusNodeId.setToNull();
    updateEntireComponentZoomDistance();

    if(_viewData._autoZooming)
        zoomToDistance(_entireComponentZoomDistance);
    else
        _viewData._zoomDistance = -1.0f;

    centrePositionInViewport(_viewData._focusPosition, _viewData._zoomDistance);
}

void GraphComponentRenderer::moveFocusToNodeClosestCameraVector()
{
    if(!componentIsValid())
        return;

    Collision collision(*_graphModel, _componentId);
    //FIXME closestNodeToCylinder/Cone?
    NodeId closestNodeId = collision.nodeClosestToLine(_viewData._camera.position(), _viewData._camera.viewVector());
    if(!closestNodeId.isNull())
        moveFocusToNode(closestNodeId);
}

void GraphComponentRenderer::moveFocusToPositionAndRadius(const QVector3D& position, float radius,
                                                          const QQuaternion& rotation)
{
    _viewData._focusNodeId.setToNull();
    _viewData._focusPosition = position;
    _entireComponentZoomDistance = zoomDistanceForRadius(radius);
    zoomToDistance(_entireComponentZoomDistance);

    centrePositionInViewport(_viewData._focusPosition, _viewData._zoomDistance, rotation);
}

bool GraphComponentRenderer::transitionRequired()
{
    if(trackingCentreOfComponent() || !focusNodeIsVisible())
        return true;

    updateEntireComponentZoomDistance();

    return _viewData._camera.distance() > _entireComponentZoomDistance;
}

void GraphComponentRenderer::computeTransition()
{
    if(trackingCentreOfComponent() || !focusNodeIsVisible())
        moveFocusToCentreOfComponent();
    else
        moveFocusToNode(focusNodeId());
}

void GraphComponentRenderer::updateTransition(float f)
{
    _viewData._camera.setDistance(u::interpolate(_viewData._transitionStart.distance(),
                                                 _viewData._transitionEnd.distance(), f));
    _viewData._camera.setFocus(u::interpolate(_viewData._transitionStart.focus(),
                                              _viewData._transitionEnd.focus(), f));
    _viewData._camera.setRotation(QQuaternion::slerp(_viewData._transitionStart.rotation(),
                                                     _viewData._transitionEnd.rotation(), f));
}

NodeId GraphComponentRenderer::focusNodeId() const
{
    return _viewData._focusNodeId;
}

bool GraphComponentRenderer::focusNodeIsVisible() const
{
    return _graphModel->graph().typeOf(focusNodeId()) != MultiElementType::Tail;
}

QVector3D GraphComponentRenderer::focusPosition() const
{
    if(_viewData._focusNodeId.isNull())
        return _viewData._focusPosition;

    return _graphModel->nodePositions().getScaledAndSmoothed(_viewData._focusNodeId);
}

bool GraphComponentRenderer::focusedOnNodeAtRadius(NodeId nodeId, float radius) const
{
    return focusNodeId() == nodeId && zoomDistanceForRadius(radius) == camera()->distance();
}

bool GraphComponentRenderer::trackingCentreOfComponent() const
{
    return _viewData._focusNodeId.isNull();
}
