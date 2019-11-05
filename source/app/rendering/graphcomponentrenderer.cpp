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
#include "shared/utils/preferences.h"

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

    const auto& edgeIds = component->edgeIds();
    std::transform(edgeIds.begin(), edgeIds.end(), std::back_inserter(_edges),
    [this](auto edgeId)
    {
        return &_graphModel->graph().edgeById(edgeId);
    });
}

void GraphComponentRenderer::cloneViewDataFrom(const GraphComponentRenderer& other)
{
    _viewData = other._viewData;
}

void GraphComponentRenderer::restoreViewData()
{
    _viewData._autoZooming = _savedViewData._autoZooming;
    _viewData._focusNodeId = _savedViewData._focusNodeId;

    updateCentreAndZoomDistance();

    auto startZoomDistance = _viewData._zoomDistance;
    zoomToDistance(_savedViewData._zoomDistance);

    if(_graphRenderer->projection() != Projection::Perspective)
    {
        _viewData._orthoStartZoomDistance = startZoomDistance;
        _viewData._orthoEndZoomDistance = _savedViewData._zoomDistance;
    }
    else
    {
        _viewData._orthoStartZoomDistance =
            _viewData._orthoEndZoomDistance = -1.0f;
    }

    if(_savedViewData._focusNodeId.isNull())
        centrePositionInViewport(_viewData._componentCentre, _viewData._zoomDistance);
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
        QVector3D nodePosition = graphModel.nodePositions().get(nodeId);
        auto& nodeVisual = graphModel.nodeVisual(nodeId);
        float distance = (centre - nodePosition).length() + nodeVisual._size;

        if(distance > maxDistance)
            maxDistance = distance;
    }

    return maxDistance;
}

float GraphComponentRenderer::zoomDistanceForRadius(float radius) const
{
    if(_graphRenderer->projection() == Projection::Perspective)
    {
        float minHalfFov = qDegreesToRadians(std::min(_fovx, _fovy) * 0.5f);

        if(minHalfFov > 0.0f)
            return std::max(radius / std::sin(minHalfFov), MINIMUM_ZOOM_DISTANCE);

        qDebug() << "WARNING: ComponentId" << _componentId << "GraphComponentRenderer fov not set";
        return MINIMUM_ZOOM_DISTANCE;
    }

    return std::max(radius, MINIMUM_ZOOM_DISTANCE);
}

float GraphComponentRenderer::entireComponentZoomDistanceFor(NodeId nodeId,
    const std::vector<NodeId>* nodeIds) const
{
    QVector3D position = !nodeId.isNull() ?
        _graphModel->nodePositions().get(nodeId) :
        _viewData._componentCentre;

    if(nodeIds == nullptr && componentIsValid())
        nodeIds = &_graphModel->graph().componentById(_componentId)->nodeIds();

    // If we don't have any nodeIds to work with (normally because the
    // component is frozen) we can't go any futher
    if(nodeIds == nullptr)
        return -1.0f;

    auto maxDistance = maxNodeDistanceFromPoint(*_graphModel, position, *nodeIds);

    // Allow single node components to be zoomed out beyond their natural maximum
    if(nodeIds->size() == 1)
        maxDistance *= 2.2f;

    return zoomDistanceForRadius(maxDistance);
}

