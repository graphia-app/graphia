#include "graphwidget.h"

#include "../rendering/openglwindow.h"
#include "../rendering/graphscene.h"
#include "../rendering/graphcomponentscene.h"

#include "../graph/graphmodel.h"
#include "../graph/grapharray.h"
#include "../rendering/graphcomponentrenderer.h"

#include "../utils/make_unique.h"
#include "../utils/utils.h"

#include "graphinteractor.h"
#include "graphcomponentinteractor.h"
#include "selectionmanager.h"

#include <QVBoxLayout>

GraphWidget::GraphWidget(std::shared_ptr<GraphModel> graphModel,
                         CommandManager& commandManager,
                         std::shared_ptr<SelectionManager> selectionManager, QWidget* parent) :
    QWidget(parent),
    _initialised(false),
    _openGLWindow(new OpenGLWindow),
    _sceneUpdateEnabled(true),
    _graphModel(graphModel),
    _selectionManager(selectionManager),
    _graphComponentRendererShared(std::make_shared<GraphComponentRendererShared>(_openGLWindow->context())),
    _graphComponentRendererManagers(std::make_shared<ComponentArray<GraphComponentRendererManager>>(graphModel->graph())),
    _numTransitioningRenderers(0),
    _graphScene(new GraphScene(this)),
    _graphInteractor(new GraphInteractor(graphModel, _graphScene, commandManager, selectionManager, this)),
    _graphComponentScene(new GraphComponentScene(this)),
    _graphComponentInteractor(new GraphComponentInteractor(graphModel, _graphComponentScene, commandManager, selectionManager, this)),
    _mode(GraphWidget::Mode::Component)
{
    _graphScene->setGraphComponentRendererManagers(_graphComponentRendererManagers);
    _graphComponentScene->setGraphComponentRendererManagers(_graphComponentRendererManagers);

    connect(&graphModel->graph(), &Graph::graphChanged, this, &GraphWidget::onGraphChanged, Qt::DirectConnection);
    connect(&graphModel->graph(), &Graph::nodeWillBeRemoved, this, &GraphWidget::onNodeWillBeRemoved, Qt::DirectConnection);
    connect(&graphModel->graph(), &Graph::componentAdded, this, &GraphWidget::onComponentAdded, Qt::DirectConnection);
    connect(&graphModel->graph(), &Graph::componentWillBeRemoved, this, &GraphWidget::onComponentWillBeRemoved, Qt::DirectConnection);

    connect(selectionManager.get(), &SelectionManager::selectionChanged, this, &GraphWidget::onSelectionChanged, Qt::DirectConnection);

    connect(_graphInteractor, &Interactor::userInteractionStarted, this, &GraphWidget::userInteractionStarted);
    connect(_graphInteractor, &Interactor::userInteractionFinished, this, &GraphWidget::userInteractionFinished);
    connect(_graphComponentInteractor, &Interactor::userInteractionStarted, this, &GraphWidget::userInteractionStarted);
    connect(_graphComponentInteractor, &Interactor::userInteractionFinished, this, &GraphWidget::userInteractionFinished);

    setLayout(new QVBoxLayout());
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->addWidget(QWidget::createWindowContainer(_openGLWindow));

    switchToComponentMode();

    _timer = new QTimer(this);
    connect(_timer, &QTimer::timeout, this, &GraphWidget::onUpdate);
    _timer->start(16);
}

GraphWidget::~GraphWidget()
{
    _timer->stop();
}

void GraphWidget::initialise()
{
    Graph* graph = &_graphModel->graph();

    for(auto componentId : graph->componentIds())
        onComponentAdded(graph, componentId);

    onGraphChanged(graph);

    switchToComponentMode();

    _initialised = true;
}

void GraphWidget::onUpdate()
{
    if(_sceneUpdateEnabled)
    {
        _preUpdateExecutor.execute();
        _openGLWindow->update();
    }

    _openGLWindow->render();
}

bool GraphWidget::interacting() const
{
    return _graphComponentInteractor->interacting();
}

void GraphWidget::resetView()
{
    if(_mode == GraphWidget::Mode::Component)
        _graphComponentScene->resetView();
}

bool GraphWidget::viewIsReset() const
{
    if(_mode == GraphWidget::Mode::Component)
        return _graphComponentScene->viewIsReset();

    return true;
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
    executeOnRendererThread([this]
    {
        _openGLWindow->setScene(_graphScene);
        _openGLWindow->setInteractor(_graphInteractor);
        _mode = GraphWidget::Mode::Overview;
    }, "GraphWidget::switchToOverviewMode");
}

