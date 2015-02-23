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
                                 CommandManager& /*commandManager*/,
                                 std::shared_ptr<SelectionManager>, /*selectionManager*/
                                 GraphWidget* graphWidget) :
    Interactor(graphWidget),
    _graphModel(graphModel),
    _scene(graphOverviewScene),
    _graphWidget(graphWidget)
{
}

void GraphOverviewInteractor::mousePressEvent(QMouseEvent* /*mouseEvent*/)
{
}

void GraphOverviewInteractor::mouseReleaseEvent(QMouseEvent* mouseEvent)
{
    auto& componentLayout = _scene->componentLayout();

    for(auto componentId : _graphModel->graph().componentIds())
    {
        auto& layoutData = componentLayout[componentId];

        if(layoutData._rect.contains(mouseEvent->pos()))
        {
            qDebug() << componentId;
            break;
        }
    }
}

void GraphOverviewInteractor::mouseMoveEvent(QMouseEvent* /*mouseEvent*/)
{
}

void GraphOverviewInteractor::mouseDoubleClickEvent(QMouseEvent* mouseEvent)
{
    auto& componentLayout = _scene->componentLayout();

    for(auto componentId : _graphModel->graph().componentIds())
    {
        auto& layoutData = componentLayout[componentId];

        if(layoutData._rect.contains(mouseEvent->pos()))
        {
            _graphWidget->switchToComponentMode(componentId);
            break;
        }
    }
}

void GraphOverviewInteractor::keyPressEvent(QKeyEvent* /*keyEvent*/)
{
    /*switch(keyEvent->key())
    {
    default:
        break;
    }*/
}

void GraphOverviewInteractor::keyReleaseEvent(QKeyEvent* /*keyEvent*/)
{
    /*switch(keyEvent->key())
    {
    default:
        break;
    }*/
}

void GraphOverviewInteractor::wheelEvent(QWheelEvent* wheelEvent)
{
    _scene->zoom(wheelEvent->angleDelta().y());
}
