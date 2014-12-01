#include "graphcomponentinteractor.h"
#include "selectionmanager.h"

#include "../commands/commandmanager.h"

#include "../rendering/graphcomponentscene.h"
#include "../rendering/openglwindow.h"

#include "../graph/graph.h"
#include "../graph/graphmodel.h"

#include "../maths/frustum.h"
#include "../maths/plane.h"
#include "../maths/boundingsphere.h"

#include "../layout/collision.h"

#include "../utils/utils.h"

#include <QVBoxLayout>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QtMath>
#include <cmath>

static ElementIdSet<NodeId> nodeIdsInsideFrustum(const GraphModel& graphModel,
                                                 ComponentId componentId,
                                                 const BaseFrustum& frustum)
{
    ElementIdSet<NodeId> selection;

    auto component = graphModel.graph().componentById(componentId);
    for(NodeId nodeId : component->nodeIds())
    {
        const QVector3D nodePosition = graphModel.nodePositions().getScaledAndSmoothed(nodeId);
        if(frustum.containsPoint(nodePosition))
            selection.insert(nodeId);
    }

    return selection;
}

static NodeId nodeIdInsideFrustumNearestPoint(const GraphModel& graphModel,
                                              ComponentId componentId,
                                              const BaseFrustum& frustum,
                                              const QVector3D& point)
{
    auto nodeIds = nodeIdsInsideFrustum(graphModel, componentId, frustum);

    NodeId closestNodeId;
    float minimumDistance = std::numeric_limits<float>::max();

    for(auto nodeId : nodeIds)
    {
        float distanceToCentre = Ray(frustum.centreLine()).distanceTo(point);
        float distanceToPoint = graphModel.nodePositions().getScaledAndSmoothed(nodeId).distanceToPoint(point);
        float distance = distanceToCentre + distanceToPoint;

        if(distance < minimumDistance)
        {
            minimumDistance = distance;
            closestNodeId = nodeId;
        }
    }

    return closestNodeId;
}

GraphComponentInteractor::GraphComponentInteractor(std::shared_ptr<GraphModel> graphModel,
                                                   std::shared_ptr<GraphComponentScene> graphComponentScene,
                                                   CommandManager &commandManager,
                                                   std::shared_ptr<SelectionManager> selectionManager) :
    Interactor(),
    _graphModel(graphModel),
    _scene(graphComponentScene),
    _commandManager(commandManager),
    _selectionManager(selectionManager),
    _rightMouseButtonHeld(false),
    _leftMouseButtonHeld(false),
    _selecting(false),
    _frustumSelecting(false),
    _mouseMoving(false)
{
    _scene->setGraphModel(graphModel);
    _scene->setSelectionManager(selectionManager);
    connect(_selectionManager.get(), &SelectionManager::selectionChanged, _scene.get(), &GraphComponentScene::onSelectionChanged);
    connect(_scene.get(), &Scene::userInteractionStarted, this, &Interactor::userInteractionStarted);
    connect(_scene.get(), &Scene::userInteractionFinished, this, &Interactor::userInteractionFinished);
}

void GraphComponentInteractor::mousePressEvent(QMouseEvent* mouseEvent)
{
    _cursorPosition = _prevCursorPosition = _clickPosition = mouseEvent->pos();

    Ray ray = _scene->camera()->rayForViewportCoordinates(_cursorPosition.x(), _cursorPosition.y());

    Collision collision(*_graphModel, _scene->focusComponentId());
    _clickedNodeId = collision.nearestNodeIntersectingLine(ray.origin(), ray.dir());

    if(_clickedNodeId.isNull())
    {
        const int PICK_RADIUS = 40;
        ConicalFrustum frustum = _scene->camera()->conicalFrustumForViewportCoordinates(
                    _cursorPosition.x(), _cursorPosition.y(), PICK_RADIUS);

        _nearClickNodeId = nodeIdInsideFrustumNearestPoint(*_graphModel, _scene->focusComponentId(),
                                                           frustum, ray.origin());
    }
    else
        _nearClickNodeId = _clickedNodeId;

    switch(mouseEvent->button())
    {
    case Qt::LeftButton:
        _leftMouseButtonHeld = true;
        _selecting = true;

        if(mouseEvent->modifiers() & Qt::ShiftModifier)
            _frustumSelectStart = _cursorPosition;
        else
            _frustumSelectStart = QPoint();
        break;

    case Qt::RightButton:
        _rightMouseButtonHeld = true;

        if(!_nearClickNodeId.isNull())
            _scene->disableFocusTracking();
        break;

    default: break;
    }
}

