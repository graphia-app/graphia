#include "graphcomponentinteractor.h"
#include "selectionmanager.h"
#include "commandmanager.h"

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

        if(!_clickedNodeId.isNull())
            _scene->disableFocusNodeTracking();
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
            _scene->enableFocusNodeTracking();
            return;
        }

        emit userInteractionFinished();

        if(!_clickedNodeId.isNull() && _rightMouseButtonHeld && _mouseMoving)
            _scene->selectFocusNodeClosestToCameraVector();

        _rightMouseButtonHeld = false;
        _scene->enableFocusNodeTracking();
        _clickedNodeId.setToNull();
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
                QPoint frustumEndPoint = mouseEvent->pos();
                Frustum frustum = _scene->camera()->frustumForViewportCoordinates(
                            _frustumSelectStart.x(), _frustumSelectStart.y(),
                            frustumEndPoint.x(), frustumEndPoint.y());

                ElementIdSet<NodeId> selection;

                auto component = _graphModel->graph().componentById(_scene->focusComponentId());
                for(NodeId nodeId : component->nodeIds())
                {
                    const QVector3D& nodePosition = _graphModel->nodePositions().at(nodeId);
                    if(frustum.containsPoint(nodePosition))
                        selection.insert(nodeId);
                }

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
        break;

    default: break;
    }

    if(_mouseMoving && !_rightMouseButtonHeld && !_leftMouseButtonHeld)
        _mouseMoving = false;
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
    else if(!_clickedNodeId.isNull())
    {
        _selecting = false;

        if(_rightMouseButtonHeld)
        {
            emit userInteractionStarted();

            const QVector3D& clickedNodePosition = _graphModel->nodePositions().at(_clickedNodeId);

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
            emit userInteractionStarted();

            if(_scene->focusNodeId().isNull())
                _scene->selectFocusNodeClosestToCameraVector();

            if(_scene->focusNodeId() != _clickedNodeId)
            {
                emit userInteractionStarted();

                const QVector3D& clickedNodePosition = _graphModel->nodePositions().at(_clickedNodeId);
                const QVector3D& rotationCentre = _graphModel->nodePositions().at(_scene->focusNodeId());
                float radius = clickedNodePosition.distanceToPoint(rotationCentre);

                BoundingSphere boundingSphere(rotationCentre, radius);
                QVector3D cursorPoint;
                Ray cursorRay = camera->rayForViewportCoordinates(_cursorPosition.x(), _cursorPosition.y());

                Plane divisionPlane(rotationCentre, cursorRay.dir().normalized());

                std::vector<QVector3D> intersections = boundingSphere.rayIntersection(cursorRay);
                if(intersections.size() > 0)
                {
                    if(divisionPlane.sideForPoint(clickedNodePosition) == Plane::Side::Front && intersections.size() > 1)
                        cursorPoint = intersections[1];
                    else
                        cursorPoint = intersections[0];
                }
                else
                {
                    // When the ray misses the node completely we clamp the cursor point on the surface of the sphere
                    QVector3D cameraToCentre = rotationCentre - camera->position();
                    float cameraToCentreLengthSq = cameraToCentre.lengthSquared();
                    float radiusSq = radius * radius;
                    float cameraToClickedLengthSq = cameraToCentreLengthSq - radiusSq;

                    // Form a right angled triangle from the camera, the circle tangent and a point which lies on the
                    // camera to rotation centre vector
                    float adjacentLength = (cameraToClickedLengthSq - radiusSq + cameraToCentreLengthSq) /
                            (2.0f * cameraToCentre.length());
                    float oppositeLength = sqrt(cameraToClickedLengthSq - (adjacentLength * adjacentLength));
                    QVector3D corner = camera->position() + ((adjacentLength / cameraToCentre.length()) * cameraToCentre);

                    Plane p(camera->position(), rotationCentre, cursorRay.origin());
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
                camera->rotateAboutViewTarget(rotation);
            }
        }
    }

    _prevCursorPosition = _cursorPosition;

    if(_rightMouseButtonHeld || _leftMouseButtonHeld)
        _mouseMoving = true;
}

void GraphComponentInteractor::mouseDoubleClickEvent(QMouseEvent* mouseEvent)
{
    if(mouseEvent->button() == Qt::LeftButton)
    {
        if(!_clickedNodeId.isNull() && !_mouseMoving)
            _scene->moveFocusToNode(_clickedNodeId, Transition::Type::EaseInEaseOut);
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
