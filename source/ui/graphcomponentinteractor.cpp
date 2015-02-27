#include "graphcomponentinteractor.h"
#include "selectionmanager.h"
#include "graphwidget.h"

#include "../commands/commandmanager.h"

#include "../rendering/graphcomponentscene.h"
#include "../rendering/graphcomponentrenderer.h"
#include "../rendering/camera.h"

#include "../graph/graph.h"
#include "../graph/graphmodel.h"

#include "../maths/frustum.h"
#include "../maths/plane.h"
#include "../maths/boundingsphere.h"

#include "../layout/collision.h"

#include "../utils/utils.h"

#include <QMouseEvent>
#include <QKeyEvent>
#include <QtMath>
#include <cmath>

GraphComponentInteractor::GraphComponentInteractor(std::shared_ptr<GraphModel> graphModel,
                                                   GraphComponentScene* graphComponentScene,
                                                   CommandManager &commandManager,
                                                   std::shared_ptr<SelectionManager> selectionManager,
                                                   GraphWidget* graphWidget) :
    GraphCommonInteractor(graphModel, commandManager, selectionManager, graphWidget),
    _scene(graphComponentScene)
{
}

void GraphComponentInteractor::rightMouseDown()
{
    if(clickedRenderer() != nullptr && !clickedNodeId().isNull())
        clickedRenderer()->disableFocusTracking();
}

void GraphComponentInteractor::rightMouseUp()
{
    if(clickedRenderer() == nullptr)
        return;

    if(!_graphWidget->transition().finished())
    {
        clickedRenderer()->enableFocusTracking();
        return;
    }

    emit userInteractionFinished();

    if(!clickedNodeId().isNull() && mouseMoving())
    {
        _scene->startTransition();
        clickedRenderer()->moveFocusToNodeClosestCameraVector();
    }

    clickedRenderer()->enableFocusTracking();
}

void GraphComponentInteractor::rightDrag()
{
    if(clickedRenderer() != nullptr && !clickedNodeId().isNull())
    {
        Camera* camera = clickedRenderer()->camera();

        if(!mouseMoving())
            emit userInteractionStarted();

        const QVector3D clickedNodePosition = _graphModel->nodePositions().getScaledAndSmoothed(clickedNodeId());

        Plane translationPlane(clickedNodePosition, camera->viewVector().normalized());

        QVector3D prevPoint = translationPlane.rayIntersection(
                    camera->rayForViewportCoordinates(localPrevCursorPosition().x(),
                                                      localPrevCursorPosition().y()));
        QVector3D curPoint = translationPlane.rayIntersection(
                    camera->rayForViewportCoordinates(localCursorPosition().x(),
                                                      localCursorPosition().y()));
        QVector3D translation = prevPoint - curPoint;

        camera->translateWorld(translation);
    }
}

void GraphComponentInteractor::leftDoubleClick()
{
    if(clickedRenderer() == nullptr)
        return;

    if(!clickedNodeId().isNull())
    {
        if(clickedNodeId() != clickedRenderer()->focusNodeId())
        {
            _scene->startTransition();
            clickedRenderer()->moveFocusToNode(clickedNodeId());
        }
    }
    else if(!clickedRenderer()->viewIsReset())
    {
        _scene->startTransition();
        clickedRenderer()->resetView();
    }
}

void GraphComponentInteractor::wheelUp()
{
    rendererUnderCursor()->zoom(1.0f);
}

void GraphComponentInteractor::wheelDown()
{
    rendererUnderCursor()->zoom(-1.0f);
}

GraphComponentRenderer*GraphComponentInteractor::rendererAtPosition(const QPoint&)
{
    return _scene->renderer();
}

QPoint GraphComponentInteractor::componentLocalCursorPosition(const ComponentId&, const QPoint& position)
{
    return position;
}

ElementIdSet<NodeId> GraphComponentInteractor::selectionForRect(const QRect& rect)
{
    Frustum frustum = _scene->renderer()->camera()->frustumForViewportCoordinates(
                rect.topLeft().x(), rect.topLeft().y(),
                rect.bottomRight().x(), rect.bottomRight().y());

    return nodeIdsInsideFrustum(*_graphModel,
                                _scene->renderer()->componentId(),
                                frustum);
}
