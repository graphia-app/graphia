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
    _projection = graphRenderer->projection();

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
    updateCameraProjection(_viewData.camera());
    zoomToDistance(_savedViewData._zoomDistance);

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

QMatrix4x4 GraphComponentRenderer::subViewportMatrix(QRectF sub, QRect viewport)
{
    auto subCentre = sub.center();
    auto viewportF = QRectF(viewport);

    // The translation values effectively represent the centre of
    // sub in terms of viewport, using NDC
    auto xNormalised = static_cast<float>((subCentre.x() - viewportF.x()) / viewportF.width());
    auto yNormalised = static_cast<float>((subCentre.y() - viewportF.y()) / viewportF.height());
    auto xTranslation = (2.0f * xNormalised) - 1.0f;
    auto yTranslation = (2.0f * yNormalised) - 1.0f;

    QMatrix4x4 m;
    m.translate(xTranslation, -yTranslation);

    auto xScale = sub.width() / viewportF.width();
    auto yScale = sub.height() / viewportF.height();
    m.scale(xScale, yScale);

    return m;
}

float GraphComponentRenderer::zoomDistanceForRadius(float radius, Projection projection) const
{
    if(projection == Projection::Unset)
        projection = _projection;

    if(projection == Projection::Perspective)
    {
        float minHalfFov = qDegreesToRadians(std::min(_fovx, _fovy) * 0.5f);

        if(minHalfFov > 0.0f)
            return std::max(radius / std::sin(minHalfFov), MINIMUM_ZOOM_DISTANCE);

        qDebug() << "WARNING: ComponentId" << _componentId << "GraphComponentRenderer fov not set";
        return MINIMUM_ZOOM_DISTANCE;
    }

    return std::max(radius, MINIMUM_ZOOM_DISTANCE);
}

float GraphComponentRenderer::maxDistanceFor(NodeId nodeId,
    const std::vector<NodeId>* nodeIds) const
{
    if(nodeIds == nullptr && componentIsValid())
        nodeIds = &_graphModel->graph().componentById(_componentId)->nodeIds();

    // If we don't have any nodeIds to work with (normally because the
    // component is frozen) we can't go any futher
    if(nodeIds == nullptr)
        return -1.0f;

    QVector3D position = !nodeId.isNull() ?
        _graphModel->nodePositions().get(nodeId) :
        _viewData._componentCentre;

    auto maxDistance = maxNodeDistanceFromPoint(*_graphModel, position, *nodeIds);

    // HACK: allow single node components to be zoomed out beyond their natural maximum
    if(nodeIds->size() == 1)
        maxDistance *= 2.2f;

    return maxDistance;
}

float GraphComponentRenderer::entireComponentZoomDistanceFor(NodeId nodeId,
    const std::vector<NodeId>* nodeIds, Projection projection) const
{
    auto maxDistance = maxDistanceFor(nodeId, nodeIds);
    return zoomDistanceForRadius(maxDistance, projection);
}

