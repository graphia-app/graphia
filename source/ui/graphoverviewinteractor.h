#ifndef GRAPHOVERVIEWINTERACTOR_H
#define GRAPHOVERVIEWINTERACTOR_H

#include "graphcommoninteractor.h"

#include <QPoint>

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
    QPoint _panStartPosition;

    void rightMouseDown();
    void rightMouseUp();
    void rightDrag();

    void leftDoubleClick();

    void wheelMove(float angle, float x, float y);
    void trackpadScrollGesture(float x, float y);
    void trackpadZoomGesture(float value, float x, float y);

    GraphComponentRenderer* rendererAtPosition(const QPoint& position);
    QPoint componentLocalCursorPosition(const ComponentId& componentId, const QPoint& position);
    NodeIdSet selectionForRect(const QRectF& rect);
};

#endif // GRAPHOVERVIEWINTERACTOR_H
