#include "graphinteractor.h"
#include "graphwidget.h"

#include "selectionmanager.h"
#include "../commands/commandmanager.h"
#include "../rendering/graphscene.h"
#include "../graph/graphmodel.h"

#include <QMouseEvent>
#include <QWheelEvent>

GraphInteractor::GraphInteractor(std::shared_ptr<GraphModel> graphModel,
                                 GraphScene* graphScene,
                                 CommandManager& /*commandManager*/,
                                 std::shared_ptr<SelectionManager>, /*selectionManager*/
                                 GraphWidget* graphWidget) :
    Interactor(graphWidget),
    _graphModel(graphModel),
    _scene(graphScene),
    _graphWidget(graphWidget)
{
}

void GraphInteractor::mousePressEvent(QMouseEvent* /*mouseEvent*/)
{
}

void GraphInteractor::mouseReleaseEvent(QMouseEvent* /*mouseEvent*/)
{
}

void GraphInteractor::mouseMoveEvent(QMouseEvent* /*mouseEvent*/)
{
}

void GraphInteractor::mouseDoubleClickEvent(QMouseEvent* mouseEvent)
{
    auto& componentLayout = _scene->componentLayout();

    for(auto componentId : _graphModel->graph().componentIds())
    {
        auto& rect = componentLayout[componentId];

        if(rect.contains(mouseEvent->pos()))
        {
            _graphWidget->switchToComponentMode(componentId);
            break;
        }
    }
}

void GraphInteractor::keyPressEvent(QKeyEvent* /*keyEvent*/)
{
    /*switch(keyEvent->key())
    {
    default:
        break;
    }*/
}

void GraphInteractor::keyReleaseEvent(QKeyEvent* /*keyEvent*/)
{
    /*switch(keyEvent->key())
    {
    default:
        break;
    }*/
}

void GraphInteractor::wheelEvent(QWheelEvent* wheelEvent)
{
    _scene->zoom(wheelEvent->angleDelta().y());
}
