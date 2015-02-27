#ifndef GRAPHCOMPONENTINTERACTOR_H
#define GRAPHCOMPONENTINTERACTOR_H

#include "graphcommoninteractor.h"
#include "../rendering/graphcomponentscene.h"

#include <QPoint>

#include <memory>

class GraphModel;
class CommandManager;
class SelectionManager;
class GraphWidget;

class GraphComponentInteractor : public GraphCommonInteractor
{
    Q_OBJECT
public:
    GraphComponentInteractor(std::shared_ptr<GraphModel> graphModel,
                             GraphComponentScene* graphComponentScene,
                             CommandManager& commandManager,
                             std::shared_ptr<SelectionManager> selectionManager,
                             GraphWidget* graphWidget = nullptr);

private:
    GraphComponentScene* _scene;

    void rightMouseDown();
    void rightMouseUp();
    void rightDrag();

    void leftDoubleClick();

    virtual void wheelUp();
    virtual void wheelDown();

    GraphComponentRenderer* rendererAtPosition(const QPoint& position);
    QPoint componentLocalCursorPosition(const ComponentId& componentId, const QPoint& position);
    ElementIdSet<NodeId> selectionForRect(const QRect& rect);
};

#endif // GRAPHCOMPONENTINTERACTOR_H
