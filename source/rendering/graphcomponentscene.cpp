#include "graphcomponentscene.h"

#include "graphcomponentrenderer.h"

#include "../graph/graphmodel.h"

#include "../ui/graphwidget.h"

GraphComponentScene::GraphComponentScene(GraphWidget* graphWidget)
    : Scene(graphWidget),
      _graphWidget(graphWidget),
      _width(0), _height(0)
{
    connect(&graphWidget->graphModel()->graph(), &Graph::componentSplit, this, &GraphComponentScene::onComponentSplit, Qt::DirectConnection);
    connect(&graphWidget->graphModel()->graph(), &Graph::componentsWillMerge, this, &GraphComponentScene::onComponentsWillMerge, Qt::DirectConnection);
    connect(&graphWidget->graphModel()->graph(), &Graph::componentWillBeRemoved, this, &GraphComponentScene::onComponentWillBeRemoved, Qt::DirectConnection);
}

void GraphComponentScene::initialise()
{
}

void GraphComponentScene::cleanup()
{
    if(renderer() != nullptr)
        renderer()->cleanup();
}

void GraphComponentScene::update(float t)
{
    if(renderer() != nullptr)
        renderer()->update(t);
}

void GraphComponentScene::render()
{
    if(renderer() != nullptr)
        renderer()->render(0, 0);
}

void GraphComponentScene::resize(int width, int height)
{
    _width = width;
    _height = height;

    if(renderer() != nullptr)
        renderer()->resize(width, height);
}

void GraphComponentScene::setComponentId(ComponentId componentId)
{
    _componentId = componentId;

    for(auto& rendererManager : rendererManagers())
    {
        auto renderer = rendererManager.get();
        renderer->setVisibility(renderer->componentId() == _componentId);
    }
}

void GraphComponentScene::resetView(Transition::Type transitionType)
{
    if(renderer() != nullptr)
        renderer()->resetView(transitionType);
}

bool GraphComponentScene::viewIsReset()
{
    if(renderer() == nullptr)
        return true;

    return renderer()->viewIsReset();
}

GraphComponentRenderer* GraphComponentScene::renderer()
{
    return GraphComponentRenderersReference::renderer(_componentId);
}

void GraphComponentScene::onComponentSplit(const Graph* graph, ComponentId oldComponentId,
                                           const ElementIdSet<ComponentId>& splitters)
{
    if(oldComponentId == _componentId)
    {
        auto oldGraphComponentRenderer = GraphComponentRenderersReference::renderer(oldComponentId);
        auto oldFocusNodeId = oldGraphComponentRenderer->focusNodeId();

        ComponentId newComponentId;

        if(oldFocusNodeId.isNull())
        {
            ComponentId largestSplitter;

            for(auto splitter : splitters)
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

            newComponentId = largestSplitter;
        }
        else
            newComponentId = graph->componentIdOfNode(oldFocusNodeId);

        auto newGraphComponentRenderer = GraphComponentRenderersReference::renderer(newComponentId);
        _graphWidget->executeOnRendererThread([this, newComponentId,
                                              newGraphComponentRenderer,
                                              oldGraphComponentRenderer]
        {
            // Clone the current camera data to the new component
            newGraphComponentRenderer->cloneCameraDataFrom(*oldGraphComponentRenderer);
            setComponentId(newComponentId);
        });
    }
}

void GraphComponentScene::onComponentsWillMerge(const Graph*, const ElementIdSet<ComponentId>&,
                                                ComponentId newComponentId)
{
    auto newGraphComponentRenderer = GraphComponentRenderersReference::renderer(newComponentId);
    auto oldGraphComponentRenderer = GraphComponentRenderersReference::renderer(_componentId);
    _graphWidget->executeOnRendererThread([this, newComponentId,
                                          newGraphComponentRenderer,
                                          oldGraphComponentRenderer]
    {
        newGraphComponentRenderer->cloneCameraDataFrom(*oldGraphComponentRenderer);
        setComponentId(newComponentId);
    });
}

void GraphComponentScene::onComponentWillBeRemoved(const Graph*, ComponentId componentId)
{
    _graphWidget->executeOnRendererThread([this, componentId]
    {
        if(componentId == _componentId)
            _graphWidget->switchToOverviewMode();
    });
}
