#include "graphview.h"
#include "selectionmanager.h"

#include "../gl/graphscene.h"
#include "../gl/openglwindow.h"

#include "../graph/graph.h"
#include "../graph/graphmodel.h"

#include "../maths/frustum.h"
#include "../maths/plane.h"
#include "../maths/boundingsphere.h"

#include "../layout/collision.h"

#include "../utils.h"

#include <QVBoxLayout>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QtMath>
#include <cmath>

GraphView::GraphView(QWidget *parent) :
    QWidget(parent),
    _rightMouseButtonHeld(false),
    _leftMouseButtonHeld(false),
    _selecting(false),
    _frustumSelecting(false),
    _mouseMoving(false)
{
    OpenGLWindow* window = new OpenGLWindow(surfaceFormat(), this);

    _graphScene = new GraphScene;
    window->setScene(_graphScene);

    connect(_graphScene, &GraphScene::userInteractionStarted, this, &GraphView::userInteractionStarted);
    connect(_graphScene, &GraphScene::userInteractionFinished, this, &GraphView::userInteractionFinished);

    this->setLayout(new QVBoxLayout());
    this->layout()->addWidget(QWidget::createWindowContainer(window));
}

void GraphView::setSelectionManager(SelectionManager* selectionManager)
{
    this->_selectionManager = selectionManager;
    connect(selectionManager, &SelectionManager::selectionChanged, _graphScene, &GraphScene::onSelectionChanged);
}

void GraphView::layoutChanged()
{
}

void GraphView::mousePressEvent(QMouseEvent* mouseEvent)
{
    switch(mouseEvent->button())
    {
    case Qt::LeftButton:
        _leftMouseButtonHeld = true;
        _selecting = true;
        break;

    case Qt::RightButton:
        _rightMouseButtonHeld = true;
        _graphScene->disableFocusNodeTracking();
        break;

    default: break;
    }

    _cursorPosition = _prevCursorPosition = mouseEvent->pos();

    Ray ray = _graphScene->camera()->rayForViewportCoordinates(_cursorPosition.x(), _cursorPosition.y());

    const ReadOnlyGraph& component = *_graphModel->graph().componentById(_graphScene->focusComponentId());

    Collision collision(component, _graphModel->nodeVisuals(), _graphModel->nodePositions());
    _clickedNodeId = collision.nearestNodeIntersectingLine(ray.origin(), ray.dir());

    if(mouseEvent->modifiers() & Qt::ShiftModifier)
        _frustumSelectStart = _cursorPosition;
}

void GraphView::mouseReleaseEvent(QMouseEvent* mouseEvent)
{
    switch(mouseEvent->button())
    {
    case Qt::RightButton:
        if(!_graphScene->interactionAllowed())
        {
            _rightMouseButtonHeld = false;
            _graphScene->enableFocusNodeTracking();
            return;
        }

        emit userInteractionFinished();

        if(_rightMouseButtonHeld && _mouseMoving)
            _graphScene->selectFocusNodeClosestToCameraVector();

        _rightMouseButtonHeld = false;
        _graphScene->enableFocusNodeTracking();
        _clickedNodeId.setToNull();
        break;

    case Qt::LeftButton:
        _leftMouseButtonHeld = false;

        if(!_graphScene->interactionAllowed())
            return;

        emit userInteractionFinished();

        if(_selecting)
        {
            if(_frustumSelecting)
            {
                if(!(mouseEvent->modifiers() & Qt::ShiftModifier))
                    _selectionManager->clearNodeSelection();

                QPoint frustumEndPoint = mouseEvent->pos();
                Frustum frustum = _graphScene->camera()->frustumForViewportCoordinates(
                            _frustumSelectStart.x(), _frustumSelectStart.y(),
                            frustumEndPoint.x(), frustumEndPoint.y());

                QSet<NodeId> selection;

                const ReadOnlyGraph& component = *_graphModel->graph().componentById(_graphScene->focusComponentId());
                for(NodeId nodeId : component.nodeIds())
                {
                    const QVector3D& nodePosition = _graphModel->nodePositions()[nodeId];
                    if(frustum.containsPoint(nodePosition))
                        selection.insert(nodeId);
                }

                _selectionManager->selectNodes(selection);

                _frustumSelecting = false;
                _graphScene->clearSelectionRect();
            }
            else
            {
                if(!_clickedNodeId.isNull())
                {
                    if(!(mouseEvent->modifiers() & Qt::ShiftModifier))
                        _selectionManager->clearNodeSelection();

                    _selectionManager->toggleNode(_clickedNodeId);
                }
                else
                    _selectionManager->clearNodeSelection();
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

void GraphView::mouseMoveEvent(QMouseEvent* mouseEvent)
{
    if(!_graphScene->interactionAllowed())
        return;

    Camera* camera = _graphScene->camera();
    _cursorPosition = mouseEvent->pos();

    if((mouseEvent->modifiers() & Qt::ShiftModifier) && _leftMouseButtonHeld)
    {
        emit userInteractionStarted();
        _selecting = true;
        _frustumSelecting = true;
        _clickedNodeId.setToNull();
        _graphScene->setSelectionRect(QRect(_frustumSelectStart, _cursorPosition).normalized());
    }
    else if(!_clickedNodeId.isNull())
    {
        _selecting = false;

        if(_rightMouseButtonHeld)
        {
            emit userInteractionStarted();

            const QVector3D& clickedNodePosition = _graphModel->nodePositions()[_clickedNodeId];

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

            if(_graphScene->focusNodeId().isNull())
                _graphScene->selectFocusNodeClosestToCameraVector();

            if(_graphScene->focusNodeId() != _clickedNodeId)
            {
                emit userInteractionStarted();

                const QVector3D& clickedNodePosition = _graphModel->nodePositions()[_clickedNodeId];
                const QVector3D& rotationCentre = _graphModel->nodePositions()[_graphScene->focusNodeId()];
                float radius = clickedNodePosition.distanceToPoint(rotationCentre);

                BoundingSphere boundingSphere(rotationCentre, radius);
                QVector3D cursorPoint;
                Ray cursorRay = camera->rayForViewportCoordinates(_cursorPosition.x(), _cursorPosition.y());

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

void GraphView::mouseDoubleClickEvent(QMouseEvent* mouseEvent)
{
    if(mouseEvent->button() == Qt::LeftButton)
    {
        if(!_clickedNodeId.isNull() && !_mouseMoving)
            _graphScene->moveFocusToNode(_clickedNodeId, Transition::Type::EaseInEaseOut);
    }
}

void GraphView::keyPressEvent(QKeyEvent* keyEvent)
{
    switch(keyEvent->key())
    {
    case Qt::Key_Delete:
        _graphModel->graph().removeNodes(_selectionManager->selectedNodes());
        _selectionManager->clearNodeSelection();
        break;

    case Qt::Key_Left:
        _graphScene->moveToNextComponent();
        break;

    case Qt::Key_Right:
        _graphScene->moveToPreviousComponent();
        break;

    default:
        break;
    }
}

void GraphView::keyReleaseEvent(QKeyEvent* keyEvent)
{
    switch(keyEvent->key())
    {
    default:
        break;
    }
}

void GraphView::wheelEvent(QWheelEvent* wheelEvent)
{
    if(wheelEvent->angleDelta().y() > 0.0f)
        _graphScene->zoom(1.0f);
    else
        _graphScene->zoom(-1.0f);
}
