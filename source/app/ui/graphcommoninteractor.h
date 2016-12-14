#ifndef GRAPHCOMMONINTERACTOR_H
#define GRAPHCOMMONINTERACTOR_H

#include "interactor.h"

#include "graph/graph.h"

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
    GraphCommonInteractor(std::shared_ptr<GraphModel> graphModel,
                          CommandManager& commandManager,
                          std::shared_ptr<SelectionManager> selectionManager,
                          GraphRenderer* graphRenderer);

protected:
    std::shared_ptr<GraphModel> _graphModel;
    CommandManager* _commandManager;
    std::shared_ptr<SelectionManager> _selectionManager;
    GraphRenderer* _graphRenderer;

private:
    QPoint _cursorPosition;
    QPoint _prevCursorPosition;
    bool _rightMouseButtonHeld = false;
    bool _leftMouseButtonHeld = false;
    bool _trackPadPanning = false;

    Qt::KeyboardModifiers _modifiers;

    bool _selecting = false;
    bool _frustumSelecting = false;
    QPoint _frustumSelectStart;

    QPoint _clickPosition;
    NodeId _clickedNodeId;
    NodeId _nearClickNodeId;

    bool _mouseMoving = false;

    GraphComponentRenderer* _clickedRenderer = nullptr;
    GraphComponentRenderer* _rendererUnderCursor = nullptr;

    void mouseDown(const QPoint &position);
    void mouseUp();
    void mousePressEvent(QMouseEvent* e) final;
    void mouseReleaseEvent(QMouseEvent* e) final;
    void mouseMoveEvent(QMouseEvent* e) final;
    void mouseDoubleClickEvent(QMouseEvent* e) final;

    void wheelEvent(QWheelEvent* e) final;

    void nativeGestureEvent(QNativeGestureEvent* e);

    virtual void leftMouseDown();
    virtual void leftMouseUp();
    virtual void leftDrag();

    virtual void rightMouseDown() {}
    virtual void rightMouseUp() {}
    virtual void rightDrag() {}

    virtual void leftDoubleClick() {}
    virtual void rightDoubleClick() {}

    virtual void wheelMove(float, float, float) {}
    virtual void trackpadZoomGesture(float, float, float) {}

    virtual GraphComponentRenderer* rendererAtPosition(const QPoint& position) const = 0;
    virtual QPoint componentLocalCursorPosition(const ComponentId& componentId, const QPoint& position) const = 0;
    virtual NodeIdSet selectionForRect(const QRectF& rect) const = 0;

protected:
    QPoint cursorPosition() const;
    QPoint prevCursorPosition() const;
    QPoint localCursorPosition() const;
    QPoint localPrevCursorPosition() const;

    Qt::KeyboardModifiers modifiers() const;

    bool mouseMoving() const { return _mouseMoving; }
    NodeId nearClickNodeId() const { return _nearClickNodeId; }

    GraphComponentRenderer* clickedRenderer() const { return _clickedRenderer; }
    GraphComponentRenderer* rendererUnderCursor() const { return _rendererUnderCursor; }

    NodeId nodeIdAtPosition(const QPoint& localPosition) const;
    NodeId nodeIdNearPosition(const QPoint& localPosition) const;
};

#endif // GRAPHCOMMONINTERACTOR_H
