#ifndef GRAPHCOMPONENTINTERACTOR_H
#define GRAPHCOMPONENTINTERACTOR_H

#include "graphcommoninteractor.h"
#include "../rendering/graphcomponentscene.h"

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
    GraphComponentInteractor(std::shared_ptr<GraphModel> graphModel,
                             GraphComponentScene* graphComponentScene,
                             CommandManager& commandManager,
                             std::shared_ptr<SelectionManager> selectionManager,
                             GraphRenderer* graphRenderer);

private:
    GraphComponentScene* _scene;

    void rightMouseDown();
    void rightMouseUp();
    void rightDrag();

    void leftDoubleClick();

    void wheelMove(float angle);
    void trackpadScrollGesture(float pixels);
    void trackpadZoomGesture(float value);

    GraphComponentRenderer* rendererAtPosition(const QPoint& position);
    QPoint componentLocalCursorPosition(const ComponentId& componentId, const QPoint& position);
    NodeIdSet selectionForRect(const QRectF& rect);
};

#endif // GRAPHCOMPONENTINTERACTOR_H
