#include "powerof2gridcomponentlayout.h"

#include "../graph/componentmanager.h"

#include <stack>
#include <vector>
#include <algorithm>

#include <QPointF>

void PowerOf2GridComponentLayout::executeReal(const Graph& graph, const std::vector<ComponentId> &componentIds,
                                              ComponentLayoutData& componentLayoutData)
{
    if(graph.numComponents() == 0)
        return;

    // Find the number of nodes in the largest component
    auto largestComponentId = graph.componentIdOfLargestComponent();
    int maxNumNodes = graph.componentById(largestComponentId)->numNodes();

    ComponentArray<int> renderSizeDivisors(graph);

    for(auto componentId : componentIds)
    {
        auto component = graph.componentById(componentId);
        int divisor = maxNumNodes / component->numNodes();
        renderSizeDivisors[componentId] = u::smallestPowerOf2GreaterThan(divisor);
    }

    std::vector<ComponentId> sortedComponentIds = componentIds;
    std::stable_sort(sortedComponentIds.begin(), sortedComponentIds.end(),
              [&renderSizeDivisors](const ComponentId& a, const ComponentId& b)
    {
        return renderSizeDivisors[a] < renderSizeDivisors[b];
    });

    std::stack<QPointF> coords;
    coords.emplace(0, 0);
    for(auto componentId : sortedComponentIds)
    {
        auto coord = coords.top();
        coords.pop();

        const int MAX_SIZE = 1024;
        const int MINIMUM_SIZE = 32;
        int divisor = renderSizeDivisors[componentId];
        int dividedSize = MAX_SIZE / (divisor * 2);

        while(dividedSize < MINIMUM_SIZE && divisor > 1)
        {
            divisor /= 2;
            dividedSize = MAX_SIZE / (divisor * 2);
        }

        if(!coords.empty() && (coord.x() + dividedSize > coords.top().x() ||
            coord.y() + dividedSize > MAX_SIZE))
        {
            coord = coords.top();
            coords.pop();
        }

        float radius = dividedSize * 0.5f;
        componentLayoutData[componentId].set(coord.x() + radius, coord.y() + radius, radius);

        QPointF right(coord.x() + dividedSize, coord.y());
        QPointF down(coord.x(), coord.y() + dividedSize);

        if(coords.empty() || right.x() < coords.top().x())
            coords.emplace(right);

        if(down.y() < MAX_SIZE)
            coords.emplace(down);
    }
}
