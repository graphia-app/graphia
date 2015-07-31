#include "graphtransformer.h"

#include "../graph/graph.h"

GraphTransformer::GraphTransformer(const Graph& graph) :
    _graph(&graph)
{
}

const Graph& GraphTransformer::graph()
{
    return _graphTransform->graph();
}

void GraphTransformer::setTransform(std::unique_ptr<GraphTransform> graphTransform)
{
    _graphTransform = std::move(graphTransform);

    connect(_graph, &Graph::graphWillChange, _graphTransform.get(), &GraphTransform::onGraphWillChange, Qt::DirectConnection);
    connect(_graph, &Graph::nodeWillBeRemoved, _graphTransform.get(), &GraphTransform::onNodeWillBeRemoved, Qt::DirectConnection);
    connect(_graph, &Graph::nodeAdded, _graphTransform.get(), &GraphTransform::onNodeAdded, Qt::DirectConnection);
    connect(_graph, &Graph::edgeWillBeRemoved, _graphTransform.get(), &GraphTransform::onEdgeWillBeRemoved, Qt::DirectConnection);
    connect(_graph, &Graph::edgeAdded, _graphTransform.get(), &GraphTransform::onEdgeAdded, Qt::DirectConnection);
    connect(_graph, &Graph::graphChanged, _graphTransform.get(), &GraphTransform::onGraphChanged, Qt::DirectConnection);
}
