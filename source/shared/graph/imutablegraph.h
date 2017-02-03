#ifndef IMUTABLEGRAPH_H
#define IMUTABLEGRAPH_H

#include "shared/graph/elementid.h"
#include "shared/graph/igraph.h"

#include <functional>

class IMutableGraph : public virtual IGraph
{
public:
    virtual ~IMutableGraph() = default;

    virtual void clear() = 0;

    virtual NodeId addNode() = 0;
    virtual NodeId addNode(NodeId nodeId) = 0;
    virtual NodeId addNode(const INode& node) = 0;
    template<typename C> void addNodes(const C& nodeIds)
    {
        if(nodeIds.empty())
            return;

        beginTransaction();

        for(auto nodeId : nodeIds)
            addNode(nodeId);

        endTransaction();
    }

    virtual void removeNode(NodeId nodeId) = 0;
    template<typename C> void removeNodes(const C& nodeIds)
    {
        if(nodeIds.empty())
            return;

        beginTransaction();

        for(auto nodeId : nodeIds)
            removeNode(nodeId);

        endTransaction();
    }

    virtual EdgeId addEdge(NodeId sourceId, NodeId targetId) = 0;
    virtual EdgeId addEdge(EdgeId edgeId, NodeId sourceId, NodeId targetId) = 0;
    virtual EdgeId addEdge(const IEdge& edge) = 0;
    template<typename C> void addEdges(const C& edges)
    {
        if(edges.empty())
            return;

        beginTransaction();

        for(const auto& edge : edges)
            addEdge(edge);

        endTransaction();
    }

    virtual void removeEdge(EdgeId edgeId) = 0;
    template<typename C> void removeEdges(const C& edgeIds)
    {
        if(edgeIds.empty())
            return;

        beginTransaction();

        for(auto edgeId : edgeIds)
            removeEdge(edgeId);

        endTransaction();
    }

    virtual void contractEdge(EdgeId edgeId) = 0;
    virtual void contractEdges(const EdgeIdSet& edgeIds) = 0;

protected:
    virtual void beginTransaction() = 0;
    virtual void endTransaction() = 0;

public:
    class ScopedTransaction
    {
    public:
        explicit ScopedTransaction(IMutableGraph& graph) : _graph(&graph)
        { _graph->beginTransaction(); }
        ~ScopedTransaction() { _graph->endTransaction(); }

    private:
        IMutableGraph* _graph;
    };

    void performTransaction(std::function<void(IMutableGraph& graph)> transaction)
    {
        ScopedTransaction lock(*this);
        transaction(*this);
    }
};

#endif // IMUTABLEGRAPH_H