void GraphWidget::switchToComponentMode(ComponentId componentId)
{
    executeOnRendererThread([this, componentId]
    {
        if(componentId.isNull())
            _graphComponentScene->setComponentId(_defaultComponentId);
        else
            _graphComponentScene->setComponentId(componentId);

        _openGLWindow->setScene(_graphComponentScene);

        _openGLWindow->setInteractor(_graphComponentInteractor);
        _mode = GraphWidget::Mode::Component;
    }, "GraphWidget::switchToComponentMode");
}

void GraphWidget::rendererStartedTransition()
{
    Q_ASSERT(_numTransitioningRenderers >= 0);

    if(_numTransitioningRenderers == 0)
        emit userInteractionStarted();

    _numTransitioningRenderers++;
}

void GraphWidget::rendererFinishedTransition()
{
    _numTransitioningRenderers--;

    Q_ASSERT(_numTransitioningRenderers >= 0);

    if(_numTransitioningRenderers == 0)
        emit userInteractionFinished();
}

void GraphWidget::executeOnRendererThread(DeferredExecutor::TaskFn task, const QString& description)
{
    _preUpdateExecutor.enqueue(task, description);
}

void GraphWidget::updateNodePositions()
{
    _graphModel->nodePositions().executeIfUpdated([this]
    {
        for(auto componentId : _graphModel->graph().componentIds())
        {
            auto graphComponentRenderer = _graphComponentRendererManagers->at(componentId).get();

            if(graphComponentRenderer->visible())
                graphComponentRenderer->updatePositionalData();
        }
    });
}

void GraphWidget::makeContextCurrent()
{
    _openGLWindow->makeContextCurrent();
}

void GraphWidget::onGraphChanged(const Graph* graph)
{
    // Find default (largest) component
    int maxNumNodes = 0;
    for(auto componentId : graph->componentIds())
    {
        auto component = graph->componentById(componentId);
        if(component->numNodes() > maxNumNodes)
        {
            maxNumNodes = component->numNodes();
            _defaultComponentId = componentId;
        }
    }

    for(auto componentId : graph->componentIds())
    {
        auto graphComponentRenderer = _graphComponentRendererManagers->at(componentId).get();

        executeOnRendererThread([this,  graphComponentRenderer]
        {
            graphComponentRenderer->updateVisualData();
            graphComponentRenderer->updatePositionalData();

            // Graph changes may significantly alter the centre; ease the transition
            if(_initialised && graphComponentRenderer->focusNodeId().isNull())
                graphComponentRenderer->moveFocusToCentreOfComponent(Transition::Type::EaseInEaseOut);
        }, QString("GraphWidget::onGraphChanged (moveFocusToCentreOfComponent) component %1").arg((int)componentId));
    }
}

void GraphWidget::onNodeWillBeRemoved(const Graph* graph, NodeId nodeId)
{
    for(auto componentId : graph->componentIds())
    {
        auto graphComponentRenderer = _graphComponentRendererManagers->at(componentId).get();

        if(graphComponentRenderer->focusNodeId() == nodeId)
        {
            executeOnRendererThread([graphComponentRenderer]
            {
                graphComponentRenderer->moveFocusToCentreOfComponent(Transition::Type::EaseInEaseOut);
            }, "GraphWidget::onNodeWillBeRemoved");
        }
    }
}

void GraphWidget::onComponentAdded(const Graph*, ComponentId componentId)
{
    auto graphComponentRenderer = _graphComponentRendererManagers->at(componentId).get();
    executeOnRendererThread([this, graphComponentRenderer, componentId]
    {
        makeContextCurrent();
        graphComponentRenderer->initialise(_graphModel, componentId, *this,
                                           _selectionManager,
                                           _graphComponentRendererShared);
    }, "GraphWidget::onComponentAdded");
}

void GraphWidget::onComponentWillBeRemoved(const Graph*, ComponentId componentId)
{
    auto graphComponentRenderer = _graphComponentRendererManagers->at(componentId).get();
    executeOnRendererThread([this, graphComponentRenderer]
    {
        makeContextCurrent();
        graphComponentRenderer->cleanup();
    }, QString("GraphWidget::onComponentWillBeRemoved (cleanup) component %1").arg((int)componentId));
}

void GraphWidget::onSelectionChanged(const SelectionManager*)
{
    for(auto componentId : _graphModel->graph().componentIds())
    {
        auto graphComponentRenderer = _graphComponentRendererManagers->at(componentId).get();
        executeOnRendererThread([graphComponentRenderer]
        {
            graphComponentRenderer->updateVisualData();
        }, QString("GraphWidget::onSelectionChanged component %1").arg((int)componentId));
    }
}

void GraphWidget::onCommandWillExecuteAsynchronously(const Command*, const QString&)
{
    _openGLWindow->disableInteraction();
    _sceneUpdateEnabled = false;
}

void GraphWidget::onCommandCompleted(const Command*)
{
    _sceneUpdateEnabled = true;
    _openGLWindow->enableInteraction();
}
