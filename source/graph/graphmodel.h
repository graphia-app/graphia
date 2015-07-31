#ifndef GRAPHMODEL_H
#define GRAPHMODEL_H

#include "../graph/graph.h"
#include "../graph/grapharray.h"
#include "../layout/layout.h"
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
    virtual MutableGraph& graph() = 0;
    virtual const Graph& graph() const = 0;
    virtual NodePositions& nodePositions() = 0;
    virtual const NodePositions& nodePositions() const = 0;

    virtual NodeVisuals& nodeVisuals() = 0;
    virtual const NodeVisuals& nodeVisuals() const = 0;
    virtual EdgeVisuals& edgeVisuals() = 0;
    virtual const EdgeVisuals& edgeVisuals() const = 0;

    virtual const QString& name() = 0;

    virtual bool editable() { return false; }

    virtual QQuickItem* contentQuickItem() { return nullptr; }
};

#endif // GRAPHMODEL_H
