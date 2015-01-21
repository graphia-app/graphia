#include "graphinteractor.h"

#include "selectionmanager.h"
#include "../commands/commandmanager.h"
#include "../rendering/graphscene.h"
#include "../graph/graphmodel.h"

class GraphWidget;

GraphInteractor::GraphInteractor(std::shared_ptr<GraphModel> /*graphModel*/,
                                 GraphScene* /*graphScene*/,
                                 CommandManager& /*commandManager*/,
                                 std::shared_ptr<SelectionManager>, /*selectionManager*/
                                 GraphWidget* parent) :
    Interactor(parent)
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

void GraphInteractor::mouseDoubleClickEvent(QMouseEvent* /*mouseEvent*/)
{
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

void GraphInteractor::wheelEvent(QWheelEvent* /*wheelEvent*/)
{
}
