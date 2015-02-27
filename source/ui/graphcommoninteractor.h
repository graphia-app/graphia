#ifndef GRAPHCOMMONINTERACTOR_H
#define GRAPHCOMMONINTERACTOR_H

#include "interactor.h"

#include "../graph/graph.h"

#include <QPoint>

class GraphModel;
class CommandManager;
class SelectionManager;
class GraphWidget;
class GraphComponentRenderer;
class BaseFrustum;

ElementIdSet<NodeId> nodeIdsInsideFrustum(const GraphModel& graphModel,
                                          ComponentId componentId,
                                          const BaseFrustum& frustum);

class GraphCommonInteractor : public Interactor
{
    Q_OBJECT
public:
    GraphCommonInteractor(std::shared_ptr<GraphModel> graphModel,
                          CommandManager& commandManager,
                          std::shared_ptr<SelectionManager> selectionManager,
                          GraphWidget* graphWidget);

protected:
    std::shared_ptr<GraphModel> _graphModel;
    CommandManager& _commandManager;
    std::shared_ptr<SelectionManager> _selectionManager;
    GraphWidget* _graphWidget;

private:
    QPoint _cursorPosition;
    QPoint _prevCursorPosition;
    bool _rightMouseButtonHeld;
    bool _leftMouseButtonHeld;

    Qt::KeyboardModifiers _modifiers;

    bool _selecting;
    bool _frustumSelecting;
    QPoint _frustumSelectStart;

    QPoint _clickPosition;
    NodeId _clickedNodeId;
    NodeId _nearClickNodeId;

    bool _mouseMoving;

    GraphComponentRenderer* _clickedRenderer;
    GraphComponentRenderer* _rendererUnderCursor;

    void mousePressEvent(QMouseEvent* e) final;
    void mouseReleaseEvent(QMouseEvent* e) final;
    void mouseMoveEvent(QMouseEvent* e) final;
    void mouseDoubleClickEvent(QMouseEvent* e) final;

    void wheelEvent(QWheelEvent* e) final;

    virtual void leftMouseDown();
    virtual void leftMouseUp();
    virtual void leftDrag();

    virtual void rightMouseDown() {}
    virtual void rightMouseUp() {}
    virtual void rightDrag() {}

    virtual void leftDoubleClick() {}
    virtual void rightDoubleClick() {}

    virtual void wheelUp() {}
    virtual void wheelDown() {}

    virtual GraphComponentRenderer* rendererAtPosition(const QPoint& position) = 0;
    virtual QPoint componentLocalCursorPosition(const ComponentId& componentId, const QPoint& position) = 0;
    virtual ElementIdSet<NodeId> selectionForRect(const QRect& rect) = 0;

protected:
    QPoint cursorPosition();
    QPoint prevCursorPosition();
    QPoint localCursorPosition();
    QPoint localPrevCursorPosition();

    Qt::KeyboardModifiers modifiers();

    bool mouseMoving() { return _mouseMoving; }
    NodeId clickedNodeId() { return _nearClickNodeId; }

    GraphComponentRenderer* clickedRenderer() { return _clickedRenderer; }
    GraphComponentRenderer* rendererUnderCursor() { return _rendererUnderCursor; }
};

#endif // GRAPHCOMMONINTERACTOR_H
