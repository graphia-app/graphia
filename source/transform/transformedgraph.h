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
    TransformedGraph(const MutableGraph& source);

    void setTransform(std::unique_ptr<GraphTransform> graphTransform);

    const std::vector<NodeId>& nodeIds() const { return _target.nodeIds(); }
    int numNodes() const { return _target.numNodes(); }
    const Node& nodeById(NodeId nodeId) const { return _target.nodeById(nodeId); }

    const std::vector<EdgeId>& edgeIds() const { return _target.edgeIds(); }
    int numEdges() const { return _target.numEdges(); }
    const Edge& edgeById(EdgeId edgeId) const { return _target.edgeById(edgeId); }

    NodeId addNode() { return _target.addNode(); }
    NodeId addNode(NodeId nodeId) { return _target.addNode(nodeId); }
    NodeId addNode(const Node& node) { return _target.addNode(node); }
    void addNodes(const ElementIdSet<NodeId>& nodeIds) { addNodes(nodeIds); }

    void removeNode(NodeId nodeId) { _target.removeNode(nodeId); }
    void removeNodes(const ElementIdSet<NodeId>& nodeIds) { _target.removeNodes(nodeIds); }

    EdgeId addEdge(NodeId sourceId, NodeId targetId) { return addEdge(sourceId, targetId); }
    EdgeId addEdge(EdgeId edgeId, NodeId sourceId, NodeId targetId) { return addEdge(edgeId, sourceId, targetId); }
    EdgeId addEdge(const Edge& edge) { return addEdge(edge); }
    void addEdges(const std::vector<Edge>& edges) { addEdges(edges); }

    void removeEdge(EdgeId edgeId) { removeEdge(edgeId); }
    void removeEdges(const ElementIdSet<EdgeId>& edgeIds) { removeEdges(edgeIds); }

    void contractEdge(EdgeId edgeId);

private:
    const MutableGraph* _source;
    std::unique_ptr<GraphTransform> _graphTransform;
    MutableGraph _target;

    class Difference
    {
    private:
        bool _added = false;
        bool _removed = false;
        bool _changed = false;

    public:
        void add()
        {
            _changed = !_added || _removed;
            _added = true;
            _removed = false;
        }

        void remove()
        {
            _changed = _added || !_removed;
            _added = false;
            _removed = true;
        }

        bool added() { return _added && _changed; }
        bool removed() { return _removed && _changed; }
    };

    NodeArray<Difference> _nodesDifference;
    EdgeArray<Difference> _edgesDifference;

    void rebuild();
};

#endif // TRANSFORMEDGRAPH_H
