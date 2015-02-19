#ifndef GRAPHCOMPONENTINTERACTOR_H
#define GRAPHCOMPONENTINTERACTOR_H

#include "interactor.h"
#include "../rendering/graphcomponentscene.h"

#include <QPoint>

#include <memory>

class GraphModel;
class CommandManager;
class SelectionManager;
class GraphWidget;

class GraphComponentInteractor : public Interactor
{
    Q_OBJECT
public:
    GraphComponentInteractor(std::shared_ptr<GraphModel> graphModel,
                             GraphComponentScene* graphComponentScene,
                             CommandManager& commandManager,
                             std::shared_ptr<SelectionManager> selectionManager,
                             GraphWidget* graphWidget = nullptr);

private:
    std::shared_ptr<GraphModel> _graphModel;
    GraphComponentScene* _scene;
    CommandManager& _commandManager;
    std::shared_ptr<SelectionManager> _selectionManager;
    GraphWidget* _graphWidget;

    bool _rightMouseButtonHeld;
    bool _leftMouseButtonHeld;

    bool _selecting;
    bool _frustumSelecting;
    QPoint _frustumSelectStart;

    QPoint _prevCursorPosition;
    QPoint _cursorPosition;
    QPoint _clickPosition;
    bool _mouseMoving;
    NodeId _clickedNodeId;
    NodeId _nearClickNodeId;

    QQuaternion mouseMoveToRotation();

    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void mouseDoubleClickEvent(QMouseEvent* e);
    void wheelEvent(QWheelEvent* e);

    void keyPressEvent(QKeyEvent* e);
    void keyReleaseEvent(QKeyEvent* e);
};

#endif // GRAPHCOMPONENTINTERACTOR_H
