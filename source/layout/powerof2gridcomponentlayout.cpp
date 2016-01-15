#include "powerof2gridcomponentlayout.h"

#include "../graph/componentmanager.h"

#include <vector>
#include <stack>
#include <algorithm>

#include <QPointF>

void PowerOf2GridComponentLayout::execute(const Graph& graph, const std::vector<ComponentId> &componentIds,
                                          int, int height, ComponentLayoutData& componentLayoutData)
{
    std::stack<QPointF> coords;

    // Find the number of nodes in the largest component
    int maxNumNodes = 0;

    if(graph.numComponents() > 0)
    {
        auto largestComponentId = graph.componentIdOfLargestComponent();
        maxNumNodes = graph.componentById(largestComponentId)->numNodes();
    }

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

    //FIXME this is a mess
    coords.emplace(0, 0);
    for(auto componentId : sortedComponentIds)
    {
        auto coord = coords.top();
        coords.pop();

        int divisor = renderSizeDivisors[componentId];
        int dividedSize = height / (divisor * 2);

        const int MINIMUM_SIZE = 32;
        if(height > MINIMUM_SIZE)
        {
            while(dividedSize < MINIMUM_SIZE && divisor > 1)
            {
                divisor /= 2;
                dividedSize = height / (divisor * 2);
            }
        }

        if(!coords.empty() && (coord.x() + dividedSize > coords.top().x() ||
            coord.y() + dividedSize > height))
        {
            coord = coords.top();
            coords.pop();
        }

        auto rect = QRectF(coord.x(), coord.y(), dividedSize, dividedSize);
        componentLayoutData[componentId] = rect;

        QPointF right(coord.x() + dividedSize, coord.y());
        QPointF down(coord.x(), coord.y() + dividedSize);

        if(coords.empty() || right.x() < coords.top().x())
            coords.emplace(right);

        if(down.y() < height)
            coords.emplace(down);
    }
}
