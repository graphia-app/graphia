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
    NodeArray<QVector3D> _layout;
    QString _name;

public:
    Graph& graph() { return _graph; }
    const Graph& graph() const { return _graph; }
    NodeArray<QVector3D>& layout() { return _layout; }
    const NodeArray<QVector3D>& layout() const { return _layout; }

    const QString& name() { return _name; }
};

#endif // GENERICGRAPHMODEL_H
