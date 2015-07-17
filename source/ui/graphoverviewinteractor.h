#ifndef GRAPHOVERVIEWINTERACTOR_H
#define GRAPHOVERVIEWINTERACTOR_H

#include "graphcommoninteractor.h"

#include <memory>

class GraphModel;
class GraphOverviewScene;
class CommandManager;
class SelectionManager;
class GraphQuickItem;

class GraphOverviewInteractor : public GraphCommonInteractor
{
    Q_OBJECT
public:
    GraphOverviewInteractor(std::shared_ptr<GraphModel> graphModel,
                            GraphOverviewScene* graphOverviewScene,
                            CommandManager& commandManager,
                            std::shared_ptr<SelectionManager> selectionManager,
                            GraphRenderer* graphRenderer);

private:
    GraphOverviewScene* _scene;

    void leftDoubleClick();

    void wheelMove(float angle);
    void trackpadScrollGesture(float pixels);
    void trackpadZoomGesture(float value);

    GraphComponentRenderer* rendererAtPosition(const QPoint& position);
    QPoint componentLocalCursorPosition(const ComponentId& componentId, const QPoint& position);
    ElementIdSet<NodeId> selectionForRect(const QRect& rect);
};

#endif // GRAPHOVERVIEWINTERACTOR_H
