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
    NodeArray<QVector3D> _nodePositions;
    ComponentArray<QVector2D> _componentPositions;
    QString _name;

public:
    Graph& graph() { return _graph; }
    const Graph& graph() const { return _graph; }
    NodeArray<QVector3D>& nodePositions() { return _nodePositions; }
    const NodeArray<QVector3D>& nodePositions() const { return _nodePositions; }
    ComponentArray<QVector2D>& componentPositions() { return _componentPositions; }
    const ComponentArray<QVector2D>& componentPositions() const { return _componentPositions; }

    const QString& name() { return _name; }
};

#endif // GENERICGRAPHMODEL_H
