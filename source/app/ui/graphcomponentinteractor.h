#ifndef GRAPHCOMPONENTINTERACTOR_H
#define GRAPHCOMPONENTINTERACTOR_H

#include "graphcommoninteractor.h"
#include "rendering/graphcomponentscene.h"

#include <QPoint>

#include <memory>

class GraphModel;
class CommandManager;
class SelectionManager;
class GraphQuickItem;

class GraphComponentInteractor : public GraphCommonInteractor
{
    Q_OBJECT
public:
    GraphComponentInteractor(GraphModel* graphModel,
                             GraphComponentScene* graphComponentScene,
                             CommandManager* commandManager,
                             SelectionManager* selectionManager,
                             GraphRenderer* graphRenderer);

private:
    GraphComponentScene* _scene;

    void rightMouseDown() override;
    void rightMouseUp() override;
    void rightDrag() override;

    void leftDoubleClick() override;

    void wheelMove(float angle, float x, float y) override;
    void trackpadZoomGesture(float value, float x, float y) override;

    GraphComponentRenderer* rendererAtPosition(const QPoint& position) const override;
    QPoint componentLocalCursorPosition(const ComponentId& componentId, const QPoint& position) const override;
    NodeIdSet selectionForRect(const QRectF& rect) const override;
};

#endif // GRAPHCOMPONENTINTERACTOR_H
