#ifndef GRAPHMODEL_H
#define GRAPHMODEL_H

#include "../graph/graph.h"
#include "../graph/grapharray.h"

#include <QVector3D>
#include <QWidget>
#include <QString>

class GraphModel : public QObject
{
    Q_OBJECT
public:
    virtual Graph& graph() = 0;
    virtual const Graph& graph() const = 0;
    virtual NodeArray<QVector3D>& layout() = 0;
    virtual const NodeArray<QVector3D>& layout() const = 0;

    virtual const QString& name() = 0;

    virtual QWidget* contentWidget() { return nullptr; }
};

#endif // GRAPHMODEL_H
