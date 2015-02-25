#ifndef GRAPHINTERACTOR_H
#define GRAPHINTERACTOR_H

#include "graphcommoninteractor.h"

#include <memory>

class GraphModel;
class GraphOverviewScene;
class CommandManager;
class SelectionManager;
class GraphWidget;

class GraphOverviewInteractor : public GraphCommonInteractor
{
    Q_OBJECT
public:
    GraphOverviewInteractor(std::shared_ptr<GraphModel> graphModel,
                    GraphOverviewScene* graphOverviewScene,
                    CommandManager& commandManager,
                    std::shared_ptr<SelectionManager> selectionManager,
                    GraphWidget* graphWidget = nullptr);

private:
    std::shared_ptr<GraphModel> _graphModel;
    GraphOverviewScene* _scene;
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
