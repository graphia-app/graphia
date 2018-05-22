#include "spanningtreetransform.h"

#include "transform/transformedgraph.h"

#include <memory>
#include <stack>

#include <QObject>

bool SpanningTreeTransform::apply(TransformedGraph& target) const
{
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

        std::stack<S> stack;
        stack.push({target.componentById(componentId)->nodeIds().at(0), {}});

        while(!stack.empty())
        {
            auto route = stack.top();
            auto nodeId = route._nodeId;
            auto traversedEdgeId = route._edgeId;
            stack.pop();

            if(visitedNodes.get(nodeId))
                continue;

            visitedNodes.set(nodeId, true);

            if(!traversedEdgeId.isNull())
                removees.set(traversedEdgeId, false);

            for(auto edgeId : target.nodeById(nodeId).edgeIds())
            {
                auto oppositeId = target.edgeById(edgeId).oppositeId(nodeId);

                if(!visitedNodes.get(oppositeId))
                    stack.push({oppositeId, edgeId});
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
