#include "transformedgraph.h"

#include "identitytransform.h"

#include "../utils/cpp1x_hacks.h"

TransformedGraph::TransformedGraph(const Graph& source) :
    Graph(true),
    _source(&source)
{
    // Forward the signals, but with the graph argument replaced with this
    connect(&_target, &Graph::graphWillChange,   [this](const Graph*)                   { emit graphWillChange(this); });
    connect(&_target, &Graph::nodeWillBeRemoved, [this](const Graph*, const Node* node) { emit nodeWillBeRemoved(this, node); });
    connect(&_target, &Graph::nodeAdded,         [this](const Graph*, const Node* node) { emit nodeAdded(this, node); });
    connect(&_target, &Graph::edgeWillBeRemoved, [this](const Graph*, const Edge* edge) { emit edgeWillBeRemoved(this, edge); });
    connect(&_target, &Graph::edgeAdded,         [this](const Graph*, const Edge* edge) { emit edgeAdded(this, edge); });
    connect(&_target, &Graph::graphChanged,      [this](const Graph*)                   { emit graphChanged(this); });

    setTransform(std::make_unique<IdentityTransform>(_target));
}

void TransformedGraph::setTransform(std::unique_ptr<GraphTransform> graphTransform)
{
    _graphTransform = std::move(graphTransform);

    connect(_source, &Graph::graphWillChange,   _graphTransform.get(), &GraphTransform::onGraphWillChange, Qt::DirectConnection);
    connect(_source, &Graph::nodeWillBeRemoved, _graphTransform.get(), &GraphTransform::onNodeWillBeRemoved, Qt::DirectConnection);
    connect(_source, &Graph::nodeAdded,         _graphTransform.get(), &GraphTransform::onNodeAdded, Qt::DirectConnection);
    connect(_source, &Graph::edgeWillBeRemoved, _graphTransform.get(), &GraphTransform::onEdgeWillBeRemoved, Qt::DirectConnection);
    connect(_source, &Graph::edgeAdded,         _graphTransform.get(), &GraphTransform::onEdgeAdded, Qt::DirectConnection);
    connect(_source, &Graph::graphChanged,      _graphTransform.get(), &GraphTransform::onGraphChanged, Qt::DirectConnection);
}
