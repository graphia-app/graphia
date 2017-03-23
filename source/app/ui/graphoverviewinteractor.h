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
    GraphOverviewInteractor(GraphModel* graphModel,
                            GraphOverviewScene* graphOverviewScene,
                            CommandManager* commandManager,
                            SelectionManager* selectionManager,
                            GraphRenderer* graphRenderer);

private:
    GraphOverviewScene* _scene;
    QPoint _panStartPosition;

    void rightMouseDown();
    void rightMouseUp();
    void rightDrag();

    void leftDoubleClick();

    void wheelMove(float angle, float x, float y);
    void trackpadZoomGesture(float value, float x, float y);

    ComponentId componentIdAtPosition(const QPoint& position) const;
    GraphComponentRenderer* rendererAtPosition(const QPoint& position) const;
    QPoint componentLocalCursorPosition(const ComponentId& componentId, const QPoint& position) const;
    NodeIdSet selectionForRect(const QRectF& rect) const;
};

#endif // GRAPHOVERVIEWINTERACTOR_H
