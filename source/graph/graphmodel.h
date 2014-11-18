#ifndef GRAPHMODEL_H
#define GRAPHMODEL_H

#include "../graph/graph.h"
#include "../graph/grapharray.h"
#include "../layout/layout.h"

#include <QWidget>
#include <QString>
#include <QColor>

#include <memory>

struct NodeVisual
{
    NodeVisual() :
        _initialised(false),
        _size(1.0f)
    {}

    bool _initialised;
    float _size;
    QColor _color;
};

typedef NodeArray<NodeVisual> NodeVisuals;

struct EdgeVisual
{
    EdgeVisual() :
        _initialised(false),
        _size(1.0f)
    {}

    bool _initialised;
    float _size;
    QColor _color;
};

typedef EdgeArray<EdgeVisual> EdgeVisuals;

class GraphModel : public QObject
{
    Q_OBJECT
public:
    virtual Graph& graph() = 0;
    virtual const Graph& graph() const = 0;
    virtual NodePositions& nodePositions() = 0;
    virtual const NodePositions& nodePositions() const = 0;

    virtual NodeVisuals& nodeVisuals() = 0;
    virtual const NodeVisuals& nodeVisuals() const = 0;
    virtual EdgeVisuals& edgeVisuals() = 0;
    virtual const EdgeVisuals& edgeVisuals() const = 0;

    virtual const QString& name() = 0;

    virtual bool editable() { return false; }

    virtual QWidget* contentWidget() { return nullptr; }
};

#endif // GRAPHMODEL_H
