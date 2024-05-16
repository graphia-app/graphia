/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

#include "graphcomponentinteractor.h"
#include "selectionmanager.h"

#include "app/commands/commandmanager.h"

#include "app/rendering/camera.h"
#include "app/rendering/graphcomponentrenderer.h"
#include "app/rendering/graphcomponentscene.h"
#include "app/rendering/graphrenderer.h"

#include "app/graph/graph.h"
#include "app/graph/graphmodel.h"

#include "app/maths/boundingsphere.h"
#include "app/maths/frustum.h"
#include "app/maths/plane.h"

#include "app/layout/collision.h"

#include <QMouseEvent>
#include <QKeyEvent>
#include <QtMath>
#include <cmath>

GraphComponentInteractor::GraphComponentInteractor(GraphModel* graphModel,
                                                   GraphComponentScene* graphComponentScene,
                                                   CommandManager* commandManager,
                                                   SelectionManager* selectionManager,
                                                   GraphRenderer* graphRenderer) :
    GraphCommonInteractor(graphModel, commandManager, selectionManager, graphRenderer),
    _scene(graphComponentScene)
{
}

void GraphComponentInteractor::rightMouseDown()
{
    if(clickedComponentRenderer() != nullptr)
    {
        clickedComponentRenderer()->disableFocusTracking();
        emit userInteractionStarted();
    }

    GraphCommonInteractor::rightMouseDown();
}

void GraphComponentInteractor::rightMouseUp()
{
    if(clickedComponentRenderer() == nullptr)
        return;

    if(_graphRenderer->transition().active())
    {
        clickedComponentRenderer()->enableFocusTracking();
        return;
    }

    emit userInteractionFinished();

    if(mouseMoving())
    {
        _scene->startTransition();
        clickedComponentRenderer()->moveFocusToNodeClosestCameraVector();
    }

    clickedComponentRenderer()->enableFocusTracking();

    GraphCommonInteractor::rightMouseUp();
}

void GraphComponentInteractor::rightDrag()
{
    if(clickedComponentRenderer() != nullptr)
        _scene->pan(nearClickNodeId(), localPrevCursorPosition(), localCursorPosition());

    GraphCommonInteractor::rightDrag();
}

void GraphComponentInteractor::leftDoubleClick()
{
    if(clickedComponentRenderer() == nullptr)
        return;

    if(!nearClickNodeId().isNull())
    {
        if(nearClickNodeId() != clickedComponentRenderer()->focusNodeId())
            _scene->moveFocusToNode(nearClickNodeId());
        else
            _scene->moveFocusToNode(nearClickNodeId(), GraphComponentRenderer::COMFORTABLE_ZOOM_RADIUS);
    }
    else if(!clickedComponentRenderer()->viewIsReset())
    {
        _scene->startTransition();
        clickedComponentRenderer()->resetView();
    }
    else
        _graphRenderer->switchToOverviewMode();
}

void GraphComponentInteractor::wheelMove(float angle, float, float)
{
    auto* renderer = componentRendererUnderCursor();

    if(renderer != nullptr)
    {
        const float WHEEL_STEP_TRANSITION_SIZE = 0.2f / 120.0f;

        renderer->zoom(angle * WHEEL_STEP_TRANSITION_SIZE, true);
    }
}

void GraphComponentInteractor::trackpadZoomGesture(float value, float, float)
{
    auto* renderer = componentRendererUnderCursor();

    if(renderer != nullptr)
        renderer->zoom(value, false);
}

void GraphComponentInteractor::trackpadPanGesture(float dx, float dy, float x, float y)
{
    auto* renderer = componentRendererUnderCursor();

    if(renderer != nullptr)
    {
        rotateRendererByMouseMove(renderer,
            {static_cast<int>(x), static_cast<int>(y)},
            {static_cast<int>(x + dx), static_cast<int>(y + dy)});
    }
}

GraphComponentRenderer* GraphComponentInteractor::componentRendererAtPosition(const QPoint&) const
{
    return _scene->componentRenderer();
}

QPoint GraphComponentInteractor::componentLocalCursorPosition(const ComponentId&, const QPoint& position) const
{
    return position;
}

NodeIdSet GraphComponentInteractor::selectionForRect(const QRectF& rect) const
{
    const Frustum frustum = _scene->componentRenderer()->camera()->frustumForViewportCoordinates(
        static_cast<int>(rect.topLeft().x()), static_cast<int>(rect.topLeft().y()),
        static_cast<int>(rect.bottomRight().x()), static_cast<int>(rect.bottomRight().y()));

    return nodeIdsInsideFrustum(*_graphModel,
        _scene->componentRenderer()->componentId(), frustum);
}
