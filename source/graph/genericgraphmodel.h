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
    std::shared_ptr<Graph> _graph;
    std::shared_ptr<NodePositions> _nodePositions;
    std::shared_ptr<NodeVisuals> _nodeVisuals;
    std::shared_ptr<EdgeVisuals> _edgeVisuals;

    QString _name;

public:
    std::shared_ptr<Graph> graph() { return _graph; }
    std::shared_ptr<const Graph> graph() const { return _graph; }
    std::shared_ptr<NodePositions> nodePositions() { return _nodePositions; }
    std::shared_ptr<const NodePositions> nodePositions() const { return _nodePositions; }

    std::shared_ptr<NodeVisuals> nodeVisuals() { return _nodeVisuals; }
    std::shared_ptr<const NodeVisuals> nodeVisuals() const { return _nodeVisuals; }
    std::shared_ptr<EdgeVisuals> edgeVisuals() { return _edgeVisuals; }
    std::shared_ptr<const EdgeVisuals> edgeVisuals() const { return _edgeVisuals; }

    const QString& name() { return _name; }

    bool editable() { return true; }

public slots:
    void onNodeAdded(const Graph*, NodeId nodeId);
    void onEdgeAdded(const Graph*, EdgeId edgeId);
};

#endif // GENERICGRAPHMODEL_H
