#ifndef TRANSFORMEDGRAPH_H
#define TRANSFORMEDGRAPH_H

#include "graphtransform.h"

#include "../graph/graph.h"
#include "../graph/mutablegraph.h"
#include "../graph/grapharray.h"

#include <QObject>

#include <functional>

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
    NodeIdDistinctSetCollection::Type typeOf(NodeId nodeId) const { return _target.typeOf(nodeId); }
    NodeIdDistinctSet mergedNodeIdsForNodeId(NodeId nodeId) const { return _target.mergedNodeIdsForNodeId(nodeId); }

    const std::vector<EdgeId>& edgeIds() const { return _target.edgeIds(); }
    int numEdges() const { return _target.numEdges(); }
    const Edge& edgeById(EdgeId edgeId) const { return _target.edgeById(edgeId); }
    bool containsEdgeId(EdgeId edgeId) const { return _target.containsEdgeId(edgeId); }
    EdgeIdDistinctSetCollection::Type typeOf(EdgeId edgeId) const { return _target.typeOf(edgeId); }
    EdgeIdDistinctSet mergedEdgeIdsForEdgeId(EdgeId edgeId) const { return _target.mergedEdgeIdsForEdgeId(edgeId); }

    EdgeIdDistinctSet edgeIdsForNodeId(NodeId nodeId) const { return _target.edgeIdsForNodeId(nodeId); }

    NodeId addNode() { return _target.addNode(); }
    NodeId addNode(NodeId nodeId) { return _target.addNode(nodeId); }
    NodeId addNode(const Node& node) { return _target.addNode(node); }
    template<typename C> void addNodes(const C& nodeIds) { addNodes(nodeIds); }

    void removeNode(NodeId nodeId) { _target.removeNode(nodeId); }
    template<typename C> void removeNodes(const C& nodeIds) { _target.removeNodes(nodeIds); }

    EdgeId addEdge(NodeId sourceId, NodeId targetId) { return _target.addEdge(sourceId, targetId); }
    EdgeId addEdge(EdgeId edgeId, NodeId sourceId, NodeId targetId) { return _target.addEdge(edgeId, sourceId, targetId); }
    EdgeId addEdge(const Edge& edge) { return _target.addEdge(edge); }
    template<typename C> void addEdges(const C& edges) { _target.addEdges(edges); }

    void removeEdge(EdgeId edgeId) { _target.removeEdge(edgeId); }
    template<typename C> void removeEdges(const C& edgeIds) { _target.removeEdges(edgeIds); }

    void contractEdge(EdgeId edgeId) { _target.contractEdge(edgeId); }
    template<typename C> void contractEdges(const C& edgeIds) { _target.contractEdges(edgeIds); }

    void reserve(const Graph& other) { _target.reserve(other); }
    void cloneFrom(const Graph& other) { _target.cloneFrom(other); }

    void update() { _target.update(); }

private:
    const Graph* _source;
    std::unique_ptr<GraphTransform> _graphTransform;
    MutableGraph _target;

    class State
    {
    private:
        enum class Value { Removed, Unchanged, Added };
        Value state = Value::Unchanged;

    public:
        void add()     { state = state == Value::Removed ? Value::Unchanged : Value::Added; }
        void remove()  { state = state == Value::Added ?   Value::Unchanged : Value::Removed; }

        bool added()   { return state == Value::Added; }
        bool removed() { return state == Value::Removed; }
    };

    NodeArray<State> _nodesState;
    EdgeArray<State> _edgesState;
    NodeArray<State> _previousNodesState;
    EdgeArray<State> _previousEdgesState;

    void rebuild();

private slots:
    void onTargetGraphChanged(const Graph* graph);
};

#endif // TRANSFORMEDGRAPH_H
