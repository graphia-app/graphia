/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#include "componentlayout.h"

#include "app/graph/graph.h"

void ComponentLayout::execute(const Graph& graph, const std::vector<ComponentId>& componentIds,
                              ComponentLayoutData& componentLayoutData)
{
    componentLayoutData.resetElements();

    executeReal(graph, componentIds, componentLayoutData);

    auto boundingBox = boundingBoxFor(componentIds, componentLayoutData);

    // Normalise layout data to start at 0, 0
    for(auto componentId : componentIds)
        componentLayoutData[componentId].translate(-boundingBox.topLeft());
}

QRectF ComponentLayout::boundingBoxFor(const std::vector<ComponentId>& componentIds,
                                       ComponentLayoutData& componentLayoutData) const
{
    QRectF boundingBox;

    for(auto componentId : componentIds)
        boundingBox = boundingBox.united(componentLayoutData[componentId].boundingBox());

    return boundingBox;
}
