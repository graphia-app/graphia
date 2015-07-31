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

    Graph& graph() { return _graphTransform->graph(); }
    const Graph& graph() const { return _graphTransform->graph(); }

    void setTransform(std::unique_ptr<GraphTransform> graphTransform);

private:
    const Graph* _graph;
    std::unique_ptr<GraphTransform> _graphTransform;
};

#endif // GRAPHTRANSFORMER_H
