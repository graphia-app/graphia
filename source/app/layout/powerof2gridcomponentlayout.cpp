/* Copyright © 2013-2022 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "powerof2gridcomponentlayout.h"

#include "graph/graph.h"
#include "graph/graphcomponent.h"
#include "graph/componentmanager.h"
#include "shared/utils/utils.h"

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
    size_t maxNumNodes = graph.componentById(largestComponentId)->numNodes();

    ComponentArray<size_t> renderSizeDivisors(graph);

    for(auto componentId : componentIds)
    {
        const auto* component = graph.componentById(componentId);
        size_t divisor = maxNumNodes / component->numNodes();
        renderSizeDivisors[componentId] = static_cast<size_t>(
            u::smallestPowerOf2GreaterThan(static_cast<int>(divisor)));
    }

    auto sortedComponentIds = componentIds;
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

        const size_t MAX_SIZE = 1024;
        const size_t MINIMUM_SIZE = 32;
        size_t divisor = renderSizeDivisors[componentId];
        size_t dividedSize = MAX_SIZE / (divisor * 2);

        while(dividedSize < MINIMUM_SIZE && divisor > 1)
        {
            divisor /= 2;
            dividedSize = MAX_SIZE / (divisor * 2);
        }

        if(!coords.empty() && (coord.x() + static_cast<qreal>(dividedSize) > coords.top().x() ||
            coord.y() + static_cast<qreal>(dividedSize) > MAX_SIZE))
        {
            coord = coords.top();
            coords.pop();
        }

        float radius = static_cast<float>(dividedSize) * 0.5f;
        componentLayoutData[componentId].set(static_cast<float>(coord.x()) + radius,
                                             static_cast<float>(coord.y()) + radius, radius);

        QPointF right(coord.x() + static_cast<qreal>(dividedSize), coord.y());
        QPointF down(coord.x(), coord.y() + static_cast<qreal>(dividedSize));

        if(coords.empty() || right.x() < coords.top().x())
            coords.emplace(right);

        if(down.y() < MAX_SIZE)
            coords.emplace(down);
    }
}
