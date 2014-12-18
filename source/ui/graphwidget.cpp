#include "graphwidget.h"

#include "../rendering/openglwindow.h"
#include "../rendering/graphscene.h"
#include "../rendering/graphcomponentscene.h"

#include "../graph/graphmodel.h"
#include "../graph/grapharray.h"
#include "../rendering/graphcomponentviewdata.h"

#include "../utils/utils.h"

#include "graphinteractor.h"
#include "graphcomponentinteractor.h"

#include <QVBoxLayout>

GraphWidget::GraphWidget(std::shared_ptr<GraphModel> graphModel,
                         CommandManager &commandManager,
                         std::shared_ptr<SelectionManager> selectionManager, QWidget *parent) :
    QWidget(parent),
    _graphComponentViewData(std::make_shared<ComponentArray<GraphComponentViewData>>(graphModel->graph())),
    _graphScene(std::make_shared<GraphScene>(_graphComponentViewData)),
    _graphInteractor(std::make_shared<GraphInteractor>(graphModel,
                                                       _graphScene,
                                                       commandManager,
                                                       selectionManager)),
    _graphComponentScene(std::make_shared<GraphComponentScene>(_graphComponentViewData)),
    _graphComponentInteractor(std::make_shared<GraphComponentInteractor>(graphModel,
                                                                         _graphComponentScene,
                                                                         commandManager,
                                                                         selectionManager))
{
    _openGLWindow = new OpenGLWindow;

    connect(&graphModel->graph(), &Graph::graphChanged, this, &GraphWidget::onGraphChanged);
    connect(&graphModel->graph(), &Graph::nodeWillBeRemoved, this, &GraphWidget::onNodeWillBeRemoved);
    connect(&graphModel->graph(), &Graph::componentAdded, this, &GraphWidget::onComponentAdded);
    connect(&graphModel->graph(), &Graph::componentWillBeRemoved, this, &GraphWidget::onComponentWillBeRemoved);
    connect(&graphModel->graph(), &Graph::componentSplit, this, &GraphWidget::onComponentSplit);
    connect(&graphModel->graph(), &Graph::componentsWillMerge, this, &GraphWidget::onComponentsWillMerge);

    connect(_graphInteractor.get(), &Interactor::userInteractionStarted, this, &GraphWidget::userInteractionStarted);
    connect(_graphInteractor.get(), &Interactor::userInteractionFinished, this, &GraphWidget::userInteractionFinished);
    connect(_graphComponentInteractor.get(), &Interactor::userInteractionStarted, this, &GraphWidget::userInteractionStarted);
    connect(_graphComponentInteractor.get(), &Interactor::userInteractionFinished, this, &GraphWidget::userInteractionFinished);

    setLayout(new QVBoxLayout());
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->addWidget(QWidget::createWindowContainer(_openGLWindow));

    switchToComponentMode();
}

bool GraphWidget::interacting() const
{
    return _graphComponentInteractor->interacting();
}

void GraphWidget::resetView()
{
    _graphComponentScene->resetView();
}

bool GraphWidget::viewIsReset() const
{
    return _graphComponentScene->viewIsReset();
}

void GraphWidget::toggleModes()
{
    switch(_mode)
    {
        case GraphWidget::Mode::Overview:
            switchToComponentMode();
            break;

        case GraphWidget::Mode::Component:
            switchToOverviewMode();
            break;

        default:
            break;
    }
}

void GraphWidget::switchToOverviewMode()
{
    _openGLWindow->setScene(_graphScene);
    _openGLWindow->setInteractor(_graphInteractor);
    _mode = GraphWidget::Mode::Overview;
}

void GraphWidget::switchToComponentMode(ComponentId /*componentId*/)
{
    _openGLWindow->setScene(_graphComponentScene);
    _openGLWindow->setInteractor(_graphComponentInteractor);
    _mode = GraphWidget::Mode::Component;
}

void GraphWidget::onGraphChanged(const Graph* graph)
{
    int maxNumNodes = 0;
    for(auto componentId : graph->componentIds())
    {
        auto component = graph->componentById(componentId);
        maxNumNodes = std::max(component->numNodes(), maxNumNodes);
    }

    for(auto componentId : graph->componentIds())
    {
        auto component = graph->componentById(componentId);
        int divisor = maxNumNodes / component->numNodes();

        _graphComponentViewData->at(componentId)._textureSizeDivisor =
                Utils::smallestPowerOf2GreaterThan(divisor);
    }
}

void GraphWidget::onNodeWillBeRemoved(const Graph*, NodeId)
{
}

void GraphWidget::onComponentAdded(const Graph*, ComponentId)
{
}

void GraphWidget::onComponentWillBeRemoved(const Graph*, ComponentId)
{
}

void GraphWidget::onComponentSplit(const Graph*, ComponentId, const ElementIdSet<ComponentId>&)
{
}

void GraphWidget::onComponentsWillMerge(const Graph*, const ElementIdSet<ComponentId>&, ComponentId)
{
}

void GraphWidget::onCommandWillExecuteAsynchronously(const Command*, const QString&)
{
    _openGLWindow->disableInteraction();
    _openGLWindow->disableSceneUpdate();
}

void GraphWidget::onCommandCompleted(const Command*)
{
    _openGLWindow->enableSceneUpdate();
    _openGLWindow->enableInteraction();
}
