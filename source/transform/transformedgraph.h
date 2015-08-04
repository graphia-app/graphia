#ifndef TRANSFORMEDGRAPH_H
#define TRANSFORMEDGRAPH_H

#include "../graph/graph.h"

#include "graphtransform.h"

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

    const std::vector<EdgeId>& edgeIds() const { return _target.edgeIds(); }
    int numEdges() const { return _target.numEdges(); }
    const Edge& edgeById(EdgeId edgeId) const { return _target.edgeById(edgeId); }

private:
    const Graph* _source;
    std::unique_ptr<GraphTransform> _graphTransform;
    MutableGraph _target;

    void rebuild();

private slots:
    void onGraphChanged(const Graph*);
};

#endif // TRANSFORMEDGRAPH_H
