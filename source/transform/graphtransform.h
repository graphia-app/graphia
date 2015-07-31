#ifndef GRAPHTRANSFORM_H
#define GRAPHTRANSFORM_H

#include "../graph/graph.h"

#include <QObject>

class GraphTransform : public QObject
{
    friend class GraphTransformer;

    Q_OBJECT

public:
    virtual const Graph& graph() = 0;

private slots:
    virtual void onGraphWillChange(const Graph*) const {}

    virtual void onNodeAdded(const Graph*, NodeId) const {}
    virtual void onNodeWillBeRemoved(const Graph*, NodeId) const {}
    virtual void onEdgeAdded(const Graph*, EdgeId) const {}
    virtual void onEdgeWillBeRemoved(const Graph*, EdgeId) const {}

    virtual void onGraphChanged(const Graph*) const {}
};

#endif // GRAPHTRANSFORM_H
