#ifndef GRAPHTRANSFORM_H
#define GRAPHTRANSFORM_H

#include "../graph/graph.h"

#include <QObject>

class GraphTransform : public QObject
{
    friend class TransformedGraph;

    Q_OBJECT

private slots:
    virtual void onGraphWillChange(const Graph*) {}

    virtual void onNodeAdded(const Graph*, const Node*) {}
    virtual void onNodeWillBeRemoved(const Graph*, const Node*) {}
    virtual void onEdgeAdded(const Graph*, const Edge*) {}
    virtual void onEdgeWillBeRemoved(const Graph*, const Edge*) {}

    virtual void onGraphChanged(const Graph*) {}
};

#endif // GRAPHTRANSFORM_H
