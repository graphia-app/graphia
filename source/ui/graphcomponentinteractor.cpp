#include "graphcomponentinteractor.h"
#include "selectionmanager.h"
#include "graphquickitem.h"

#include "../commands/commandmanager.h"

#include "../rendering/graphrenderer.h"
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
                                                   GraphRenderer* graphRenderer) :
    GraphCommonInteractor(graphModel, commandManager, selectionManager, graphRenderer),
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

    if(_graphRenderer->transition().active())
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

        Plane translationPlane(clickedNodePosition, camera->viewVector());

        QVector3D prevPoint = translationPlane.rayIntersection(
                    camera->rayForViewportCoordinates(localPrevCursorPosition().x(),
                                                      localPrevCursorPosition().y()));
        QVector3D curPoint = translationPlane.rayIntersection(
                    camera->rayForViewportCoordinates(localCursorPosition().x(),
                                                      localCursorPosition().y()));
        QVector3D translation = prevPoint - curPoint;

        camera->translate(translation);
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

void GraphComponentInteractor::wheelMove(float angle, float, float)
{
    const float WHEEL_STEP_TRANSITION_SIZE = 0.2f / 120.0f;

    rendererUnderCursor()->zoom(angle * WHEEL_STEP_TRANSITION_SIZE, true);
}

void GraphComponentInteractor::trackpadScrollGesture(float, float)
{
    //FIXME Should do a pan
}

void GraphComponentInteractor::trackpadZoomGesture(float value, float, float)
{
    rendererUnderCursor()->zoom(value, false);
}

GraphComponentRenderer* GraphComponentInteractor::rendererAtPosition(const QPoint&)
{
    return _scene->componentRenderer();
}

QPoint GraphComponentInteractor::componentLocalCursorPosition(const ComponentId&, const QPoint& position)
{
    return position;
}

NodeIdSet GraphComponentInteractor::selectionForRect(const QRectF& rect)
{
    Frustum frustum = _scene->componentRenderer()->camera()->frustumForViewportCoordinates(
                rect.topLeft().x(), rect.topLeft().y(),
                rect.bottomRight().x(), rect.bottomRight().y());

    return nodeIdsInsideFrustum(*_graphModel,
                                _scene->componentRenderer()->componentId(),
                                frustum);
}
