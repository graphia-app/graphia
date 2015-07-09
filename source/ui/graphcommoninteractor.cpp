#include "graphcommoninteractor.h"
#include "selectionmanager.h"
#include "graphquickitem.h"

#include "../commands/commandmanager.h"

#include "../rendering/graphrenderer.h"
#include "../rendering/graphcomponentscene.h"
#include "../rendering/graphcomponentrenderer.h"
#include "../rendering/camera.h"

#include "../graph/graph.h"
#include "../graph/graphmodel.h"

#include "../maths/frustum.h"

#include "../layout/collision.h"

#include "../utils/utils.h"

#include <QMouseEvent>
#include <QKeyEvent>
#include <QtMath>
#include <cmath>

ElementIdSet<NodeId> nodeIdsInsideFrustum(const GraphModel& graphModel,
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

GraphCommonInteractor::GraphCommonInteractor(std::shared_ptr<GraphModel> graphModel,
                                             CommandManager& commandManager,
                                             std::shared_ptr<SelectionManager> selectionManager,
                                             GraphRenderer* graphRenderer) :
    Interactor(graphRenderer),
    _graphModel(graphModel),
    _commandManager(commandManager),
    _selectionManager(selectionManager),
    _graphRenderer(graphRenderer),
    _rightMouseButtonHeld(false),
    _leftMouseButtonHeld(false),
    _selecting(false),
    _frustumSelecting(false),
    _mouseMoving(false),
    _clickedRenderer(nullptr),
    _rendererUnderCursor(nullptr)
{
    connect(this, &Interactor::userInteractionStarted, graphRenderer, &GraphRenderer::userInteractionStarted);
    connect(this, &Interactor::userInteractionFinished, graphRenderer, &GraphRenderer::userInteractionFinished);
}

void GraphCommonInteractor::mousePressEvent(QMouseEvent* mouseEvent)
{
    _clickedRenderer = rendererAtPosition(mouseEvent->pos());
    _clickPosition = mouseEvent->pos();
    _modifiers = mouseEvent->modifiers();

    _cursorPosition = _prevCursorPosition = _clickPosition;

    if(clickedRenderer() != nullptr)
    {
        Ray ray = clickedRenderer()->camera()->rayForViewportCoordinates(localCursorPosition().x(),
                                                                         localCursorPosition().y());

        Collision collision(*_graphModel, clickedRenderer()->componentId());
        _clickedNodeId = collision.nearestNodeIntersectingLine(ray.origin(), ray.dir());

        if(_clickedNodeId.isNull())
        {
            const int PICK_RADIUS = 40;
            ConicalFrustum frustum = clickedRenderer()->camera()->conicalFrustumForViewportCoordinates(
                        localCursorPosition().x(), localCursorPosition().y(), PICK_RADIUS);

            _nearClickNodeId = nodeIdInsideFrustumNearestPoint(*_graphModel, clickedRenderer()->componentId(),
                                                               frustum, ray.origin());
        }
        else
            _nearClickNodeId = _clickedNodeId;
    }

    switch(mouseEvent->button())
    {
    case Qt::LeftButton:
        _leftMouseButtonHeld = true;
        leftMouseDown();
        break;

    case Qt::RightButton:
        _rightMouseButtonHeld = true;
        rightMouseDown();
        break;

    default: break;
    }
}

void GraphCommonInteractor::mouseReleaseEvent(QMouseEvent* mouseEvent)
{
    _cursorPosition = mouseEvent->pos();
    _modifiers = mouseEvent->modifiers();

    switch(mouseEvent->button())
    {
    case Qt::LeftButton:
        _leftMouseButtonHeld = false;
        leftMouseUp();
        break;

    case Qt::RightButton:
        _rightMouseButtonHeld = false;
        rightMouseUp();
        break;

    default: break;
    }

    if(!_rightMouseButtonHeld && !_leftMouseButtonHeld)
    {
        _clickedNodeId.setToNull();
        _nearClickNodeId.setToNull();
        _clickedRenderer = nullptr;
        _mouseMoving = false;
    }
}

void GraphCommonInteractor::mouseMoveEvent(QMouseEvent* mouseEvent)
{
    _rendererUnderCursor = rendererAtPosition(mouseEvent->pos());

    if(_graphRenderer->transition().active())
        return;

    _cursorPosition = mouseEvent->pos();
    _modifiers = mouseEvent->modifiers();

    if(!_mouseMoving)
    {
        const int MIN_MANHATTAN_MOVE = 3;
        QPoint p = _cursorPosition - _clickPosition;
        if(p.manhattanLength() <= MIN_MANHATTAN_MOVE)
            return;
    }

    if(_leftMouseButtonHeld)
        leftDrag();
    else if(_rightMouseButtonHeld)
        rightDrag();

    _prevCursorPosition = _cursorPosition;

    if(_leftMouseButtonHeld || _rightMouseButtonHeld)
        _mouseMoving = true;
}

void GraphCommonInteractor::mouseDoubleClickEvent(QMouseEvent* mouseEvent)
{
    if(_mouseMoving)
        return;

    _modifiers = mouseEvent->modifiers();

    switch(mouseEvent->button())
    {
    case Qt::LeftButton:
        leftDoubleClick();
        break;

    case Qt::RightButton:
        rightDoubleClick();
        break;

    default: break;
    }
}

void GraphCommonInteractor::leftMouseDown()
{
    _selecting = true;

    if(modifiers() & Qt::ShiftModifier)
        _frustumSelectStart = cursorPosition();
    else
        _frustumSelectStart = QPoint();
}

void GraphCommonInteractor::leftMouseUp()
{
    if(_graphRenderer->transition().active())
        return;

    emit userInteractionFinished();

    if(_selecting)
    {
        if(_frustumSelecting)
        {
            QPoint frustumSelectEnd = cursorPosition();
            auto selection = selectionForRect(QRect(_frustumSelectStart, frustumSelectEnd));

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
            _graphRenderer->clearSelectionRect();
        }
        else
        {
            if(!_clickedNodeId.isNull())
            {
                bool nodeSelected = _selectionManager->nodeIsSelected(_clickedNodeId);
                bool toggling = modifiers() & Qt::ShiftModifier;
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

static QQuaternion mouseMoveToRotation(const QPoint& prev, const QPoint& cur,
                                       const GraphComponentRenderer* renderer)
{
    int w = renderer->width();
    int h = renderer->height();

    QVector3D previous = virtualTrackballVector(w, h, prev);
    QVector3D current = virtualTrackballVector(w, h, cur);

    QVector3D axis = QVector3D::crossProduct(previous, current).normalized();

    float dot = QVector3D::dotProduct(previous, current);
    float value = dot / (previous.length() * current.length());
    value = Utils::clamp(-1.0f, 1.0f, value);
    float radians = std::acos(value);
    float angle = -qRadiansToDegrees(radians);

    auto m = renderer->camera()->viewMatrix();
    m.setColumn(3, QVector4D(0.0f, 0.0f, 0.0f, 1.0f));

    QQuaternion rotation = QQuaternion::fromAxisAndAngle(axis * m, angle);

    return rotation;
}

void GraphCommonInteractor::leftDrag()
{
    if(modifiers() & Qt::ShiftModifier)
    {
        if(!_frustumSelecting)
        {
            emit userInteractionStarted();
            _frustumSelecting = true;
            _clickedNodeId.setToNull();
            _nearClickNodeId.setToNull();

            // This can happen if the user holds shift after clicking
            if(_frustumSelectStart.isNull())
                _frustumSelectStart = cursorPosition();
        }

        _graphRenderer->setSelectionRect(QRect(_frustumSelectStart, cursorPosition()).normalized());
    }
    else if(_frustumSelecting)
    {
        // Shift key has been released
        _frustumSelectStart = QPoint();
        _frustumSelecting = false;
        _graphRenderer->clearSelectionRect();

        emit userInteractionFinished();
    }
    else if(clickedRenderer() != nullptr)
    {
        _selecting = false;

        if(!_mouseMoving)
            emit userInteractionStarted();

        Camera* camera = clickedRenderer()->camera();
        QQuaternion rotation = mouseMoveToRotation(localPrevCursorPosition(),
                                                   localCursorPosition(),
                                                   clickedRenderer());
        camera->rotate(rotation);
    }
}

QPoint GraphCommonInteractor::cursorPosition()
{
    return _cursorPosition;
}

QPoint GraphCommonInteractor::prevCursorPosition()
{
    return _prevCursorPosition;
}

QPoint GraphCommonInteractor::localCursorPosition()
{
    Q_ASSERT(clickedRenderer() != nullptr);
    return componentLocalCursorPosition(clickedRenderer()->componentId(), _cursorPosition);
}

QPoint GraphCommonInteractor::localPrevCursorPosition()
{
    Q_ASSERT(clickedRenderer() != nullptr);
    return componentLocalCursorPosition(clickedRenderer()->componentId(), _prevCursorPosition);
}

Qt::KeyboardModifiers GraphCommonInteractor::modifiers()
{
    return _modifiers;
}

void GraphCommonInteractor::wheelEvent(QWheelEvent* wheelEvent)
{
    _rendererUnderCursor = rendererAtPosition(wheelEvent->pos());

    if(wheelEvent->angleDelta().y() > 0.0f)
        wheelUp();
    else
        wheelDown();
}
