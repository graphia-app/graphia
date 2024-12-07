/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "graphoverviewinteractor.h"

#include "selectionmanager.h"
#include "app/commands/commandmanager.h"

#include "app/rendering/graphrenderer.h"
#include "app/rendering/graphoverviewscene.h"

#include "app/graph/graph.h"
#include "app/graph/graphmodel.h"

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
    _scene->pan(static_cast<float>(delta.x()),
        static_cast<float>(delta.y()));
}

void GraphOverviewInteractor::leftDoubleClick()
{
    auto componentId = componentIdAtPosition(cursorPosition());

    if(!componentId.isNull())
        _graphRenderer->switchToComponentMode("doTransition"_yes, componentId, clickedNodeId());
    else
        _scene->resetView();
}

void GraphOverviewInteractor::wheelMove(float angle, float x, float y)
{
    const float WHEEL_STEP_TRANSITION_SIZE = 0.2f / 120.0f;

    _scene->zoom(angle * WHEEL_STEP_TRANSITION_SIZE, x, y, "doTransition"_yes);
}

void GraphOverviewInteractor::trackpadZoomGesture(float value, float x, float y)
{
    _scene->zoom(value, x, y, "doTransition"_no);
}

void GraphOverviewInteractor::trackpadPanGesture(float dx, float dy, float x, float y)
{
    auto* renderer = componentRendererUnderCursor();

    if(renderer == nullptr)
        return;

    const QPoint from = componentLocalCursorPosition(renderer->componentId(),
        {static_cast<int>(x), static_cast<int>(y)});
    const QPoint to = componentLocalCursorPosition(renderer->componentId(),
        {static_cast<int>(x + dx), static_cast<int>(y + dy)});

    rotateRendererByMouseMove(renderer, from, to);
}

ComponentId GraphOverviewInteractor::componentIdAtPosition(const QPoint& position) const
{
    const auto& componentLayout = _scene->componentLayout();

    for(auto componentId : _graphModel->graph().componentIds())
    {
        auto radius = componentLayout[componentId].radius();
        auto separation = componentLayout[componentId].centre() - position;
        auto lengthSq = static_cast<float>((separation.x() * separation.x()) +
            (separation.y() * separation.y()));

        if(lengthSq < (radius * radius))
            return componentId;
    }

    return {};
}

GraphComponentRenderer* GraphOverviewInteractor::componentRendererAtPosition(const QPoint& position) const
{
    auto componentId = componentIdAtPosition(position);

    if(!componentId.isNull())
        return _graphRenderer->componentRendererForId(componentId);

    return nullptr;
}

QPoint GraphOverviewInteractor::componentLocalCursorPosition(const ComponentId& componentId, const QPoint& position) const
{
    const auto& componentLayout = _scene->componentLayout();
    auto rect = componentLayout[componentId].boundingBox();

    QPoint transformedPos(position.x() - static_cast<int>(rect.x()),
        position.y() - static_cast<int>(rect.y()));
    return transformedPos;
}

NodeIdSet GraphOverviewInteractor::selectionForRect(const QRectF& rect) const
{
    NodeIdSet selection;

    const auto& componentLayout = _scene->componentLayout();

    for(auto componentId : _graphModel->graph().componentIds())
    {
        auto layoutRect = componentLayout[componentId].boundingBox();

        if(rect.intersects((layoutRect)))
        {
            const auto* renderer = _graphRenderer->componentRendererForId(componentId);
            auto subRect = rect.intersected(layoutRect).translated(-layoutRect.topLeft());

            auto frustum = renderer->camera()->frustumForViewportCoordinates(
                static_cast<int>(subRect.topLeft().x()), static_cast<int>(subRect.topLeft().y()),
                static_cast<int>(subRect.bottomRight().x()), static_cast<int>(subRect.bottomRight().y()));

            auto subSelection = nodeIdsInsideFrustum(*_graphModel, componentId, frustum);
            selection.insert(subSelection.begin(), subSelection.end());
        }
    }

    return selection;
}
