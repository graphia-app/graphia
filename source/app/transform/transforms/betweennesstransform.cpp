#include "betweennesstransform.h"
#include "transform/transformedgraph.h"
#include "graph/graphmodel.h"
#include "shared/utils/threadpool.h"

#include <stack>
#include <queue>

void BetweennessTransform::apply(TransformedGraph& target) const
{
    target.setPhase(QStringLiteral("Betweenness"));
    target.setProgress(0);

    const auto& nodeIds = target.nodeIds();
    std::atomic_int progress(0);

    concurrent_for(nodeIds.begin(), nodeIds.end(),
    [this, &progress, &target](const NodeId)
    {
        progress++;
        target.setProgress(progress.load() * 100 / static_cast<int>(target.numNodes()));
    }, true);

    target.setProgress(-1);

    if(cancelled())
        return;

    _graphModel->createAttribute(QObject::tr("Node Betweenness"))
        .setDescription(QObject::tr("A node's betweenness is the number of shortest paths that pass through it."))
        .setIntValueFn([](NodeId) { return 0; })
        .setFlag(AttributeFlag::VisualiseByComponent);
}

std::unique_ptr<GraphTransform> BetweennessTransformFactory::create(const GraphTransformConfig&) const
{
    return std::make_unique<BetweennessTransform>(graphModel());
}

