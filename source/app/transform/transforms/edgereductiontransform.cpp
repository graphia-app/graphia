#include "edgereductiontransform.h"

#include "transform/transformedgraph.h"

#include <memory>
#include <random>

#include <QObject>

void EdgeReductionTransform::apply(TransformedGraph& target) const
{
    target.setPhase(QObject::tr("Edge Reduction"));

    auto percentage = static_cast<size_t>(std::get<int>(config().parameterByName(QStringLiteral("Percentage"))->_value));
    auto minimum = static_cast<size_t>(std::get<int>(config().parameterByName(QStringLiteral("Minimum"))->_value));

    EdgeArray<bool> removees(target, true);

    uint64_t progress = 0;
    for(auto nodeId : target.nodeIds())
    {
        auto edgeIds = target.nodeById(nodeId).edgeIds();

        if(edgeIds.empty())
            continue;

        std::mt19937 generator(static_cast<int>(nodeId));
        std::uniform_int_distribution<size_t> distribution(0, edgeIds.size() - 1);

        size_t numEdgesToRetain = std::max(minimum, (edgeIds.size() * percentage) / 100);

        for(size_t i = 0u; i < numEdgesToRetain; i++)
        {
            auto index = distribution(generator);
            removees.set(edgeIds[index], false);
        }

        target.setProgress(static_cast<int>((progress++ * 100u) /
            static_cast<uint64_t>(target.numNodes())));
    }

    progress = 0;

    for(const auto& edgeId : target.edgeIds())
    {
        if(removees.get(edgeId))
            target.mutableGraph().removeEdge(edgeId);

        target.setProgress(static_cast<int>((progress++ * 100u) /
            static_cast<uint64_t>(target.numEdges())));
    }

    target.setProgress(-1);
}

std::unique_ptr<GraphTransform> EdgeReductionTransformFactory::create(const GraphTransformConfig&) const
{
    return std::make_unique<EdgeReductionTransform>();
}
