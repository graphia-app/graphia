#ifndef GENERICGRAPHMODEL_H
#define GENERICGRAPHMODEL_H

#include "graphmodel.h"

#include "../transform/graphtransformer.h"

class GenericGraphModel : public GraphModel
{
    Q_OBJECT
public:
    GenericGraphModel(const QString& name);

private:
    MutableGraph _graph;
    GraphTransformer _graphTransformer;
    NodePositions _nodePositions;
    NodeVisuals _nodeVisuals;
    EdgeVisuals _edgeVisuals;

    QString _name;

public:
    MutableGraph& mutableGraph() { return _graph; }
    Graph& graph() { return _graphTransformer.graph(); }
    const Graph& graph() const { return _graphTransformer.graph(); }
    NodePositions& nodePositions() { return _nodePositions; }
    const NodePositions& nodePositions() const { return _nodePositions; }

    NodeVisuals& nodeVisuals() { return _nodeVisuals; }
    const NodeVisuals& nodeVisuals() const { return _nodeVisuals; }
    EdgeVisuals& edgeVisuals() { return _edgeVisuals; }
    const EdgeVisuals& edgeVisuals() const { return _edgeVisuals; }

    const QString& name() { return _name; }

    bool editable() { return true; }

public slots:
    void onNodeAdded(const Graph*, NodeId nodeId);
    void onEdgeAdded(const Graph*, EdgeId edgeId);
};

#endif // GENERICGRAPHMODEL_H
