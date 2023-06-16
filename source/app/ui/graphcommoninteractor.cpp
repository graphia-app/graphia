/* Copyright © 2013-2023 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "graphcommoninteractor.h"
#include "graphquickitem.h"
#include "selectionmanager.h"

#include "commands/commandmanager.h"
#include "commands/selectnodescommand.h"

#include "rendering/graphrenderer.h"
#include "rendering/graphcomponentscene.h"
#include "rendering/camera.h"
#include "rendering/graphcomponentrenderer.h"

#include "shared/utils/utils.h"

#include "graph/graph.h"
#include "graph/graphmodel.h"

#include "maths/frustum.h"

#include "layout/collision.h"

#include "ui/visualisations/elementvisual.h"

#include <QMouseEvent>
#include <QKeyEvent>
#include <QtMath>

#include <cmath>
#include <algorithm>

NodeIdSet nodeIdsInsideFrustum(const GraphModel& graphModel,
                               ComponentId componentId,
                               const BaseFrustum& frustum)
{
    NodeIdSet selection;

    const auto* component = graphModel.graph().componentById(componentId);
    Q_ASSERT(component != nullptr);

    for(const NodeId nodeId : component->nodeIds())
    {
        if(graphModel.nodeVisual(nodeId).state().test(VisualFlags::Unhighlighted))
            continue;

        const QVector3D nodePosition = graphModel.nodePositions().get(nodeId);
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
        if(graphModel.nodeVisual(nodeId).state().test(VisualFlags::Unhighlighted))
            continue;

        const float distanceToCentre = Ray(frustum.centreLine()).distanceTo(point);
        const float distanceToPoint = graphModel.nodePositions().get(nodeId).distanceToPoint(point);
        const float distance = distanceToCentre + distanceToPoint;

        if(distance < minimumDistance)
        {
            minimumDistance = distance;
            closestNodeId = nodeId;
        }
    }

    return closestNodeId;
}

GraphCommonInteractor::GraphCommonInteractor(GraphModel* graphModel,
                                             CommandManager* commandManager,
                                             SelectionManager* selectionManager,
                                             GraphRenderer* graphRenderer) :
    Interactor(graphRenderer),
    _graphModel(graphModel),
    _commandManager(commandManager),
    _selectionManager(selectionManager),
    _graphRenderer(graphRenderer)
{
    connect(this, &Interactor::userInteractionStarted, graphRenderer, &GraphRenderer::userInteractionStarted);
    connect(this, &Interactor::userInteractionFinished, graphRenderer, &GraphRenderer::userInteractionFinished);

    connect(this, &Interactor::clicked, graphRenderer, &GraphRenderer::clicked);
}

void GraphCommonInteractor::mouseDown(const QPoint& position)
{
    _clickedComponentRenderer = componentRendererAtPosition(position);
    _clickPosition = position;

    _cursorPosition = _prevCursorPosition = _clickPosition;

    if(clickedComponentRenderer() != nullptr)
    {
        _clickedNodeId = nodeIdAtPosition(localCursorPosition());

        if(_clickedNodeId.isNull())
            _nearClickNodeId = nodeIdNearPosition(localCursorPosition());
        else
            _nearClickNodeId = _clickedNodeId;
    }
}

void GraphCommonInteractor::mouseUp()
{
    _clickedNodeId.setToNull();
    _nearClickNodeId.setToNull();
    _clickedComponentRenderer = nullptr;
    _mouseMoving = false;
}

void GraphCommonInteractor::mousePressEvent(const QPoint& pos, Qt::KeyboardModifiers modifiers, Qt::MouseButton button)
{
    mouseDown(pos);
    _modifiers = modifiers;

    switch(button)
    {
    case Qt::LeftButton:
        _leftMouseButtonHeld = true;
        leftMouseDown();
        break;

    case Qt::RightButton:
        _rightMouseButtonHeld = true;
        rightMouseDown();
        break;

    case Qt::MiddleButton:
        _middleMouseButtonHeld = true;
        middleMouseDown();
        break;

    default: break;
    }
}

void GraphCommonInteractor::mouseReleaseEvent(const QPoint& pos, Qt::KeyboardModifiers modifiers, Qt::MouseButton button)
{
    _cursorPosition = pos;
    _modifiers = modifiers;

    switch(button)
    {
    case Qt::LeftButton:
        _leftMouseButtonHeld = false;
        leftMouseUp();
        break;

    case Qt::RightButton:
        _rightMouseButtonHeld = false;
        rightMouseUp();
        break;

    case Qt::MiddleButton:
        _middleMouseButtonHeld = false;
        middleMouseUp();
        break;


    default: break;
    }

    if(!_leftMouseButtonHeld && !_rightMouseButtonHeld && !_middleMouseButtonHeld)
        mouseUp();
}

void GraphCommonInteractor::mouseMoveEvent(const QPoint& pos, Qt::KeyboardModifiers modifiers, Qt::MouseButton)
{
    if(_graphRenderer->transition().active())
        return;

    _componentRendererUnderCursor = componentRendererAtPosition(pos);
    _cursorPosition = pos;
    _modifiers = modifiers;

    if(!_mouseMoving)
    {
        const int MIN_MANHATTAN_MOVE = 3;
        const QPoint p = _cursorPosition - _clickPosition;
        if(p.manhattanLength() <= MIN_MANHATTAN_MOVE)
            return;
    }

    if(_leftMouseButtonHeld)
        leftDrag();
    else if(_rightMouseButtonHeld)
        rightDrag();
    else if(_middleMouseButtonHeld)
        middleDrag();

    _prevCursorPosition = _cursorPosition;

    if(_leftMouseButtonHeld || _rightMouseButtonHeld || _middleMouseButtonHeld)
        _mouseMoving = true;
}

void GraphCommonInteractor::mouseDoubleClickEvent(const QPoint&, Qt::KeyboardModifiers modifiers, Qt::MouseButton button)
{
    if(_mouseMoving)
        return;

    _modifiers = modifiers;

    switch(button)
    {
    case Qt::LeftButton:
        leftDoubleClick();
        break;

    case Qt::RightButton:
        rightDoubleClick();
        break;

    case Qt::MiddleButton:
        middleDoubleClick();
        break;

    default: break;
    }
}


// https://www.opengl.org/wiki/Object_Mouse_Trackball
static QVector3D virtualTrackballVector(int width, int height, const QPoint& cursor)
{
    const auto minDimension = static_cast<float>(std::min(width, height));
    const float x = static_cast<float>(2 * cursor.x() - width) / minDimension;
    const float y = static_cast<float>(height - 2 * cursor.y()) / minDimension;
    const float d = std::sqrt(x * x + y * y);
    const float RADIUS = 0.9f; // Radius of trackball
    const float ROOT2 = std::sqrt(2.0f);
    const float CUTOFF = ROOT2 / 2.0f;

    float z = 0.0f;

    if(d < (RADIUS * CUTOFF))
    {
        // Sphere
        z = std::sqrt(RADIUS * RADIUS - d * d);
    }
    else
    {
        // Hyperbolic sheet
        const float t = RADIUS / ROOT2;
        z = t * t / d;
    }

    return {x, y, z};
}

static QQuaternion mouseMoveToRotation(const QPoint& prev, const QPoint& cur,
                                       const GraphComponentRenderer* renderer)
{
    const int w = renderer->width();
    const int h = renderer->height();

    QQuaternion rotation;

    if(renderer->projection() == Projection::TwoDee)
    {
        const QPoint centre(w / 2, h / 2);
        const QVector2D previous(prev - centre);
        const QVector2D current(cur - centre);

        auto distanceFromCentre = current.length();

        float radians = std::atan2(previous.y(), previous.x()) -
            std::atan2(current.y(), current.x());
        radians = u::normaliseAngle(radians);
        float angle = qRadiansToDegrees(radians);

        // Close to the centre of rotation, taper down to a deadzone, thereby
        // avoiding violent spins due to the vectors being very short
        const auto wf = static_cast<float>(w);
        const float deadzoneScale = wf < 200.0f ? wf / 200.0f : 1.0f;
        const float deadzoneDistance = 25.0f * deadzoneScale;
        const float dampingDistance = deadzoneDistance * 2.0f;
        if(distanceFromCentre < dampingDistance)
        {
            auto dampingFactor = distanceFromCentre >= deadzoneDistance ?
                ((distanceFromCentre - deadzoneDistance) /
                (dampingDistance - deadzoneDistance)) : 0.0f;

            angle *= dampingFactor;
        }

        const float currentAngle = renderer->camera()->rotation().toEulerAngles().z();

        rotation = QQuaternion::fromAxisAndAngle({0.0f, 0.0f, 1.0f}, currentAngle - angle);
    }
    else
    {
        const QVector3D previous = virtualTrackballVector(w, h, prev);
        const QVector3D current = virtualTrackballVector(w, h, cur);

        const QVector3D axis = QVector3D::crossProduct(current, previous).normalized();

        const float dot = QVector3D::dotProduct(previous, current);
        float value = dot / (previous.length() * current.length());
        value = std::clamp(value, -1.0f, 1.0f);
        const float radians = std::acos(value);
        const float angle = qRadiansToDegrees(radians);

        auto m = renderer->camera()->viewMatrix();
        m.setColumn(3, QVector4D(0.0f, 0.0f, 0.0f, 1.0f));

        rotation = QQuaternion::fromAxisAndAngle(axis * m, angle) *
            renderer->camera()->rotation();
    }

    return rotation;
}

void GraphCommonInteractor::rotateRendererByMouseMove(GraphComponentRenderer* renderer,
    const QPoint& from, const QPoint& to)
{
    auto* camera = renderer->camera();
    auto rotation = mouseMoveToRotation(from, to, renderer);
    camera->setRotation(rotation);
}

void GraphCommonInteractor::leftMouseDown()
{
    _selecting = true;

    if((modifiers() & Qt::ShiftModifier) != 0)
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
            const QPoint frustumSelectEnd = cursorPosition();
            auto selection = selectionForRect(QRect(_frustumSelectStart, frustumSelectEnd));

            if(!selection.empty())
            {
                _commandManager->execute(ExecutePolicy::Once,
                    makeSelectNodesCommand(_selectionManager,
                    selection, SelectNodesClear::None));
            }

            _frustumSelectStart = QPoint();
            _frustumSelecting = false;
            _graphRenderer->clearSelectionRect();
        }
        else
        {
            const bool multiSelect = (modifiers() & Qt::ShiftModifier) != 0;

            if(!_clickedNodeId.isNull())
            {
                _commandManager->execute(ExecutePolicy::Once,
                    makeSelectNodeCommand(_selectionManager, _clickedNodeId,
                    !multiSelect ? SelectNodesClear::Selection :
                                   SelectNodesClear::None));
            }
            else if(!_selectionManager->selectedNodes().empty() && !multiSelect)
            {
                _commandManager->executeOnce(
                    [this](Command&) { return _selectionManager->clearNodeSelection(); },
                    {tr("Select None"), tr("Selecting None"), tr("Selected None")});
            }
        }

        emit clicked(Qt::LeftButton, static_cast<int>(modifiers()), _clickedNodeId);
        _selecting = false;
    }
}

void GraphCommonInteractor::leftDrag()
{
    auto* renderer = clickedComponentRenderer();

    if((modifiers() & Qt::ShiftModifier) != 0)
    {
        if(!_frustumSelecting)
        {
            // If we're not selecting already, something else will have emitted UIS
            if(_selecting)
                emit userInteractionStarted();

            _selecting = true;
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
    else if(renderer != nullptr && renderer->componentIsValid())
    {
        _selecting = false;

        if(!_mouseMoving)
            emit userInteractionStarted();

        rotateRendererByMouseMove(renderer,
            localPrevCursorPosition(), localCursorPosition());
    }
}

void GraphCommonInteractor::rightMouseDown()
{
    _selecting = true;
}

void GraphCommonInteractor::rightMouseUp()
{
    if(_selecting)
    {
        emit clicked(Qt::RightButton, static_cast<int>(modifiers()), _clickedNodeId);
        _selecting = false;
    }
}

void GraphCommonInteractor::rightDrag()
{
    _selecting = false;
}

void GraphCommonInteractor::middleMouseDown()
{
    _selecting = true;
}

void GraphCommonInteractor::middleMouseUp()
{
    if(_selecting)
    {
        emit clicked(Qt::MiddleButton, static_cast<int>(modifiers()), _clickedNodeId);
        _selecting = false;
    }
}

void GraphCommonInteractor::middleDrag()
{
    _selecting = false;
}

QPoint GraphCommonInteractor::cursorPosition() const
{
    return _cursorPosition;
}

QPoint GraphCommonInteractor::prevCursorPosition() const
{
    return _prevCursorPosition;
}

QPoint GraphCommonInteractor::localCursorPosition() const
{
    Q_ASSERT(clickedComponentRenderer() != nullptr);
    return componentLocalCursorPosition(clickedComponentRenderer()->componentId(), _cursorPosition);
}

QPoint GraphCommonInteractor::localPrevCursorPosition() const
{
    Q_ASSERT(clickedComponentRenderer() != nullptr);
    return componentLocalCursorPosition(clickedComponentRenderer()->componentId(), _prevCursorPosition);
}

Qt::KeyboardModifiers GraphCommonInteractor::modifiers() const
{
    return _modifiers;
}

NodeId GraphCommonInteractor::nodeIdAtPosition(const QPoint& localPosition) const
{
    auto* renderer = clickedComponentRenderer();
    if(renderer == nullptr || !renderer->componentIsValid())
        return {};

    auto ray = renderer->camera()->rayForViewportCoordinates(localPosition.x(), localPosition.y());

    Collision collision(*_graphModel, renderer->componentId());
    return collision.nearestNodeIntersectingLine(ray.origin(), ray.dir());
}

NodeId GraphCommonInteractor::nodeIdNearPosition(const QPoint& localPosition) const
{
    const int PICK_RADIUS = 40;

    auto* renderer = clickedComponentRenderer();
    if(renderer == nullptr || !renderer->componentIsValid())
        return {};

    auto frustum = renderer->camera()->conicalFrustumForViewportCoordinates(
                localPosition.x(), localPosition.y(), PICK_RADIUS);
    auto ray = renderer->camera()->rayForViewportCoordinates(localPosition.x(), localPosition.y());

    return nodeIdInsideFrustumNearestPoint(*_graphModel, renderer->componentId(),
                                           frustum, ray.origin());
}

void GraphCommonInteractor::wheelEvent(const QPoint& pos, int angle)
{
    if(_graphRenderer->transition().active())
        return;

    _componentRendererUnderCursor = componentRendererAtPosition(pos);

    wheelMove(static_cast<float>(angle),
        static_cast<float>(pos.x()),
        static_cast<float>(pos.y()));
}

void GraphCommonInteractor::zoomGestureEvent(const QPoint& pos, float value)
{
    if(_graphRenderer->transition().active())
        return;

    _componentRendererUnderCursor = componentRendererAtPosition(pos);

    trackpadZoomGesture(value,
        static_cast<float>(pos.x()),
        static_cast<float>(pos.y()));
}

void GraphCommonInteractor::panGestureEvent(const QPoint& pos, const QPoint& delta)
{
    if(_graphRenderer->transition().active())
        return;

    _componentRendererUnderCursor = componentRendererAtPosition(pos);

    trackpadPanGesture(
        static_cast<float>(delta.x()),
        static_cast<float>(delta.y()),
        static_cast<float>(pos.x()),
        static_cast<float>(pos.y()));
}
