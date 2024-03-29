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

#include "removeleavestransform.h"

#include "transform/transformedgraph.h"

#include <memory>
#include <vector>

#include <QObject>

using namespace Qt::Literals::StringLiterals;

static void removeLeaves(TransformedGraph& target, Progressable& progressable, size_t limit = 0)
{
    const bool unlimited = (limit == 0);

    // Hoist this out of the main loop, to avoid the alloc cost between passes
    std::vector<NodeId> removees;

    while(unlimited || limit-- > 0)
    {
        for(auto nodeId : target.nodeIds())
        {
            if(target.nodeById(nodeId).degree() <= 1)
                removees.emplace_back(nodeId);
        }

        if(removees.empty())
            break;

        uint64_t progress = 0;
        for(auto nodeId : removees)
        {
            target.mutableGraph().removeNode(nodeId);
            progressable.setProgress(static_cast<int>((progress++ * 100u) /
                static_cast<uint64_t>(target.numNodes())));
        }
        progressable.setProgress(-1);

        removees.clear();

        // Do a manual update so that things are up-to-date for the next pass
        target.update();
    }
}
void RemoveLeavesTransform::apply(TransformedGraph& target)
{
    setPhase(QObject::tr("Leaf Removal"));

    auto limit = static_cast<size_t>(std::get<int>(config().parameterByName(u"Limit"_s)->_value));
    removeLeaves(target, *this, limit);
}

void RemoveBranchesTransform::apply(TransformedGraph& target)
{
    setPhase(QObject::tr("Branch Removal"));

    removeLeaves(target, *this);
}

std::unique_ptr<GraphTransform> RemoveLeavesTransformFactory::create(const GraphTransformConfig&) const
{
    return std::make_unique<RemoveLeavesTransform>();
}

std::unique_ptr<GraphTransform> RemoveBranchesTransformFactory::create(const GraphTransformConfig&) const
{
    return std::make_unique<RemoveBranchesTransform>();
}
