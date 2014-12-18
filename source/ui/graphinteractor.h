#ifndef GRAPHINTERACTOR_H
#define GRAPHINTERACTOR_H

#include "interactor.h"

#include <memory>

class GraphModel;
class GraphScene;
class CommandManager;
class SelectionManager;

class GraphInteractor : public Interactor
{
    Q_OBJECT
public:
    GraphInteractor(std::shared_ptr<GraphModel> graphModel,
                    std::shared_ptr<GraphScene> graphScene,
                    CommandManager& commandManager,
                    std::shared_ptr<SelectionManager> selectionManager);

private:
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void mouseDoubleClickEvent(QMouseEvent* e);
    void wheelEvent(QWheelEvent* e);

    void keyPressEvent(QKeyEvent* e);
    void keyReleaseEvent(QKeyEvent* e);
};

#endif // GRAPHINTERACTOR_H
