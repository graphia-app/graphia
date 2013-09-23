#ifndef GRAPHMODEL_H
#define GRAPHMODEL_H

#include "../graph/graph.h"
#include "../graph/grapharray.h"
#include "../layout/layout.h"

#include <QVector2D>
#include <QVector3D>
#include <QWidget>
#include <QString>

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

    virtual const QString& name() = 0;

    virtual QWidget* contentWidget() { return nullptr; }
};

#endif // GRAPHMODEL_H
