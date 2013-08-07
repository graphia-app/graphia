#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QWidget>

#include "graph/graph.h"
#include "graph/grapharray.h"

class GraphWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GraphWidget(QWidget *parent = 0) :
        QWidget(parent) {}

signals:
    void progress(int percentage) const;
    void complete(int success) const;

public slots:

public:
    virtual const Graph& graph() = 0;
    virtual const NodeArray<QVector3D>& layout() = 0;

    virtual const QString& name() = 0;

    virtual bool initialised() { return false; }
};

#endif // GRAPHWIDGET_H
