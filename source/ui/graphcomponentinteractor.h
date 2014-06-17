#ifndef GRAPHCOMPONENTINTERACTOR_H
#define GRAPHCOMPONENTINTERACTOR_H

#include "interactor.h"
#include "../rendering/graphcomponentscene.h"

class GraphModel;
class CommandManager;
class SelectionManager;

class GraphComponentInteractor : public Interactor
{
    Q_OBJECT
public:
    GraphComponentInteractor(GraphModel* graphModel,
                             GraphComponentScene* graphComponentScene,
                             CommandManager* commandManager,
                             SelectionManager* selectionManager);
    virtual ~GraphComponentInteractor() {}

private:
    GraphModel* _graphModel;
    GraphComponentScene* _scene;
    CommandManager* _commandManager;
    SelectionManager* _selectionManager;

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

    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void mouseDoubleClickEvent(QMouseEvent* e);
    void wheelEvent(QWheelEvent* e);

    void keyPressEvent(QKeyEvent* e);
    void keyReleaseEvent(QKeyEvent* e);
};

#endif // GRAPHCOMPONENTINTERACTOR_H
