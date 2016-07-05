#ifndef IMUTABLEGRAPH_H
#define IMUTABLEGRAPH_H

#include "shared/graph/elementid.h"
#include "shared/graph/igraph.h"

class IMutableGraph : public virtual IGraph
{
public:
    virtual ~IMutableGraph() = default;

    virtual NodeId addNode() = 0;
    virtual NodeId addNode(NodeId nodeId) = 0;
    virtual NodeId addNode(const INode& node) = 0;
    virtual void removeNode(NodeId nodeId) = 0;

    virtual EdgeId addEdge(NodeId sourceId, NodeId targetId) = 0;
    virtual EdgeId addEdge(EdgeId edgeId, NodeId sourceId, NodeId targetId) = 0;
    virtual EdgeId addEdge(const IEdge& edge) = 0;
    virtual void removeEdge(EdgeId edgeId) = 0;

    virtual void contractEdge(EdgeId edgeId) = 0;
    virtual void contractEdges(const EdgeIdSet& edgeIds) = 0;
};

#endif // IMUTABLEGRAPH_H