void GraphComponentInteractor::mouseReleaseEvent(QMouseEvent* mouseEvent)
{
    switch(mouseEvent->button())
    {
    case Qt::RightButton:
        if(!_scene->transitioning())
        {
            _rightMouseButtonHeld = false;
            _scene->enableFocusTracking();
            return;
        }

        emit userInteractionFinished();

        if(!_nearClickNodeId.isNull() && _rightMouseButtonHeld && _mouseMoving)
            _scene->selectFocusNodeClosestToCameraVector();

        _rightMouseButtonHeld = false;
        _scene->enableFocusTracking();
        _clickedNodeId.setToNull();
        _nearClickNodeId.setToNull();
        break;

    case Qt::LeftButton:
        _leftMouseButtonHeld = false;

        if(!_scene->transitioning())
            return;

        emit userInteractionFinished();

        if(_selecting)
        {
            if(_frustumSelecting)
            {
                QPoint frustumSelectEndPoint = mouseEvent->pos();
                Frustum frustum = _scene->camera()->frustumForViewportCoordinates(
                            _frustumSelectStart.x(), _frustumSelectStart.y(),
                            frustumSelectEndPoint.x(), frustumSelectEndPoint.y());

                auto selection = nodeIdsInsideFrustum(*_graphModel, _scene->focusComponentId(), frustum);

                auto previousSelection = _selectionManager->selectedNodes();
                _commandManager.execute(tr("Select Nodes"), tr("Selecting Nodes"),
                    [this, selection](Command& command)
                    {
                        bool nodesSelected = _selectionManager->selectNodes(selection);
                        command.setPastParticiple(_selectionManager->numNodesSelectedAsString());
                        return nodesSelected;
                    },
                    [this, previousSelection](Command&) { _selectionManager->setSelectedNodes(previousSelection); });

                _frustumSelectStart = QPoint();
                _frustumSelecting = false;
                _scene->clearSelectionRect();
            }
            else
            {
                if(!_clickedNodeId.isNull())
                {
                    bool nodeSelected = _selectionManager->nodeIsSelected(_clickedNodeId);
                    bool toggling = mouseEvent->modifiers() & Qt::ShiftModifier;
                    auto previousSelection = _selectionManager->selectedNodes();
                    auto toggleNodeId = _clickedNodeId;
                    _commandManager.executeSynchronous(nodeSelected ? tr("Deselect Node") : tr("Select Node"),
                        [this, nodeSelected, toggling, toggleNodeId](Command& command)
                        {
                            if(!toggling)
                                _selectionManager->clearNodeSelection();

                            _selectionManager->toggleNode(toggleNodeId);

                            if(!nodeSelected)
                                command.setPastParticiple(_selectionManager->numNodesSelectedAsString());
                            return true;
                        },
                        [this, previousSelection](Command&) { _selectionManager->setSelectedNodes(previousSelection); });
                }
                else
                {
                    auto previousSelection = _selectionManager->selectedNodes();
                    _commandManager.executeSynchronous(tr("Select None"),
                        [this](Command&) { return _selectionManager->clearNodeSelection(); },
                        [this, previousSelection](Command&) { _selectionManager->setSelectedNodes(previousSelection); });
                }
            }

            _selecting = false;
        }

        _clickedNodeId.setToNull();
        _nearClickNodeId.setToNull();
        break;

    default: break;
    }

    if(_mouseMoving && !_rightMouseButtonHeld && !_leftMouseButtonHeld)
        _mouseMoving = false;
}

// https://www.opengl.org/wiki/Object_Mouse_Trackball
static QVector3D virtualTrackballVector(int width, int height, const QPoint& cursor)
{
    const int minDimension = std::min(width, height);
    const float x = (2.0f * cursor.x() - width) / minDimension;
    const float y = (height - 2.0f * cursor.y()) / minDimension;
    const float d = std::sqrt(x * x + y * y);
    const float RADIUS = 0.9f; // Radius of trackball
    const float ROOT2 = std::sqrt(2.0f);
    const float CUTOFF = ROOT2 / 2.0f;

    float z;

    if(d < (RADIUS * CUTOFF))
    {
        // Sphere
        z = std::sqrt(RADIUS * RADIUS - d * d);
    }
    else
    {
        // Hyperbolic sheet
        float t = RADIUS / ROOT2;
        z = t * t / d;
    }

    return QVector3D(x, y, z);
}

