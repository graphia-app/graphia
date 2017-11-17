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

    void rightMouseDown() override;
    void rightMouseUp() override;
    void rightDrag() override;

    void leftDoubleClick() override;

    void wheelMove(float angle, float x, float y) override;
    void trackpadZoomGesture(float value, float x, float y) override;

    ComponentId componentIdAtPosition(const QPoint& position) const;
    GraphComponentRenderer* rendererAtPosition(const QPoint& position) const override;
    QPoint componentLocalCursorPosition(const ComponentId& componentId, const QPoint& position) const override;
    NodeIdSet selectionForRect(const QRectF& rect) const override;
};

#endif // GRAPHOVERVIEWINTERACTOR_H
