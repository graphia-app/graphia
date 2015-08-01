#ifndef GRAPHTRANSFORMER_H
#define GRAPHTRANSFORMER_H

#include "graphtransform.h"

#include <QObject>

#include <memory>

class Graph;

class GraphTransformer : public QObject
{
    Q_OBJECT

public:
    GraphTransformer(Graph& graph);

    Graph& graph() { return _transformedGraph; }
    const Graph& graph() const { return _transformedGraph; }

    void setTransform(std::unique_ptr<GraphTransform> graphTransform);

private:
    const Graph* _graph;
    std::unique_ptr<GraphTransform> _graphTransform;
    MutableGraph _transformedGraph;
};

#endif // GRAPHTRANSFORMER_H
