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
    virtual std::shared_ptr<Graph> graph() = 0;
    virtual std::shared_ptr<const Graph> graph() const = 0;
    virtual std::shared_ptr<NodePositions> nodePositions() = 0;
    virtual std::shared_ptr<const NodePositions> nodePositions() const = 0;

    virtual std::shared_ptr<NodeVisuals> nodeVisuals() = 0;
    virtual std::shared_ptr<const NodeVisuals> nodeVisuals() const = 0;
    virtual std::shared_ptr<EdgeVisuals> edgeVisuals() = 0;
    virtual std::shared_ptr<const EdgeVisuals> edgeVisuals() const = 0;

    virtual const QString& name() = 0;

    virtual bool editable() { return false; }

    virtual QWidget* contentWidget() { return nullptr; }
};

#endif // GRAPHMODEL_H
