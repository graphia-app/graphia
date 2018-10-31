#include "graphoverviewinteractor.h"
#include "graphquickitem.h"

#include "selectionmanager.h"
#include "commands/commandmanager.h"

#include "rendering/graphrenderer.h"
#include "rendering/graphoverviewscene.h"

#include "graph/graph.h"
#include "graph/graphmodel.h"

#include <QMouseEvent>
#include <QWheelEvent>

GraphOverviewInteractor::GraphOverviewInteractor(GraphModel* graphModel,
                                                 GraphOverviewScene* graphOverviewScene,
                                                 CommandManager* commandManager,
                                                 SelectionManager* selectionManager,
                                                 GraphRenderer* graphRenderer) :
    GraphCommonInteractor(graphModel, commandManager, selectionManager, graphRenderer),
    _scene(graphOverviewScene)
{
}

void GraphOverviewInteractor::rightMouseDown()
{
    GraphCommonInteractor::rightMouseDown();

    _panStartPosition = cursorPosition();
}

void GraphOverviewInteractor::rightMouseUp()
{
    GraphCommonInteractor::rightMouseUp();

    emit userInteractionFinished();
}

void GraphOverviewInteractor::rightDrag()
{
    GraphCommonInteractor::rightDrag();

    if(!mouseMoving())
        emit userInteractionStarted();

    auto delta = cursorPosition() - prevCursorPosition();
    _scene->pan(delta.x(), delta.y());
}

void GraphOverviewInteractor::leftDoubleClick()
{
    auto componentId = componentIdAtPosition(cursorPosition());

    if(!componentId.isNull())
        _graphRenderer->switchToComponentMode(true, componentId, clickedNodeId());
}

void GraphOverviewInteractor::wheelMove(float angle, float x, float y)
{
    if(angle > 0.0f)
        _scene->zoom(GraphOverviewScene::ZoomType::In, x, y, true);
    else
        _scene->zoom(GraphOverviewScene::ZoomType::Out, x, y, true);
}

void GraphOverviewInteractor::trackpadZoomGesture(float value, float x, float y)
{
    _scene->zoom(value, x, y, false);
}

ComponentId GraphOverviewInteractor::componentIdAtPosition(const QPoint& position) const
{
    auto& componentLayout = _scene->componentLayout();

    for(auto componentId : _graphModel->graph().componentIds())
    {
        auto radius = componentLayout[componentId].radius();
        auto separation = componentLayout[componentId].centre() - position;
        float lengthSq = (separation.x() * separation.x()) + (separation.y() * separation.y());

        if(lengthSq < (radius * radius))
            return componentId;
    }

    return {};
}

GraphComponentRenderer* GraphOverviewInteractor::rendererAtPosition(const QPoint& position) const
{
    auto componentId = componentIdAtPosition(position);

    if(!componentId.isNull())
        return _graphRenderer->componentRendererForId(componentId);

    return nullptr;
}

QPoint GraphOverviewInteractor::componentLocalCursorPosition(const ComponentId& componentId, const QPoint& position) const
{
    auto& componentLayout = _scene->componentLayout();
    auto rect = componentLayout[componentId].boundingBox();

    QPoint transformedPos(position.x() - rect.x(), position.y() - rect.y());
    return transformedPos;
}

NodeIdSet GraphOverviewInteractor::selectionForRect(const QRectF& rect) const
{
    NodeIdSet selection;

    auto& componentLayout = _scene->componentLayout();

    for(auto componentId : _graphModel->graph().componentIds())
    {
        auto layoutRect = componentLayout[componentId].boundingBox();

        if(rect.intersects((layoutRect)))
        {
            auto renderer = _graphRenderer->componentRendererForId(componentId);
            auto subRect = rect.intersected(layoutRect).translated(-layoutRect.topLeft());

            auto frustum = renderer->camera()->frustumForViewportCoordinates(
                        subRect.topLeft().x(), subRect.topLeft().y(),
                        subRect.bottomRight().x(), subRect.bottomRight().y());

            auto subSelection = nodeIdsInsideFrustum(*_graphModel, componentId, frustum);
            selection.insert(subSelection.begin(), subSelection.end());
        }
    }

    return selection;
}
