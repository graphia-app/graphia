#include "graphwidget.h"

#include "../rendering/openglwindow.h"
#include "../rendering/graphoverviewscene.h"
#include "../rendering/graphcomponentscene.h"

#include "../graph/graphmodel.h"
#include "../graph/grapharray.h"
#include "../rendering/graphcomponentrenderer.h"

#include "../utils/make_unique.h"
#include "../utils/utils.h"

#include "graphoverviewinteractor.h"
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
    _graphRenderer(std::make_shared<GraphRenderer>(this, _openGLWindow->context())),
    _graphComponentRendererManagers(std::make_shared<ComponentArray<GraphComponentRendererManager>>(graphModel->graph())),
    _numTransitioningRenderers(0),
    _modeChanged(false),
    _mode(GraphWidget::Mode::Component)
{
    connect(&graphModel->graph(), &Graph::graphChanged, this, &GraphWidget::onGraphChanged, Qt::DirectConnection);
    connect(&graphModel->graph(), &Graph::componentAdded, this, &GraphWidget::onComponentAdded, Qt::DirectConnection);
    connect(&graphModel->graph(), &Graph::componentWillBeRemoved, this, &GraphWidget::onComponentWillBeRemoved, Qt::DirectConnection);

    _graphOverviewScene = new GraphOverviewScene(this);
    _graphOverviewScene->setGraphComponentRendererManagers(_graphComponentRendererManagers);
    _graphOverviewInteractor = new GraphOverviewInteractor(graphModel, _graphOverviewScene,
                                                           commandManager, selectionManager, this);

    _graphComponentScene = new GraphComponentScene(this);
    _graphComponentScene->setGraphComponentRendererManagers(_graphComponentRendererManagers);
    _graphComponentInteractor = new GraphComponentInteractor(graphModel, _graphComponentScene,
                                                             commandManager, selectionManager, this);

    connect(selectionManager.get(), &SelectionManager::selectionChanged, this, &GraphWidget::onSelectionChanged, Qt::DirectConnection);

    connect(_graphOverviewInteractor, &Interactor::userInteractionStarted, this, &GraphWidget::userInteractionStarted);
    connect(_graphOverviewInteractor, &Interactor::userInteractionFinished, this, &GraphWidget::userInteractionFinished);
    connect(_graphComponentInteractor, &Interactor::userInteractionStarted, this, &GraphWidget::userInteractionStarted);
    connect(_graphComponentInteractor, &Interactor::userInteractionFinished, this, &GraphWidget::userInteractionFinished);

    setLayout(new QVBoxLayout());
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->addWidget(QWidget::createWindowContainer(_openGLWindow));

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
        onComponentAdded(graph, componentId, false);

    onGraphChanged(graph);

    switchToComponentMode();

    _initialised = true;
}

