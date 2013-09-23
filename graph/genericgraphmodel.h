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
    ComponentPositions _componentPositions;
    QString _name;

public:
    Graph& graph() { return _graph; }
    const Graph& graph() const { return _graph; }
    NodePositions& nodePositions() { return _nodePositions; }
    const NodePositions& nodePositions() const { return _nodePositions; }
    ComponentPositions& componentPositions() { return _componentPositions; }
    const ComponentPositions& componentPositions() const { return _componentPositions; }

    const QString& name() { return _name; }
};

#endif // GENERICGRAPHMODEL_H
