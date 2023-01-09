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

#ifndef GRAPHCOMMONINTERACTOR_H
#define GRAPHCOMMONINTERACTOR_H

#include "interactor.h"

#include "shared/graph/elementid_containers.h"

#include <QPoint>
#include <QNativeGestureEvent>

class GraphModel;
class CommandManager;
class SelectionManager;
class GraphRenderer;
class GraphComponentRenderer;
class BaseFrustum;

NodeIdSet nodeIdsInsideFrustum(const GraphModel& graphModel,
                               ComponentId componentId,
                               const BaseFrustum& frustum);

class GraphCommonInteractor : public Interactor
{
    Q_OBJECT
public:
    GraphCommonInteractor(GraphModel* graphModel,
                          CommandManager* commandManager,
                          SelectionManager* selectionManager,
                          GraphRenderer* graphRenderer);

protected:
    GraphModel* _graphModel = nullptr; // NOLINT cppcoreguidelines-non-private-member-variables-in-classes
    CommandManager* _commandManager = nullptr; // NOLINT cppcoreguidelines-non-private-member-variables-in-classes
    SelectionManager* _selectionManager = nullptr; // NOLINT cppcoreguidelines-non-private-member-variables-in-classes
    GraphRenderer* _graphRenderer = nullptr; // NOLINT cppcoreguidelines-non-private-member-variables-in-classes

private:
    QPoint _cursorPosition;
    QPoint _prevCursorPosition;
    bool _rightMouseButtonHeld = false;
    bool _leftMouseButtonHeld = false;
    bool _middleMouseButtonHeld = false;

    Qt::KeyboardModifiers _modifiers;

    bool _selecting = false;
    bool _frustumSelecting = false;
    QPoint _frustumSelectStart;

    QPoint _clickPosition;
    NodeId _clickedNodeId;
    NodeId _nearClickNodeId;

    bool _mouseMoving = false;

    GraphComponentRenderer* _clickedComponentRenderer = nullptr;
    GraphComponentRenderer* _componentRendererUnderCursor = nullptr;

    void mouseDown(const QPoint &position);
    void mouseUp();
    void mousePressEvent(const QPoint& pos, Qt::KeyboardModifiers modifiers, Qt::MouseButton button) final;
    void mouseReleaseEvent(const QPoint& pos, Qt::KeyboardModifiers modifiers, Qt::MouseButton button) final;
    void mouseMoveEvent(const QPoint& pos, Qt::KeyboardModifiers modifiers, Qt::MouseButton button) final;
    void mouseDoubleClickEvent(const QPoint& pos, Qt::KeyboardModifiers modifiers, Qt::MouseButton button) final;

    void wheelEvent(const QPoint& pos, int angle) final;

    void zoomGestureEvent(const QPoint& pos, float value) override;
    void panGestureEvent(const QPoint& pos, const QPoint& delta) override;

    virtual GraphComponentRenderer* componentRendererAtPosition(const QPoint& position) const = 0;
    virtual QPoint componentLocalCursorPosition(const ComponentId& componentId, const QPoint& position) const = 0;
    virtual NodeIdSet selectionForRect(const QRectF& rect) const = 0;

protected:
    void rotateRendererByMouseMove(GraphComponentRenderer* renderer, const QPoint& from, const QPoint& to);

    virtual void leftMouseDown();
    virtual void leftMouseUp();
    virtual void leftDrag();

    virtual void rightMouseDown();
    virtual void rightMouseUp();
    virtual void rightDrag();

    virtual void middleMouseDown();
    virtual void middleMouseUp();
    virtual void middleDrag();

    virtual void leftDoubleClick() {}
    virtual void rightDoubleClick() {}
    virtual void middleDoubleClick() {}

    virtual void wheelMove(float, float, float) {}
    virtual void trackpadZoomGesture(float, float, float) {}
    virtual void trackpadPanGesture(float, float, float, float) {}

    QPoint cursorPosition() const;
    QPoint prevCursorPosition() const;
    QPoint localCursorPosition() const;
    QPoint localPrevCursorPosition() const;

    Qt::KeyboardModifiers modifiers() const;

    bool mouseMoving() const { return _mouseMoving; }
    NodeId clickedNodeId() const { return _clickedNodeId; }
    NodeId nearClickNodeId() const { return _nearClickNodeId; }

    GraphComponentRenderer* clickedComponentRenderer() const { return _clickedComponentRenderer; }
    GraphComponentRenderer* componentRendererUnderCursor() const { return _componentRendererUnderCursor; }

    NodeId nodeIdAtPosition(const QPoint& localPosition) const;
    NodeId nodeIdNearPosition(const QPoint& localPosition) const;
};

#endif // GRAPHCOMMONINTERACTOR_H
