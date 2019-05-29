#include "spanningtreetransform.h"

#include "transform/transformedgraph.h"

#include "graph/componentmanager.h"
#include "graph/graphcomponent.h"

#include <memory>
#include <deque>

#include <QObject>

void SpanningTreeTransform::apply(TransformedGraph& target) const
{
    bool dfs = config().parameterHasValue(QStringLiteral("Traversal Order"), QStringLiteral("Depth First"));

    target.setPhase(QObject::tr("Spanning Tree"));
    target.setProgress(-1);

    EdgeArray<bool> removees(target, true);
    NodeArray<bool> visitedNodes(target, false);

    ComponentManager componentManager(target);

    for(auto componentId : componentManager.componentIds())
    {
        struct S
        {
            NodeId _nodeId;
            EdgeId _edgeId;
        };

        std::deque<S> deque;
        deque.push_back({componentManager.componentById(componentId)->nodeIds().at(0), {}});

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

    for(const auto& edgeId : target.edgeIds())
    {
        if(removees.get(edgeId))
            target.mutableGraph().removeEdge(edgeId);

        target.setProgress(static_cast<int>((progress++ * 100u) /
            static_cast<uint64_t>(target.numEdges())));
    }

    target.setProgress(-1);
}

std::unique_ptr<GraphTransform> SpanningTreeTransformFactory::create(const GraphTransformConfig&) const
{
    return std::make_unique<SpanningTreeTransform>();
}
