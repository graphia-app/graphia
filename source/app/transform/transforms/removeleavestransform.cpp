#include "removeleavestransform.h"

#include "transform/transformedgraph.h"

#include <memory>

#include <QObject>

bool RemoveLeavesTransform::apply(TransformedGraph& target) const
{
    target.setPhase(QObject::tr("Leaf Removal"));

    auto limit = static_cast<size_t>(boost::get<int>(config().parameterByName(QStringLiteral("Limit"))->_value));
    bool unlimited = (limit == 0);

    bool changed = false;

    while(unlimited || limit-- > 0)
    {
        bool nodesRemoved = false;
        uint64_t progress = 0;
        for(auto nodeId : target.nodeIds())
        {
            if(target.nodeById(nodeId).degree() == 1)
            {
                target.mutableGraph().removeNode(nodeId);
                nodesRemoved = true;
            }

            target.setProgress((progress++ * 100) / target.numNodes());
        }

        if(!nodesRemoved)
            break;

        changed = true;

        target.setProgress(-1);
        target.update();
    }

    return changed;
}

std::unique_ptr<GraphTransform> RemoveLeavesTransformFactory::create(const GraphTransformConfig&) const
{
    return std::make_unique<RemoveLeavesTransform>();
}
