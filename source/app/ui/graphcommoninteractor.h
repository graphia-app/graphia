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

    enum class TrackpadPanningState
    {
        Inactive,
        Initiated,
        Active
    }
    _trackPadPanningState = {};

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
    void mousePressEvent(QMouseEvent* mouseEvent) final;
    void mouseReleaseEvent(QMouseEvent* mouseEvent) final;
    void mouseMoveEvent(QMouseEvent* mouseEvent) final;
    void mouseDoubleClickEvent(QMouseEvent* mouseEvent) final;

    void wheelEvent(QWheelEvent* wheelEvent) final;

    void nativeGestureEvent(QNativeGestureEvent* nativeEvent) override;

    virtual GraphComponentRenderer* componentRendererAtPosition(const QPoint& position) const = 0;
    virtual QPoint componentLocalCursorPosition(const ComponentId& componentId, const QPoint& position) const = 0;
    virtual NodeIdSet selectionForRect(const QRectF& rect) const = 0;

protected:
    virtual void leftMouseDown();
    virtual void leftMouseUp();
    virtual void leftDrag();

    virtual void rightMouseDown();
    virtual void rightMouseUp();
    virtual void rightDrag();

    virtual void leftDoubleClick() {}
    virtual void rightDoubleClick() {}

    virtual void wheelMove(float, float, float) {}
    virtual void trackpadZoomGesture(float, float, float) {}

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
