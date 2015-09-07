#ifndef GENERICGRAPHMODEL_H
#define GENERICGRAPHMODEL_H

#include "graphmodel.h"

#include "../transform/transformedgraph.h"

class GenericGraphModel : public GraphModel
{
    Q_OBJECT
public:
    GenericGraphModel(const QString& name);

private:
    MutableGraph _graph;
    TransformedGraph _transformedGraph;
    NodePositions _nodePositions;
    NodeVisuals _nodeVisuals;
    EdgeVisuals _edgeVisuals;

    NodeArray<QString> _nodeNames;

    QString _name;

public:
    MutableGraph& mutableGraph() { return _graph; }
    Graph& graph() { return _transformedGraph; }
    const Graph& graph() const { return _transformedGraph; }
    NodePositions& nodePositions() { return _nodePositions; }
    const NodePositions& nodePositions() const { return _nodePositions; }

    NodeVisuals& nodeVisuals() { return _nodeVisuals; }
    const NodeVisuals& nodeVisuals() const { return _nodeVisuals; }
    EdgeVisuals& edgeVisuals() { return _edgeVisuals; }
    const EdgeVisuals& edgeVisuals() const { return _edgeVisuals; }

    NodeArray<QString>& nodeNames() { return _nodeNames; }
    const NodeArray<QString>& nodeNames() const { return _nodeNames; }

    const QString& name() { return _name; }

    bool editable() { return true; }

private slots:
    void onGraphChanged(const Graph* graph);
};

#endif // GENERICGRAPHMODEL_H
