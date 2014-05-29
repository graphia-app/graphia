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
    m_rightMouseButtonHeld(false),
    m_leftMouseButtonHeld(false),
    m_selecting(false),
    m_frustumSelecting(false),
    m_mouseMoving(false)
{
    OpenGLWindow* window = new OpenGLWindow(surfaceFormat(), this);

    graphScene = new GraphScene;
    window->setScene(graphScene);

    connect(graphScene, &GraphScene::userInteractionStarted, this, &GraphView::userInteractionStarted);
    connect(graphScene, &GraphScene::userInteractionFinished, this, &GraphView::userInteractionFinished);

    this->setLayout(new QVBoxLayout());
    this->layout()->addWidget(QWidget::createWindowContainer(window));
}

void GraphView::setSelectionManager(SelectionManager* selectionManager)
{
    this->_selectionManager = selectionManager;
    connect(selectionManager, &SelectionManager::selectionChanged, graphScene, &GraphScene::onSelectionChanged);
}

void GraphView::layoutChanged()
{
}

void GraphView::mousePressEvent(QMouseEvent* mouseEvent)
{
    switch(mouseEvent->button())
    {
    case Qt::LeftButton:
        m_leftMouseButtonHeld = true;
        m_selecting = true;
        break;

    case Qt::RightButton:
        m_rightMouseButtonHeld = true;
        graphScene->disableFocusNodeTracking();
        break;

    default: break;
    }

    m_pos = m_prevPos = mouseEvent->pos();

    Ray ray = graphScene->camera()->rayForViewportCoordinates(m_pos.x(), m_pos.y());

    const ReadOnlyGraph& component = *_graphModel->graph().componentById(graphScene->focusComponentId());

    Collision collision(component, _graphModel->nodeVisuals(), _graphModel->nodePositions());
    clickedNodeId = collision.nearestNodeIntersectingLine(ray.origin(), ray.dir());

    if(mouseEvent->modifiers() & Qt::ShiftModifier)
        m_frustumSelectStart = m_pos;
}

void GraphView::mouseReleaseEvent(QMouseEvent* mouseEvent)
{
    switch(mouseEvent->button())
    {
    case Qt::RightButton:
        if(!graphScene->interactionAllowed())
        {
            m_rightMouseButtonHeld = false;
            graphScene->enableFocusNodeTracking();
            return;
        }

        emit userInteractionFinished();

        if(m_rightMouseButtonHeld && m_mouseMoving)
            graphScene->selectFocusNodeClosestToCameraVector();

        m_rightMouseButtonHeld = false;
        graphScene->enableFocusNodeTracking();
        clickedNodeId.setToNull();
        break;

    case Qt::LeftButton:
        m_leftMouseButtonHeld = false;

        if(!graphScene->interactionAllowed())
            return;

        emit userInteractionFinished();

        if(m_selecting)
        {
            if(m_frustumSelecting)
            {
                if(!(mouseEvent->modifiers() & Qt::ShiftModifier))
                    _selectionManager->clearNodeSelection();

                QPoint frustumEndPoint = mouseEvent->pos();
                Frustum frustum = graphScene->camera()->frustumForViewportCoordinates(
                            m_frustumSelectStart.x(), m_frustumSelectStart.y(),
                            frustumEndPoint.x(), frustumEndPoint.y());

                QSet<NodeId> selection;

                const ReadOnlyGraph& component = *_graphModel->graph().componentById(graphScene->focusComponentId());
                for(NodeId nodeId : component.nodeIds())
                {
                    const QVector3D& nodePosition = _graphModel->nodePositions()[nodeId];
                    if(frustum.containsPoint(nodePosition))
                        selection.insert(nodeId);
                }

                _selectionManager->selectNodes(selection);

                m_frustumSelecting = false;
                graphScene->clearSelectionRect();
            }
            else
            {
                if(!clickedNodeId.isNull())
                {
                    if(!(mouseEvent->modifiers() & Qt::ShiftModifier))
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

void GraphView::mouseMoveEvent(QMouseEvent* mouseEvent)
{
    if(!graphScene->interactionAllowed())
        return;

    Camera* camera = graphScene->camera();
    m_pos = mouseEvent->pos();

    if((mouseEvent->modifiers() & Qt::ShiftModifier) && m_leftMouseButtonHeld)
    {
        emit userInteractionStarted();
        m_selecting = true;
        m_frustumSelecting = true;
        clickedNodeId.setToNull();
        graphScene->setSelectionRect(QRect(m_frustumSelectStart, m_pos).normalized());
    }
    else if(!clickedNodeId.isNull())
    {
        m_selecting = false;

        if(m_rightMouseButtonHeld)
        {
            emit userInteractionStarted();

            const QVector3D& clickedNodePosition = _graphModel->nodePositions()[clickedNodeId];

            Plane translationPlane(clickedNodePosition, camera->viewVector().normalized());

            QVector3D prevPoint = translationPlane.rayIntersection(
                        camera->rayForViewportCoordinates(m_prevPos.x(), m_prevPos.y()));
            QVector3D curPoint = translationPlane.rayIntersection(
                        camera->rayForViewportCoordinates(m_pos.x(), m_pos.y()));
            QVector3D translation = prevPoint - curPoint;

            camera->translateWorld(translation);
        }
        else if(m_leftMouseButtonHeld)
        {
            emit userInteractionStarted();

            if(graphScene->focusNodeId().isNull())
                graphScene->selectFocusNodeClosestToCameraVector();

            if(graphScene->focusNodeId() != clickedNodeId)
            {
                emit userInteractionStarted();

                const QVector3D& clickedNodePosition = _graphModel->nodePositions()[clickedNodeId];
                const QVector3D& rotationCentre = _graphModel->nodePositions()[graphScene->focusNodeId()];
                float radius = clickedNodePosition.distanceToPoint(rotationCentre);

                BoundingSphere boundingSphere(rotationCentre, radius);
                QVector3D cursorPoint;
                Ray cursorRay = camera->rayForViewportCoordinates(m_pos.x(), m_pos.y());

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

    m_prevPos = m_pos;

    if(m_rightMouseButtonHeld || m_leftMouseButtonHeld)
        m_mouseMoving = true;
}

void GraphView::mouseDoubleClickEvent(QMouseEvent* mouseEvent)
{
    if(mouseEvent->button() == Qt::LeftButton)
    {
        if(!clickedNodeId.isNull() && !m_mouseMoving)
            graphScene->moveFocusToNode(clickedNodeId, Transition::Type::EaseInEaseOut);
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
        graphScene->moveToNextComponent();
        break;

    case Qt::Key_Right:
        graphScene->moveToPreviousComponent();
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
        graphScene->zoom(1.0f);
    else
        graphScene->zoom(-1.0f);
}
