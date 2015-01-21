#include "graphscene.h"

#include "../graph/graphmodel.h"

#include "../ui/graphwidget.h"

#include "../utils/utils.h"

GraphScene::GraphScene(GraphWidget* graphWidget)
    : Scene(graphWidget),
      _graphWidget(graphWidget),
      _renderSizeDivisors(graphWidget->graphModel()->graph())
{
    connect(&graphWidget->graphModel()->graph(), &Graph::graphChanged, this, &GraphScene::onGraphChanged, Qt::DirectConnection);
}

void GraphScene::initialise()
{
    onGraphChanged(&_graphWidget->graphModel()->graph());
}

void GraphScene::cleanup()
{
}

void GraphScene::update(float /*t*/)
{
}

void GraphScene::render()
{
}

void GraphScene::resize(int /*w*/, int /*h*/)
{
}

void GraphScene::onGraphChanged(const Graph* graph)
{
    // Find the number of nodes in the largest component
    int maxNumNodes = 0;
    for(auto componentId : graph->componentIds())
    {
        auto component = graph->componentById(componentId);
        if(component->numNodes() > maxNumNodes)
            maxNumNodes = component->numNodes();
    }

    for(auto componentId : graph->componentIds())
    {
        auto component = graph->componentById(componentId);
        int divisor = maxNumNodes / component->numNodes();
        _renderSizeDivisors[componentId] = Utils::smallestPowerOf2GreaterThan(divisor);
    }
}