QQuaternion GraphComponentInteractor::mouseMoveToRotation()
{
    QVector3D previous = virtualTrackballVector(_scene->width(), _scene->height(), _prevCursorPosition);
    QVector3D current = virtualTrackballVector(_scene->width(), _scene->height(), _cursorPosition);

    QVector3D axis = QVector3D::crossProduct(previous, current).normalized();

    float dot = QVector3D::dotProduct(previous, current);
    float value = dot / (previous.length() * current.length());
    value = Utils::clamp(-1.0f, 1.0f, value);
    float radians = std::acos(value);
    float angle = -qRadiansToDegrees(radians);

    auto m = _scene->camera()->viewMatrix();
    m.setColumn(3, QVector4D(0.0f, 0.0f, 0.0f, 1.0f));

    QQuaternion rotation = QQuaternion::fromAxisAndAngle(axis * m, angle);

    return rotation;
}

void GraphComponentInteractor::mouseMoveEvent(QMouseEvent* mouseEvent)
{
    if(!_scene->transitioning())
        return;

    Camera* camera = _scene->camera();
    _cursorPosition = mouseEvent->pos();

    if(!_mouseMoving)
    {
        const int MIN_MANHATTAN_MOVE = 3;
        QPoint p = _cursorPosition - _clickPosition;
        if(p.manhattanLength() <= MIN_MANHATTAN_MOVE)
            return;
    }

    if(_leftMouseButtonHeld && (mouseEvent->modifiers() & Qt::ShiftModifier))
    {
        if(!_frustumSelecting)
        {
            emit userInteractionStarted();
            _frustumSelecting = true;
            _clickedNodeId.setToNull();
            _nearClickNodeId.setToNull();

            // This can happen if the user holds shift after clicking
            if(_frustumSelectStart.isNull())
                _frustumSelectStart = _cursorPosition;
        }

        _scene->setSelectionRect(QRect(_frustumSelectStart, _cursorPosition).normalized());
    }
    else if(_leftMouseButtonHeld && _frustumSelecting)
    {
        // Shift key has been released
        _frustumSelectStart = QPoint();
        _frustumSelecting = false;
        _scene->clearSelectionRect();

        emit userInteractionFinished();
    }
    else if(!_nearClickNodeId.isNull() && _rightMouseButtonHeld)
    {
        _selecting = false;

        if(!_mouseMoving)
            emit userInteractionStarted();

        const QVector3D clickedNodePosition = _graphModel->nodePositions().getScaledAndSmoothed(_nearClickNodeId);

        Plane translationPlane(clickedNodePosition, camera->viewVector().normalized());

        QVector3D prevPoint = translationPlane.rayIntersection(
                    camera->rayForViewportCoordinates(_prevCursorPosition.x(), _prevCursorPosition.y()));
        QVector3D curPoint = translationPlane.rayIntersection(
                    camera->rayForViewportCoordinates(_cursorPosition.x(), _cursorPosition.y()));
        QVector3D translation = prevPoint - curPoint;

        camera->translateWorld(translation);
    }
    else if(_leftMouseButtonHeld)
    {
        _selecting = false;

        if(!_mouseMoving)
            emit userInteractionStarted();

        QQuaternion rotation = mouseMoveToRotation();
        camera->rotateAboutViewTarget(rotation);
    }

    _prevCursorPosition = _cursorPosition;

    if(_rightMouseButtonHeld || _leftMouseButtonHeld)
        _mouseMoving = true;
}

void GraphComponentInteractor::mouseDoubleClickEvent(QMouseEvent* mouseEvent)
{
    if(mouseEvent->button() == Qt::LeftButton)
    {
        if(!_mouseMoving)
        {
            if(!_nearClickNodeId.isNull())
            {
                if(_nearClickNodeId != _scene->focusNodeId())
                    _scene->moveFocusToNode(_nearClickNodeId, Transition::Type::EaseInEaseOut);
            }
            else if(!_scene->trackingCentreOfMass())
                _scene->moveFocusToCentreOfMass(Transition::Type::EaseInEaseOut);
        }
    }
}

void GraphComponentInteractor::keyPressEvent(QKeyEvent* keyEvent)
{
    switch(keyEvent->key())
    {
    case Qt::Key_Left:
        _scene->moveToNextComponent();
        break;

    case Qt::Key_Right:
        _scene->moveToPreviousComponent();
        break;

    default:
        break;
    }
}

void GraphComponentInteractor::keyReleaseEvent(QKeyEvent* /*keyEvent*/)
{
    /*switch(keyEvent->key())
    {
    default:
        break;
    }*/
}

void GraphComponentInteractor::wheelEvent(QWheelEvent* wheelEvent)
{
    if(wheelEvent->angleDelta().y() > 0.0f)
        _scene->zoom(1.0f);
    else
        _scene->zoom(-1.0f);
}
