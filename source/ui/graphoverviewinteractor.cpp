#include "graphoverviewinteractor.h"
#include "graphwidget.h"

#include "selectionmanager.h"
#include "../commands/commandmanager.h"
#include "../rendering/graphoverviewscene.h"
#include "../graph/graphmodel.h"

#include <QMouseEvent>
#include <QWheelEvent>

GraphOverviewInteractor::GraphOverviewInteractor(std::shared_ptr<GraphModel> graphModel,
                                 GraphOverviewScene* graphOverviewScene,
                                 CommandManager& commandManager,
                                 std::shared_ptr<SelectionManager> selectionManager,
                                 GraphWidget* graphWidget) :
    GraphCommonInteractor(graphModel, commandManager, selectionManager, graphWidget),
    _scene(graphOverviewScene)
{
}

void GraphOverviewInteractor::leftDoubleClick()
{
    auto& componentLayout = _scene->componentLayout();

    for(auto componentId : _graphModel->graph().componentIds())
    {
        auto& layoutData = componentLayout[componentId];

        if(layoutData._rect.contains(cursorPosition()))
        {
            _graphWidget->switchToComponentMode(componentId);
            break;
        }
    }
}

void GraphOverviewInteractor::wheelUp()
{
    _scene->zoom(1.0f);
}

void GraphOverviewInteractor::wheelDown()
{
    _scene->zoom(-1.0f);
}

GraphComponentRenderer* GraphOverviewInteractor::rendererAtPosition(const QPoint& pos)
{
    auto& componentLayout = _scene->componentLayout();

    for(auto componentId : _graphModel->graph().componentIds())
    {
        auto& layoutData = componentLayout[componentId];

        if(layoutData._rect.contains(pos))
            return _scene->rendererForComponentId(componentId);
    }

    return nullptr;
}

QPoint GraphOverviewInteractor::componentLocalCursorPosition(const ComponentId& componentId, const QPoint& pos)
{
    auto& componentLayout = _scene->componentLayout();
    auto& layoutData = componentLayout[componentId];

    QPoint transformedPos(pos.x() - layoutData._rect.x(), pos.y() - layoutData._rect.y());
    return transformedPos;
}

ElementIdSet<NodeId> GraphOverviewInteractor::selectionForRect(const QRect& rect)
{
    ElementIdSet<NodeId> selection;

    auto& componentLayout = _scene->componentLayout();

    for(auto componentId : _graphModel->graph().componentIds())
    {
        auto& layoutData = componentLayout[componentId];

        if(rect.intersects((layoutData._rect)))
        {
            auto renderer = _scene->rendererForComponentId(componentId);
            auto subRect = rect.intersected(layoutData._rect).translated(-layoutData._rect.topLeft());

            Frustum frustum = renderer->camera()->frustumForViewportCoordinates(
                        subRect.topLeft().x(), subRect.topLeft().y(),
                        subRect.bottomRight().x(), subRect.bottomRight().y());

            auto subSelection = nodeIdsInsideFrustum(*_graphModel, componentId, frustum);
            selection.insert(subSelection.begin(), subSelection.end());
        }
    }

    return selection;
}
