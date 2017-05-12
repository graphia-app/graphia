#ifndef TRANSFORMEDGRAPH_H
#define TRANSFORMEDGRAPH_H

#include "graphtransform.h"
#include "transformcache.h"

#include "graph/graph.h"
#include "graph/mutablegraph.h"
#include "shared/graph/grapharray.h"

#include "attributes/attribute.h"

#include <QObject>

#include <functional>

class GraphModel;
class ICommand;

class TransformedGraph : public Graph
{
    Q_OBJECT

public:
    explicit TransformedGraph(GraphModel& graphModel, const MutableGraph& source);

    void enableAutoRebuild() { _autoRebuild = true; rebuild(); }
    void addTransform(std::unique_ptr<const GraphTransform> t) { _transforms.emplace_back(std::move(t)); }
    void clearTransforms() { _transforms.clear(); }

    void setCommand(ICommand* command) { _command = command; }

    const std::vector<NodeId>& nodeIds() const { return _target.nodeIds(); }
    int numNodes() const { return _target.numNodes(); }
    const INode& nodeById(NodeId nodeId) const { return _target.nodeById(nodeId); }
    bool containsNodeId(NodeId nodeId) const { return _target.containsNodeId(nodeId); }
    MultiElementType typeOf(NodeId nodeId) const { return _target.typeOf(nodeId); }
    ConstNodeIdDistinctSet mergedNodeIdsForNodeId(NodeId nodeId) const { return _target.mergedNodeIdsForNodeId(nodeId); }

    const std::vector<EdgeId>& edgeIds() const { return _target.edgeIds(); }
    int numEdges() const { return _target.numEdges(); }
    const IEdge& edgeById(EdgeId edgeId) const { return _target.edgeById(edgeId); }
    bool containsEdgeId(EdgeId edgeId) const { return _target.containsEdgeId(edgeId); }
    MultiElementType typeOf(EdgeId edgeId) const { return _target.typeOf(edgeId); }
    ConstEdgeIdDistinctSet mergedEdgeIdsForEdgeId(EdgeId edgeId) const { return _target.mergedEdgeIdsForEdgeId(edgeId); }

    EdgeIdDistinctSets edgeIdsForNodeId(NodeId nodeId) const { return _target.edgeIdsForNodeId(nodeId); }

    void setPhase(const QString& phase) const { _source->setPhase(phase); }
    void clearPhase() const { _source->clearPhase(); }
    QString phase() const { return _source->phase(); }

    void setProgress(int progress);

    MutableGraph& mutableGraph() { return _target; }

    void reserve(const Graph& other);
    MutableGraph& operator=(const MutableGraph& other);

    void update() { _target.update(); }

private:
    GraphModel* _graphModel = nullptr;

    const MutableGraph* _source;
    std::vector<std::unique_ptr<const GraphTransform>> _transforms;

    // TransformedGraph has the target as a member rather than inheriting
    // from MutableGraph for two reasons:
    //   1. A TransformedGraph shouldn't be mutable
    //   2. The signals the target emits must be intercepted before being
    //      passed on to other parts of the application
    MutableGraph _target;

    TransformCache _cache;

    bool _graphChangeOccurred = false;
    bool _autoRebuild = false;
    ICommand* _command = nullptr;

    class State
    {
    private:
        enum class Value { Removed, Unchanged, Added };
        Value state = Value::Unchanged;

    public:
        void add()     { state = state == Value::Removed ? Value::Unchanged : Value::Added; }
        void remove()  { state = state == Value::Added ?   Value::Unchanged : Value::Removed; }

        bool added() const   { return state == Value::Added; }
        bool removed() const { return state == Value::Removed; }
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
