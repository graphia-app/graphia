#ifndef TRANSFORMEDGRAPH_H
#define TRANSFORMEDGRAPH_H

#include "graphtransform.h"

#include "../graph/graph.h"
#include "../graph/grapharray.h"

#include <QObject>

class TransformedGraph : public Graph
{
    Q_OBJECT

public:
    TransformedGraph(const Graph& source);

    void setTransform(std::unique_ptr<GraphTransform> graphTransform);

    const std::vector<NodeId>& nodeIds() const { return _target.nodeIds(); }
    int numNodes() const { return _target.numNodes(); }
    const Node& nodeById(NodeId nodeId) const { return _target.nodeById(nodeId); }
    bool containsNodeId(NodeId nodeId) const { return _target.containsNodeId(nodeId); }
    MultiNodeId::Type typeOf(NodeId nodeId) const { return _target.typeOf(nodeId); }

    const std::vector<EdgeId>& edgeIds() const { return _target.edgeIds(); }
    int numEdges() const { return _target.numEdges(); }
    const Edge& edgeById(EdgeId edgeId) const { return _target.edgeById(edgeId); }
    bool containsEdgeId(EdgeId edgeId) const { return _target.containsEdgeId(edgeId); }
    MultiEdgeId::Type typeOf(EdgeId edgeId) const { return _target.typeOf(edgeId); }

    NodeId addNode() { return _target.addNode(); }
    NodeId addNode(NodeId nodeId) { return _target.addNode(nodeId); }
    NodeId addNode(const Node& node) { return _target.addNode(node); }
    void addNodes(const NodeIdSet& nodeIds) { addNodes(nodeIds); }

    void removeNode(NodeId nodeId) { _target.removeNode(nodeId); }
    void removeNodes(const NodeIdSet& nodeIds) { _target.removeNodes(nodeIds); }

    EdgeId addEdge(NodeId sourceId, NodeId targetId) { return _target.addEdge(sourceId, targetId); }
    EdgeId addEdge(EdgeId edgeId, NodeId sourceId, NodeId targetId) { return _target.addEdge(edgeId, sourceId, targetId); }
    EdgeId addEdge(const Edge& edge) { return _target.addEdge(edge); }
    void addEdges(const std::vector<Edge>& edges) { _target.addEdges(edges); }

    void removeEdge(EdgeId edgeId) { _target.removeEdge(edgeId); }
    void removeEdges(const EdgeIdSet& edgeIds) { _target.removeEdges(edgeIds); }

    void contractEdge(EdgeId edgeId) { _target.contractEdge(edgeId); }

    void reserve(const Graph& other) { _target.reserve(other); }
    void cloneFrom(const Graph& other) { _target.cloneFrom(other); }

private:
    const Graph* _source;
    std::unique_ptr<GraphTransform> _graphTransform;
    MutableGraph _target;

    class Difference
    {
    private:
        enum class State { Removed, Unchanged, Added };
        State state = State::Unchanged;

    public:
        void add()     { state = state == State::Removed ? State::Unchanged : State::Added; }
        void remove()  { state = state == State::Added ?   State::Unchanged : State::Removed; }

        bool added()   { return state == State::Added; }
        bool removed() { return state == State::Removed; }
    };

    NodeArray<Difference> _nodesDifference;
    EdgeArray<Difference> _edgesDifference;

    void rebuild();

private slots:
    void onTargetGraphChanged(const Graph* graph);
};

#endif // TRANSFORMEDGRAPH_H
