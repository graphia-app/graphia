#include "graphoverviewinteractor.h"
#include "graphquickitem.h"

#include "selectionmanager.h"
#include "../commands/commandmanager.h"

#include "../rendering/graphrenderer.h"
#include "../rendering/graphoverviewscene.h"

#include "../graph/graphmodel.h"

#include <QMouseEvent>
#include <QWheelEvent>

GraphOverviewInteractor::GraphOverviewInteractor(std::shared_ptr<GraphModel> graphModel,
                                                 GraphOverviewScene* graphOverviewScene,
                                                 CommandManager& commandManager,
                                                 std::shared_ptr<SelectionManager> selectionManager,
                                                 GraphRenderer* graphRenderer) :
    GraphCommonInteractor(graphModel, commandManager, selectionManager, graphRenderer),
    _scene(graphOverviewScene)
{
}

void GraphOverviewInteractor::rightMouseDown()
{
    _panStartPosition = cursorPosition();
}

void GraphOverviewInteractor::rightMouseUp()
{
    emit userInteractionFinished();
}

void GraphOverviewInteractor::rightDrag()
{
    if(!mouseMoving())
        emit userInteractionStarted();

    QPoint delta = cursorPosition() - prevCursorPosition();
    _scene->pan(delta.x(), delta.y());
}

void GraphOverviewInteractor::leftDoubleClick()
{
    auto& componentLayout = _scene->componentLayout();

    for(auto componentId : _graphModel->graph().componentIds())
    {
        if(componentLayout[componentId].contains(cursorPosition()))
        {
            _graphRenderer->switchToComponentMode(true, componentId);
            break;
        }
    }
}

void GraphOverviewInteractor::wheelMove(float angle, float x, float y)
{
    _scene->zoom(angle, x, y, true);
}

void GraphOverviewInteractor::trackpadScrollGesture(float x, float y)
{
    //FIXME test
    _scene->pan(x, y);
}

void GraphOverviewInteractor::trackpadZoomGesture(float value, float x, float y)
{
    //FIXME test
    _scene->zoom(value, x, y, false);
}

GraphComponentRenderer* GraphOverviewInteractor::rendererAtPosition(const QPoint& pos)
{
    auto& componentLayout = _scene->componentLayout();

    for(auto componentId : _graphModel->graph().componentIds())
    {
        if(componentLayout[componentId].contains(pos))
            return _graphRenderer->componentRendererForId(componentId);
    }

    return nullptr;
}

QPoint GraphOverviewInteractor::componentLocalCursorPosition(const ComponentId& componentId, const QPoint& pos)
{
    auto& componentLayout = _scene->componentLayout();
    auto& rect = componentLayout[componentId];

    QPoint transformedPos(pos.x() - rect.x(), pos.y() - rect.y());
    return transformedPos;
}

NodeIdSet GraphOverviewInteractor::selectionForRect(const QRectF& rect)
{
    NodeIdSet selection;

    auto& componentLayout = _scene->componentLayout();

    for(auto componentId : _graphModel->graph().componentIds())
    {
        auto& layoutRect = componentLayout[componentId];

        if(rect.intersects((layoutRect)))
        {
            auto renderer = _graphRenderer->componentRendererForId(componentId);
            auto subRect = rect.intersected(layoutRect).translated(-layoutRect.topLeft());

            Frustum frustum = renderer->camera()->frustumForViewportCoordinates(
                        subRect.topLeft().x(), subRect.topLeft().y(),
                        subRect.bottomRight().x(), subRect.bottomRight().y());

            auto subSelection = nodeIdsInsideFrustum(*_graphModel, componentId, frustum);
            selection.insert(subSelection.begin(), subSelection.end());
        }
    }

    return selection;
}
