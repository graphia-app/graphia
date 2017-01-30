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
    GraphComponentInteractor(const std::shared_ptr<GraphModel>& graphModel,
                             GraphComponentScene* graphComponentScene,
                             CommandManager& commandManager,
                             const std::shared_ptr<SelectionManager>& selectionManager,
                             GraphRenderer* graphRenderer);

private:
    GraphComponentScene* _scene;

    void rightMouseDown();
    void rightMouseUp();
    void rightDrag();

    void leftDoubleClick();

    void wheelMove(float angle, float x, float y);
    void trackpadZoomGesture(float value, float x, float y);

    GraphComponentRenderer* rendererAtPosition(const QPoint& position) const;
    QPoint componentLocalCursorPosition(const ComponentId& componentId, const QPoint& position) const;
    NodeIdSet selectionForRect(const QRectF& rect) const;
};

#endif // GRAPHCOMPONENTINTERACTOR_H
