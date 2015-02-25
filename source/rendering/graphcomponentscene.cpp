#include "graphcomponentscene.h"

#include "graphcomponentrenderer.h"

#include "../graph/graphmodel.h"

#include "../ui/graphwidget.h"

#include <QOpenGLContext>
#include <QOpenGLFunctions_3_3_Core>

GraphComponentScene::GraphComponentScene(GraphWidget* graphWidget)
    : Scene(graphWidget),
      _graphWidget(graphWidget),
      _width(0), _height(0)
{
    connect(&graphWidget->graphModel()->graph(), &Graph::componentSplit, this, &GraphComponentScene::onComponentSplit, Qt::DirectConnection);
    connect(&graphWidget->graphModel()->graph(), &Graph::componentsWillMerge, this, &GraphComponentScene::onComponentsWillMerge, Qt::DirectConnection);
    connect(&graphWidget->graphModel()->graph(), &Graph::componentWillBeRemoved, this, &GraphComponentScene::onComponentWillBeRemoved, Qt::DirectConnection);
    connect(&graphWidget->graphModel()->graph(), &Graph::graphChanged, this, &GraphComponentScene::onGraphChanged, Qt::DirectConnection);
    connect(&graphWidget->graphModel()->graph(), &Graph::nodeWillBeRemoved, this, &GraphComponentScene::onNodeWillBeRemoved, Qt::DirectConnection);
}

void GraphComponentScene::initialise()
{
    _funcs = context().versionFunctions<QOpenGLFunctions_3_3_Core>();
    if(!_funcs)
        qFatal("Could not obtain required OpenGL context version");
    _funcs->initializeOpenGLFunctions();
}

void GraphComponentScene::update(float t)
{
    _graphWidget->updateNodePositions();
    _graphWidget->transition().update(t);

    if(renderer() != nullptr)
        renderer()->update(t);
}

void GraphComponentScene::render()
{
    _funcs->glBindFramebuffer(GL_FRAMEBUFFER, 0);

    _funcs->glClearColor(0.75f, 0.75f, 0.75f, 1.0f);
    _funcs->glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    _graphWidget->clearScene();

    if(renderer() != nullptr)
        renderer()->render(0, 0);

    _graphWidget->renderScene();
}

void GraphComponentScene::resize(int width, int height)
{
    _width = width;
    _height = height;

    _graphWidget->resizeScene(width, height);

    if(renderer() != nullptr)
        renderer()->resize(width, height);
}

void GraphComponentScene::onShow()
{
    if(visible())
    {
        for(auto& rendererManager : rendererManagers())
        {
            auto renderer = rendererManager.get();
            renderer->setVisible(renderer->componentId() == _componentId);
        }
    }
}

void GraphComponentScene::setComponentId(ComponentId componentId)
{
    _componentId = componentId;
    onShow();
}

void GraphComponentScene::saveViewData()
{
    if(renderer() != nullptr)
        renderer()->saveViewData();
}

void GraphComponentScene::restoreViewData()
{
    if(renderer() != nullptr)
        renderer()->restoreViewData();
}

void GraphComponentScene::resetView()
{
    if(renderer() != nullptr)
    {
        startTransition();
        renderer()->resetView();
    }
}

bool GraphComponentScene::viewIsReset()
{
    if(renderer() == nullptr)
        return true;

    return renderer()->viewIsReset();
}

GraphComponentRenderer* GraphComponentScene::renderer()
{
    return rendererForComponentId(_componentId);
}

void GraphComponentScene::startTransition(Transition::Type transitionType, float duration)
{
    if(_graphWidget->transition().finished())
        _graphWidget->rendererStartedTransition();

    _graphWidget->transition().start(duration, transitionType,
    [this](float f)
    {
        renderer()->updateTransition(f);
    },
    [this]
    {
        _graphWidget->rendererFinishedTransition();
    });
}

void GraphComponentScene::onComponentSplit(const Graph* graph, const ComponentSplitSet& componentSplitSet)
{
    if(!visible())
        return;

    auto oldComponentId = componentSplitSet.oldComponentId();
    if(oldComponentId == _componentId)
    {
        ComponentId largestSplitter;

        for(auto splitter : componentSplitSet.splitters())
        {

            if(largestSplitter.isNull())
                largestSplitter = splitter;
            else
            {
                auto splitterNumNodes = graph->componentById(splitter)->numNodes();
                auto largestNumNodes = graph->componentById(largestSplitter)->numNodes();

                if(splitterNumNodes > largestNumNodes)
                    largestSplitter = splitter;
            }
        }

        auto oldGraphComponentRenderer = rendererForComponentId(oldComponentId);

        _graphWidget->executeOnRendererThread([this, largestSplitter,
                                              oldGraphComponentRenderer]
        {
            auto& graph = _graphWidget->graphModel()->graph();

            ComponentId newComponentId;

            if(oldGraphComponentRenderer->trackingCentreOfComponent())
                newComponentId = largestSplitter;
            else
                newComponentId = graph.componentIdOfNode(oldGraphComponentRenderer->focusNodeId());

            Q_ASSERT(!newComponentId.isNull());

            auto newGraphComponentRenderer = rendererForComponentId(newComponentId);

            newGraphComponentRenderer->cloneViewDataFrom(*oldGraphComponentRenderer);
            setComponentId(newComponentId);
        }, "GraphComponentScene::onComponentSplit (clone camera data, set component ID)");
    }
}

void GraphComponentScene::onComponentsWillMerge(const Graph*, const ComponentMergeSet& componentMergeSet)
{
    if(!visible())
        return;

    for(auto merger : componentMergeSet.mergers())
    {
        if(merger == _componentId)
        {
            auto newComponentId = componentMergeSet.newComponentId();
            auto newGraphComponentRenderer = rendererForComponentId(newComponentId);
            auto oldGraphComponentRenderer = rendererForComponentId(_componentId);
            _graphWidget->executeOnRendererThread([this, newComponentId,
                                                  newGraphComponentRenderer,
                                                  oldGraphComponentRenderer]
            {
                newGraphComponentRenderer->cloneViewDataFrom(*oldGraphComponentRenderer);
                setComponentId(newComponentId);
            }, "GraphComponentScene::onComponentsWillMerge (clone camera data, set component ID)");
            break;
        }
    }
}

void GraphComponentScene::onComponentWillBeRemoved(const Graph*, ComponentId componentId, bool hasMerged)
{
    if(visible() && componentId == _componentId && !hasMerged)
        _graphWidget->switchToOverviewMode();
}

void GraphComponentScene::onGraphChanged(const Graph*)
{  
    _graphWidget->executeOnRendererThread([this]
    {
        if(visible())
        {
            resize(_width, _height);

            // Graph changes may significantly alter the centre; ease the transition
            if(renderer()->trackingCentreOfComponent())
            {
                startTransition();
                renderer()->moveFocusToCentreOfComponent();
            }
        }
    }, "GraphComponentScene::onGraphChanged (resize/moveFocusToCentreOfComponent)");
}

void GraphComponentScene::onNodeWillBeRemoved(const Graph*, NodeId nodeId)
{
    if(renderer()->focusNodeId() == nodeId)
    {
        _graphWidget->executeOnRendererThread([this]
        {
            startTransition();
            renderer()->moveFocusToCentreOfComponent();
        }, "GraphWidget::onNodeWillBeRemoved");
    }
}
