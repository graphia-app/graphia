/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

#ifndef IGRAPHCOMPONENT_H
#define IGRAPHCOMPONENT_H

#include "elementid.h"
#include "igraph.h"

#include <vector>

class IGraphComponent
{
public:
    virtual ~IGraphComponent() = default;

    virtual const std::vector<NodeId>& nodeIds() const = 0;
    size_t numNodes() const { return nodeIds().size(); }

    virtual const std::vector<EdgeId>& edgeIds() const = 0;
    size_t numEdges() const { return edgeIds().size(); }

    virtual const IGraph& graph() const = 0;
};

#endif // IGRAPHCOMPONENT_H
