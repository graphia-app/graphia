#include "spanningtreetransform.h"

#include "transform/transformedgraph.h"

#include <memory>
#include <deque>

#include <QObject>

bool SpanningTreeTransform::apply(TransformedGraph& target) const
{
    bool dfs = config().parameterHasValue(QStringLiteral("Traversal Order"), QStringLiteral("Depth First"));

    target.setPhase(QObject::tr("Spanning Tree"));
    target.setProgress(-1);

    EdgeArray<bool> removees(target, true);
    NodeArray<bool> visitedNodes(target, false);

    for(auto componentId : target.componentIds())
    {
        struct S
        {
            NodeId _nodeId;
            EdgeId _edgeId;
        };

        std::deque<S> deque;
        deque.push_back({target.componentById(componentId)->nodeIds().at(0), {}});

        while(!deque.empty())
        {
            S route;

            if(dfs)
            {
                route = deque.back();
                deque.pop_back();
            }
            else
            {
                route = deque.front();
                deque.pop_front();
            }

            auto nodeId = route._nodeId;
            auto traversedEdgeId = route._edgeId;

            if(visitedNodes.get(nodeId))
                continue;

            visitedNodes.set(nodeId, true);

            if(!traversedEdgeId.isNull())
                removees.set(traversedEdgeId, false);

            for(auto edgeId : target.nodeById(nodeId).edgeIds())
            {
                auto oppositeId = target.edgeById(edgeId).oppositeId(nodeId);

                if(!visitedNodes.get(oppositeId))
                    deque.push_back({oppositeId, edgeId});
            }
        }
    }

    uint64_t progress = 0;

    bool anyEdgeRemoved = false;
    for(const auto& edgeId : target.edgeIds())
    {
        if(removees.get(edgeId))
        {
            target.mutableGraph().removeEdge(edgeId);
            anyEdgeRemoved = true;
        }

        target.setProgress((progress++ * 100) / target.numEdges());
    }

    target.setProgress(-1);

    return anyEdgeRemoved;
}

std::unique_ptr<GraphTransform> SpanningTreeTransformFactory::create(const GraphTransformConfig&) const
{
    return std::make_unique<SpanningTreeTransform>();
}