void GraphComponentRenderer::updateCentreAndZoomDistance(const std::vector<NodeId>* nodeIds)
{
    if(nodeIds == nullptr && componentIsValid())
        nodeIds = &_graphModel->graph().componentById(_componentId)->nodeIds();

    if(nodeIds == nullptr)
        return;

    _viewData._componentCentre = _graphModel->nodePositions().centreOfMass(*nodeIds);

    auto maxDistance = maxDistanceFor(_viewData._focusNodeId, nodeIds);
    if(maxDistance >= 0.0f)
    {
        _entireComponentZoomDistance = zoomDistanceForRadius(maxDistance);
        _orthoCameraDistance = zoomDistanceForRadius(maxDistance, Projection::Perspective);
        _maxDistanceFromFocus = maxDistance;
    }
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

void GraphComponentRenderer::updateCameraProjection(Camera& camera)
{
    auto aspectRatio = static_cast<float>(_dimensions.width() / _dimensions.height());

    auto minCameraDistance = _viewData.camera().distance();

    if(_zoomTransition.active())
        minCameraDistance = std::min(minCameraDistance, _targetZoomDistance);
    else if(_graphRenderer->transition().active())
        minCameraDistance = std::min(minCameraDistance, _viewData._transitionEnd._camera.distance());

    // Keep the near clipping plane as far away as possible, and in
    // so doing maximise the available depth buffer resolution
    const auto near = std::max(minCameraDistance - _maxDistanceFromFocus, 0.3f);

    //FIXME: ideally this should be calculated too, but it gets complicated with transitions etc.
    const auto far = 50000.0f;

    if(_projection == Projection::Perspective)
    {
        camera.setPerspectiveProjection(_fovy, aspectRatio, near, far);
    }
    else
    {
        auto horizontal = _viewData._zoomDistance * aspectRatio;
        auto vertical = _viewData._zoomDistance;

        camera.setOrthographicProjection(-horizontal, horizontal,
            -vertical, vertical, near, far);
    }
}

QMatrix4x4 GraphComponentRenderer::modelViewMatrix() const
{
    return _viewData.camera().viewMatrix();
}

QMatrix4x4 GraphComponentRenderer::projectionMatrix() const
{
    return subViewportMatrix(_dimensions, {0, 0, _viewportWidth, _viewportHeight}) *
        _viewData.camera().projectionMatrix();
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
    if(dimensions == _dimensions)
        return;

    _dimensions = dimensions;

    auto aspectRatio = static_cast<float>(_dimensions.width() / _dimensions.height());
    _fovy = 60.0f;
    _fovx = _fovy * aspectRatio;

    _viewData.camera().setViewport(_dimensions);
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

void GraphComponentRenderer::centreNodeInViewport(NodeId nodeId, float zoomDistance, QQuaternion rotation)
{
    if(nodeId.isNull())
        return;

    centrePositionInViewport(_graphModel->nodePositions().get(nodeId), zoomDistance, rotation);
}

void GraphComponentRenderer::centrePositionInViewport(const QVector3D& focus,
    float zoomDistance, QQuaternion rotation)
{
    if(zoomDistance < 0.0f)
    {
        if(_projection == Projection::Perspective)
        {
            if(_viewData._autoZooming)
                zoomDistance = _entireComponentZoomDistance;
            else
            {
                // In the perspective case we need to calculate the zoom distance based on
                // the current camera position and its target...

                const QVector3D& oldPosition = _viewData.camera().position();
                QVector3D newPosition;
                Plane translationPlane(focus, _viewData.camera().viewVector());

                if(translationPlane.sideForPoint(oldPosition) == Plane::Side::Back)
                {
                    // We're behind the translation plane, so move along it
                    QVector3D cameraPlaneIntersection = translationPlane.rayIntersection(
                                Ray(oldPosition, _viewData.camera().viewVector()));
                    QVector3D translation = focus - cameraPlaneIntersection;

                    newPosition = oldPosition + translation;
                }
                else
                {
                    // We're in front of the translation plane, so move directly to the target
                    newPosition = focus + (oldPosition - _viewData.camera().focus());
                }

                zoomDistance = newPosition.distanceToPoint(focus);
            }
        }
        else
        {
            // ...but in the ortho case the camera is fixed, so we don't need to bother
            zoomDistance = _viewData._zoomDistance;

            if(_viewData._autoZooming || _entireComponentZoomDistance < zoomDistance)
                zoomDistance = _entireComponentZoomDistance;
        }
    }

    zoomDistance = std::clamp(zoomDistance, MINIMUM_ZOOM_DISTANCE, _entireComponentZoomDistance);

    if(!_zoomTransition.active())
        zoomToDistance(zoomDistance);

    auto cameraDistance = _projection == Projection::Perspective ?
        // In perspective, camera moves
        zoomDistance :
        // In ortho, camera is fixed in a non-intersecting position
        _orthoCameraDistance;

    Q_ASSERT(_graphRenderer != nullptr);

    if(!_graphRenderer->transition().active() || !_viewData.camera().valid())
    {
        _viewData.camera().setDistance(cameraDistance);
        _viewData.camera().setFocus(focus);

        if(!rotation.isNull())
            _viewData.camera().setRotation(rotation);

        updateCameraProjection(_viewData.camera());
        _viewData._cameraAndLighting._lightScale = _orthoCameraDistance;

        _viewData._transitionStart = _viewData._transitionEnd =
            _viewData._cameraAndLighting;
    }
    else
    {
        _viewData._transitionStart = _viewData._cameraAndLighting;

        _viewData._transitionEnd = _viewData._cameraAndLighting;
        _viewData._transitionEnd._camera.setDistance(cameraDistance);
        _viewData._transitionEnd._camera.setFocus(focus);

        if(!rotation.isNull())
            _viewData._transitionEnd._camera.setRotation(rotation);

        updateCameraProjection(_viewData._transitionEnd._camera);
        _viewData._transitionEnd._lightScale = _orthoCameraDistance;
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
    NodeId closestNodeId = collision.nodeClosestToLine(_viewData.camera().position(), _viewData.camera().viewVector());
    if(!closestNodeId.isNull())
        moveFocusToNode(closestNodeId);
}

void GraphComponentRenderer::moveFocusToNodes(const std::vector<NodeId>& nodeIds,
    const QQuaternion& rotation)
{
    _viewData._focusNodeId.setToNull();
    updateCentreAndZoomDistance(&nodeIds);
    zoomToDistance(_entireComponentZoomDistance);

    centrePositionInViewport(_viewData._componentCentre, _viewData._zoomDistance, rotation);
}

void GraphComponentRenderer::doProjectionTransition()
{
    if(!componentIsValid())
        return;

    updateCentreAndZoomDistance();

    QQuaternion rotation = nullQuaternion();

    if(_projection == Projection::TwoDee)
    {
        // When moving to 2D mode, rotate to look along the Z axis, so that
        // there is no perceptible change when the layout is flattened
        auto delta = QQuaternion::rotationTo(camera()->viewVector(), {0.0f, 0.0f, -1.0f});
        rotation = delta * camera()->rotation();
    }

    if(_viewData._focusNodeId.isNull())
        centrePositionInViewport(_viewData._componentCentre, -1.0f, rotation);
    else
        centreNodeInViewport(_viewData._focusNodeId, -1.0f, rotation);
}

bool GraphComponentRenderer::transitionRequired() const
{
    if(trackingCentreOfComponent() || !focusNodeIsVisible())
        return true;

    Q_ASSERT(!_viewData._focusNodeId.isNull());
    auto entireComponentZoomDistance =
        entireComponentZoomDistanceFor(_viewData._focusNodeId);

    return _viewData.camera().distance() > entireComponentZoomDistance;
}

void GraphComponentRenderer::computeTransition()
{
    if(trackingCentreOfComponent() || !focusNodeIsVisible())
        moveFocusToCentreOfComponent();
    else
        moveFocusToNode(focusNodeId());
}

static QMatrix4x4 interpolateProjectionMatrices(const QMatrix4x4& a, const QMatrix4x4& b, float f)
{
    auto m = u::interpolate(a, b, f);

    // If both matrices are affine they are (probably) ortho projections, and we're
    // (probably) zooming, in which case we decompose the scaling and use it for
    // the interpolation, rather than using the cells of the matrix directly
    if(a.isAffine() && b.isAffine())
    {
        for(int column = 0; column < 2; column++)
        {
            for(int row = 0; row < 2; row++)
            {
                auto ai = a(row, column);
                auto bi = b(row, column);

                ai = ai != 0.0f ? 1.0f / ai : ai;
                bi = bi != 0.0f ? 1.0f / bi : bi;
                auto mi = u::interpolate(ai, bi, f);
                m(row, column) = mi != 0.0f ? 1.0f / mi : mi;
            }
        }
    }

    return m;
}

void GraphComponentRenderer::updateTransition(float f)
{
    _viewData.camera().setDistance(u::interpolate(
        _viewData._transitionStart._camera.distance(),
        _viewData._transitionEnd._camera.distance(),
        f));

    _viewData.camera().setFocus(u::interpolate(
        _viewData._transitionStart._camera.focus(),
        _viewData._transitionEnd._camera.focus(),
        f));

    _viewData.camera().setRotation(QQuaternion::slerp(
        _viewData._transitionStart._camera.rotation(),
        _viewData._transitionEnd._camera.rotation(),
        f));

    _viewData.camera().setProjectionMatrix(interpolateProjectionMatrices(
        _viewData._transitionStart._camera.projectionMatrix(),
        _viewData._transitionEnd._camera.projectionMatrix(),
        f));

    _viewData._cameraAndLighting._lightScale = u::interpolate(
        _viewData._transitionStart._lightScale,
        _viewData._transitionEnd._lightScale,
        f);
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
    bool zoomMatchesRadius = qFuzzyCompare(zoomDistanceForRadius(radius),
        _projection == Projection::Perspective ? camera()->distance() : _viewData._zoomDistance);

    return focusNodeId() == nodeId && zoomMatchesRadius;
}

bool GraphComponentRenderer::trackingCentreOfComponent() const
{
    return _viewData._focusNodeId.isNull();
}