void GraphWidget::onUpdate()
{
    if(_sceneUpdateEnabled)
    {
        if(_modeChanged)
        {
            switch(_mode)
            {
                case GraphWidget::Mode::Overview:
                    finishTransitionToOverviewMode();
                    break;

                case GraphWidget::Mode::Component:
                    finishTransitionToComponentMode();
                    break;

                default:
                    break;
            }

            _modeChanged = false;
        }

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
    {
        _graphComponentScene->startTransition();
        _graphComponentScene->resetView();
    }
}

bool GraphWidget::viewIsReset() const
{
    if(_mode == GraphWidget::Mode::Component)
        return _graphComponentScene->viewIsReset();

    return true;
}

void GraphWidget::switchToOverviewMode()
{
    executeOnRendererThread([this]
    {
        // So that we can return to the current view parameters later
        _graphComponentScene->saveViewData();

        if(_mode != GraphWidget::Mode::Overview)
        {
            if(!_graphComponentScene->viewIsReset())
            {
                if(_transition.finished())
                    rendererStartedTransition();

                _graphComponentScene->startTransition(0.3f, Transition::Type::EaseInEaseOut,
                [this]
                {
                    _modeChanged = true;
                    _mode = GraphWidget::Mode::Overview;
                    rendererFinishedTransition();
                });

                _graphComponentScene->resetView();
            }
            else
            {
                _modeChanged = true;
                _mode = GraphWidget::Mode::Overview;
            }
        }
        else
            finishTransitionToOverviewMode();
    }, "GraphWidget::switchToOverviewMode");
}

void GraphWidget::finishTransitionToOverviewMode()
{
    _openGLWindow->setScene(_graphOverviewScene);
    _openGLWindow->setInteractor(_graphOverviewInteractor);

    if(_modeChanged)
    {
        // When we first change to overview mode we want all
        // the renderers to be in their reset state
        for(auto componentId : _graphModel->graph().componentIds())
        {
            auto renderer = _graphComponentRendererManagers->at(componentId).get();
            renderer->resetView();
        }

        _graphOverviewScene->startTransitionFromComponentMode(_graphComponentScene->componentId(),
                                                              0.3f, Transition::Type::EaseInEaseOut);
    }
}

void GraphWidget::switchToComponentMode(ComponentId componentId)
{
    executeOnRendererThread([this, componentId]
    {
        if(componentId.isNull())
            _graphComponentScene->setComponentId(_defaultComponentId);
        else
            _graphComponentScene->setComponentId(componentId);

        if(_mode != GraphWidget::Mode::Component)
        {
            if(_transition.finished())
                rendererStartedTransition();

            _graphOverviewScene->startTransitionToComponentMode(_graphComponentScene->componentId(),
                                                                0.3f, Transition::Type::EaseInEaseOut,
            [this]
            {
                _modeChanged = true;
                _mode = GraphWidget::Mode::Component;
                rendererFinishedTransition();
            });
        }
        else
            finishTransitionToComponentMode();

    }, "GraphWidget::switchToComponentMode");
}

void GraphWidget::finishTransitionToComponentMode()
{
    _openGLWindow->setScene(_graphComponentScene);
    _openGLWindow->setInteractor(_graphComponentInteractor);

    if(!_graphComponentScene->savedViewIsReset())
    {
        // Go back to where we were before
        if(_modeChanged)
            _graphComponentScene->startTransition(0.3f, Transition::Type::EaseInEaseOut);

        _graphComponentScene->restoreViewData();
    }
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

void GraphWidget::setSelectionRect(const QRect& rect)
{
    _graphRenderer->setSelectionRect(rect);
}

void GraphWidget::clearSelectionRect()
{
    _graphRenderer->clearSelectionRect();
}

void GraphWidget::resizeScene(int width, int height)
{
    _graphRenderer->resize(width, height);
}

void GraphWidget::clearScene()
{
    _graphRenderer->clear();
}

void GraphWidget::renderScene()
{
    _graphRenderer->render();
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
        //FIXME: this makes me feel dirty
        // This is a slight hack to prevent there being a gap in which
        // layout can occur, inbetween the graph change and user
        // interaction phases
        rendererStartedTransition();

        executeOnRendererThread([this, componentId]
        {
            auto graphComponentRenderer = _graphComponentRendererManagers->at(componentId).get();

            makeContextCurrent();
            if(!graphComponentRenderer->initialised())
            {
                graphComponentRenderer->initialise(_graphModel, componentId, *this,
                                                   _selectionManager,
                                                   _graphRenderer);
            }
            else
            {
                graphComponentRenderer->updateVisualData();
                graphComponentRenderer->updatePositionalData();
            }

            // Partner to the hack described above
            rendererFinishedTransition();
        }, QString("GraphWidget::onGraphChanged (initialise/update) component %1").arg((int)componentId));
    }
}

void GraphWidget::onComponentAdded(const Graph*, ComponentId componentId, bool)
{
    auto graphComponentRenderer = _graphComponentRendererManagers->at(componentId).get();
    executeOnRendererThread([this, graphComponentRenderer, componentId]
    {
        makeContextCurrent();
        graphComponentRenderer->initialise(_graphModel, componentId, *this,
                                           _selectionManager,
                                           _graphRenderer);
    }, "GraphWidget::onComponentAdded");
}

void GraphWidget::onComponentWillBeRemoved(const Graph*, ComponentId componentId, bool)
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
