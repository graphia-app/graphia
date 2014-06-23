#ifndef GENERICGRAPHMODEL_H
#define GENERICGRAPHMODEL_H

#include <QLabel>

#include "graphmodel.h"

class GenericGraphModel : public GraphModel
{
    Q_OBJECT
public:
    GenericGraphModel(const QString& name);

private:
    Graph _graph;
    NodePositions _nodePositions;
    NodeVisuals _nodeVisuals;
    EdgeVisuals _edgeVisuals;

    QString _name;

public:
    Graph& graph() { return _graph; }
    const Graph& graph() const { return _graph; }
    NodePositions& nodePositions() { return _nodePositions; }
    const NodePositions& nodePositions() const { return _nodePositions; }

    NodeVisuals& nodeVisuals() { return _nodeVisuals; }
    const NodeVisuals& nodeVisuals() const { return _nodeVisuals; }
    EdgeVisuals& edgeVisuals() { return _edgeVisuals; }
    const EdgeVisuals& edgeVisuals() const { return _edgeVisuals; }

    const QString& name() { return _name; }

    bool editable() { return true; }

public slots:
    void onNodeAdded(const Graph&, NodeId nodeId);
    void onEdgeAdded(const Graph&, EdgeId edgeId);
};

#endif // GENERICGRAPHMODEL_H
