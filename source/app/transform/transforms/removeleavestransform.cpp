#include "removeleavestransform.h"

#include "transform/transformedgraph.h"

#include <memory>
#include <vector>

#include <QObject>

bool RemoveLeavesTransform::apply(TransformedGraph& target) const
{
    target.setPhase(QObject::tr("Leaf Removal"));

    auto limit = static_cast<size_t>(boost::get<int>(config().parameterByName(QStringLiteral("Limit"))->_value));
    bool unlimited = (limit == 0);

    bool changed = false;

    // Hoist this out of the main loop, to avoid the alloc cost between passes
    std::vector<NodeId> removees;

    while(unlimited || limit-- > 0)
    {
        for(auto nodeId : target.nodeIds())
        {
            if(target.nodeById(nodeId).degree() == 1)
                removees.emplace_back(nodeId);
        }

        if(removees.empty())
            break;

        uint64_t progress = 0;
        for(auto nodeId : removees)
        {
            target.mutableGraph().removeNode(nodeId);
            target.setProgress((progress++ * 100) / target.numNodes());
        }
        target.setProgress(-1);

        removees.clear();

        changed = true;

        // Do a manual update so that things are up-to-date for the next pass
        target.update();
    }

    return changed;
}

std::unique_ptr<GraphTransform> RemoveLeavesTransformFactory::create(const GraphTransformConfig&) const
{
    return std::make_unique<RemoveLeavesTransform>();
}
