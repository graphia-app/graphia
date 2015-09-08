#ifndef GRAPHMODEL_H
#define GRAPHMODEL_H

#include "../graph/graph.h"
#include "../graph/grapharray.h"
#include "../transform/transformedgraph.h"

#include "../layout/nodepositions.h"
#include "../utils/cpp1x_hacks.h"

#include <QQuickItem>
#include <QString>
#include <QColor>

#include <memory>

struct NodeVisual
{
    NodeVisual() noexcept {}
    NodeVisual(NodeVisual&& other) noexcept :
        _initialised(other._initialised),
        _size(other._size),
        _color(other._color)
    {}

    bool _initialised = false;
    float _size = 1.0f;
    QColor _color;
};

typedef NodeArray<NodeVisual> NodeVisuals;

struct EdgeVisual
{
    EdgeVisual() noexcept {}
    EdgeVisual(EdgeVisual&& other) noexcept :
        _initialised(other._initialised),
        _size(other._size),
        _color(other._color)
    {}

    bool _initialised = false;
    float _size = 1.0f;
    QColor _color;
};

typedef EdgeArray<EdgeVisual> EdgeVisuals;

class GraphModel : public QObject
{
    Q_OBJECT
public:
    GraphModel(const QString& name);

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
    const Graph& graph() const { return _transformedGraph; }
    NodePositions& nodePositions() { return _nodePositions; }
    const NodePositions& nodePositions() const { return _nodePositions; }

    const NodeVisuals& nodeVisuals() const { return _nodeVisuals; }
    const EdgeVisuals& edgeVisuals() const { return _edgeVisuals; }

    const NodeArray<QString>& nodeNames() const { return _nodeNames; }
    void setNodeName(NodeId nodeId, const QString& name) { _nodeNames[nodeId] = name; }

    const QString& name() const { return _name; }

    bool editable() const { return true; }

    virtual QQuickItem* contentQuickItem() const { return nullptr; }

private slots:
    void onGraphChanged(const Graph* graph);
};

#endif // GRAPHMODEL_H
