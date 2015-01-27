#ifndef GRAPHINTERACTOR_H
#define GRAPHINTERACTOR_H

#include "interactor.h"

#include <memory>

class GraphModel;
class GraphScene;
class CommandManager;
class SelectionManager;
class GraphWidget;

class GraphInteractor : public Interactor
{
    Q_OBJECT
public:
    GraphInteractor(std::shared_ptr<GraphModel> graphModel,
                    GraphScene* graphScene,
                    CommandManager& commandManager,
                    std::shared_ptr<SelectionManager> selectionManager,
                    GraphWidget* graphWidget = nullptr);

private:
    std::shared_ptr<GraphModel> _graphModel;
    GraphScene* _scene;
    GraphWidget* _graphWidget;

    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void mouseDoubleClickEvent(QMouseEvent* e);
    void wheelEvent(QWheelEvent* e);

    void keyPressEvent(QKeyEvent* e);
    void keyReleaseEvent(QKeyEvent* e);
};

#endif // GRAPHINTERACTOR_H