void GraphComponentRenderer::updateCentreAndZoomDistance(const std::vector<NodeId>* nodeIds)
{
    if(nodeIds == nullptr && componentIsValid())
        nodeIds = &_graphModel->graph().componentById(_componentId)->nodeIds();

    if(nodeIds == nullptr)
        return;

    _viewData._componentCentre = _graphModel->nodePositions().centreOfMass(*nodeIds);

    auto distance = entireComponentZoomDistanceFor(_viewData._focusNodeId, nodeIds);
    if(distance >= 0.0f)
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
                updateCentreAndZoomDistance();
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

                centrePositionInViewport(_viewData._componentCentre, _viewData._zoomDistance);
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

    if(_graphRenderer->projection() == Projection::Perspective)
    {
        _viewData._camera.setPerspectiveProjection(_fovy, aspectRatio, 0.3f, 50000.0f);
    }
    else
    {
        auto horizontal = _viewData._zoomDistance * aspectRatio;
        auto vertical = _viewData._zoomDistance;

        _viewData._camera.setOrthographicProjection(-horizontal, horizontal,
            -vertical, vertical, 0.3f, 50000.0f);
    }

    _viewData._camera.setViewport(_dimensions);

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

    if(visible() && doTransition)
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

void GraphComponentRenderer::zoomToDistance(float distance)
{
    distance = std::clamp(distance, MINIMUM_ZOOM_DISTANCE, _entireComponentZoomDistance);
    _viewData._zoomDistance = _targetZoomDistance = distance;
}

void GraphComponentRenderer::centreNodeInViewport(NodeId nodeId, float zoomDistance)
{
    if(nodeId.isNull())
        return;

    centrePositionInViewport(_graphModel->nodePositions().get(nodeId), zoomDistance);
}

void GraphComponentRenderer::centrePositionInViewport(const QVector3D& focus,
                                                      float zoomDistance,
                                                      QQuaternion rotation)
{
    if(zoomDistance < 0.0f)
    {
        if(_graphRenderer->projection() == Projection::Perspective)
        {
            if(_viewData._autoZooming)
                zoomDistance = _entireComponentZoomDistance;
            else
            {
                // In the perspective case we need to calculate the zoom distance based on
                // the current camera position and its target...

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
        }
        else
        {
            // ...but in the ortho case the camera is fixed, so we don't need to bother
            zoomDistance = _viewData._zoomDistance;

            if(_viewData._autoZooming || _entireComponentZoomDistance < zoomDistance)
            {
                _viewData._orthoStartZoomDistance = zoomDistance;
                _viewData._orthoEndZoomDistance = _entireComponentZoomDistance;
            }
        }
    }

    zoomDistance = std::clamp(zoomDistance, MINIMUM_ZOOM_DISTANCE, _entireComponentZoomDistance);

    if(!_zoomTransition.active())
        zoomToDistance(zoomDistance);

    auto cameraDistance = _graphRenderer->projection() == Projection::Perspective ?
        // In perspective, camera moves
        zoomDistance :
        // In ortho, camera is fixed in a non-intersecting position
        _entireComponentZoomDistance * 3.0f;

    Q_ASSERT(_graphRenderer != nullptr);

    if(!_graphRenderer->transition().active() || !_viewData._camera.valid())
    {
        _viewData._camera.setDistance(cameraDistance);
        _viewData._camera.setFocus(focus);
        if(!rotation.isNull())
            _viewData._camera.setRotation(rotation);

        _viewData._transitionStart = _viewData._transitionEnd = _viewData._camera;
        _viewData._orthoStartZoomDistance = _viewData._orthoEndZoomDistance =
            _viewData._zoomDistance;
    }
    else
    {
        _viewData._transitionStart = _viewData._camera;

        _viewData._transitionEnd = _viewData._camera;
        _viewData._transitionEnd.setDistance(cameraDistance);
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
    updateCentreAndZoomDistance();

    float zoomDistance = -1.0f;
    if(radius >= 0.0f)
        zoomDistance = zoomDistanceForRadius(radius);

    centreNodeInViewport(nodeId, zoomDistance);
}

void GraphComponentRenderer::moveSavedFocusToNode(NodeId nodeId)
{
    if(!componentIsValid())
        return;

    _savedViewData._focusNodeId = nodeId;
    _savedViewData._autoZooming = false;

    float zoomDistance = entireComponentZoomDistanceFor(_savedViewData._focusNodeId);

    _savedViewData._zoomDistance = _targetZoomDistance = zoomDistance;
    _savedViewData._orthoStartZoomDistance =
        _savedViewData._orthoEndZoomDistance = -1.0f;
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
    updateCentreAndZoomDistance();

    centrePositionInViewport(_viewData._componentCentre);
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

void GraphComponentRenderer::moveFocusToNodes(const std::vector<NodeId>& nodeIds,
    const QQuaternion& rotation)
{
    auto zoomDistance = _viewData._zoomDistance;

    _viewData._focusNodeId.setToNull();
    updateCentreAndZoomDistance(&nodeIds);
    zoomToDistance(_entireComponentZoomDistance);

    if(_graphRenderer->projection() != Projection::Perspective)
    {
        _viewData._orthoStartZoomDistance = zoomDistance;
        _viewData._orthoEndZoomDistance = _viewData._zoomDistance;
    }
    else
    {
        _viewData._orthoStartZoomDistance =
            _viewData._orthoEndZoomDistance = -1.0f;
    }

    centrePositionInViewport(_viewData._componentCentre, _viewData._zoomDistance, rotation);
}

bool GraphComponentRenderer::transitionRequired()
{
    if(trackingCentreOfComponent() || !focusNodeIsVisible())
        return true;

    //FIXME What's this doing here? Will it be executed on the correct thread?
    // It's preventing transitionRequired being const
    updateCentreAndZoomDistance();

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

    if(_graphRenderer->projection() != Projection::Perspective)
    {
        _viewData._zoomDistance = u::interpolate(_viewData._orthoStartZoomDistance,
            _viewData._orthoEndZoomDistance, f);
    }
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
        return _viewData._componentCentre;

    return _graphModel->nodePositions().get(_viewData._focusNodeId);
}

bool GraphComponentRenderer::focusedOnNodeAtRadius(NodeId nodeId, float radius) const
{
    return focusNodeId() == nodeId && qFuzzyCompare(zoomDistanceForRadius(radius), camera()->distance());
}

bool GraphComponentRenderer::trackingCentreOfComponent() const
{
    return _viewData._focusNodeId.isNull();
}
