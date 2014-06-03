#ifndef GRAPHMODEL_H
#define GRAPHMODEL_H

#include "../graph/graph.h"
#include "../graph/grapharray.h"
#include "../layout/layout.h"

#include <QWidget>
#include <QString>
#include <QColor>

struct NodeVisual
{
    bool _initialised;
    float _size;
    QColor _color;
    QColor _outlineColor;
};

typedef NodeArray<NodeVisual> NodeVisuals;

struct EdgeVisual
{
    bool _initialised;
    float _size;
    QColor _color;
    QColor _outlineColor;
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
    virtual ComponentPositions& componentPositions() = 0;
    virtual const ComponentPositions& componentPositions() const = 0;

    virtual NodeVisuals& nodeVisuals() = 0;
    virtual const NodeVisuals& nodeVisuals() const = 0;
    virtual EdgeVisuals& edgeVisuals() = 0;
    virtual const EdgeVisuals& edgeVisuals() const = 0;

    virtual const QString& name() = 0;

    virtual QWidget* contentWidget() { return nullptr; }
};

#endif // GRAPHMODEL_H
