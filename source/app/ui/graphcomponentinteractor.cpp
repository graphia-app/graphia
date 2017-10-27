#include "graphcomponentinteractor.h"
#include "selectionmanager.h"
#include "graphquickitem.h"

#include "commands/commandmanager.h"

#include "rendering/camera.h"
#include "rendering/graphcomponentrenderer.h"
#include "rendering/graphcomponentscene.h"
#include "rendering/graphrenderer.h"

#include "graph/graph.h"
#include "graph/graphmodel.h"

#include "maths/boundingsphere.h"
#include "maths/frustum.h"
#include "maths/plane.h"

#include "layout/collision.h"

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
    GraphCommonInteractor::rightMouseDown();

    if(clickedRenderer() != nullptr)
    {
        clickedRenderer()->disableFocusTracking();
        emit userInteractionStarted();
    }
}

void GraphComponentInteractor::rightMouseUp()
{
    GraphCommonInteractor::rightMouseUp();

    if(clickedRenderer() == nullptr)
        return;

    if(_graphRenderer->transition().active())
    {
        clickedRenderer()->enableFocusTracking();
        return;
    }

    emit userInteractionFinished();

    if(mouseMoving())
    {
        _scene->startTransition();
        clickedRenderer()->moveFocusToNodeClosestCameraVector();
    }

    clickedRenderer()->enableFocusTracking();
}

void GraphComponentInteractor::rightDrag()
{
    GraphCommonInteractor::rightDrag();

    if(clickedRenderer() != nullptr)
        _scene->pan(nearClickNodeId(), localPrevCursorPosition(), localCursorPosition());
}

void GraphComponentInteractor::leftDoubleClick()
{
    if(clickedRenderer() == nullptr)
        return;

    if(!nearClickNodeId().isNull())
    {
        if(nearClickNodeId() != clickedRenderer()->focusNodeId())
            _scene->moveFocusToNode(nearClickNodeId());
        else
            _scene->moveFocusToNode(nearClickNodeId(), GraphComponentRenderer::COMFORTABLE_ZOOM_DISTANCE);
    }
    else if(!clickedRenderer()->viewIsReset())
    {
        _scene->startTransition();
        clickedRenderer()->resetView();
    }
    else
        _graphRenderer->switchToOverviewMode();
}

void GraphComponentInteractor::wheelMove(float angle, float, float)
{
    const float WHEEL_STEP_TRANSITION_SIZE = 0.2f / 120.0f;

    rendererUnderCursor()->zoom(angle * WHEEL_STEP_TRANSITION_SIZE, true);
}

void GraphComponentInteractor::trackpadZoomGesture(float value, float, float)
{
    rendererUnderCursor()->zoom(value, false);
}

GraphComponentRenderer* GraphComponentInteractor::rendererAtPosition(const QPoint&) const
{
    return _scene->componentRenderer();
}

QPoint GraphComponentInteractor::componentLocalCursorPosition(const ComponentId&, const QPoint& position) const
{
    return position;
}

NodeIdSet GraphComponentInteractor::selectionForRect(const QRectF& rect) const
{
    Frustum frustum = _scene->componentRenderer()->camera()->frustumForViewportCoordinates(
                rect.topLeft().x(), rect.topLeft().y(),
                rect.bottomRight().x(), rect.bottomRight().y());

    return nodeIdsInsideFrustum(*_graphModel,
                                _scene->componentRenderer()->componentId(),
                                frustum);
}
