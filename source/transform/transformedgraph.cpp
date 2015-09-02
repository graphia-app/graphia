#include "transformedgraph.h"

#include "../utils/utils.h"
#include "../utils/cpp1x_hacks.h"

#include <functional>

TransformedGraph::TransformedGraph(const Graph& source) :
    Graph(),
    _source(&source),
    _nodesDifference(source),
    _edgesDifference(source)
{
    connect(_source, &Graph::graphChanged, [this](const Graph*) { rebuild(); });
    connect(&_target, &Graph::graphChanged, this, &TransformedGraph::onTargetGraphChanged, Qt::DirectConnection);
    enableComponentManagement();

    // These connections allow us to track what changes, so we can then
    // re-emit a canonical set of signals once the transform is complete
    connect(_source, &Graph::nodeWillBeRemoved, [this](const Graph*, const Node* node) { _nodesDifference[node->id()].remove(); });
    connect(_source, &Graph::nodeAdded,         [this](const Graph*, const Node* node) { _nodesDifference[node->id()].add(); });
    connect(_source, &Graph::edgeWillBeRemoved, [this](const Graph*, const Edge* edge) { _edgesDifference[edge->id()].remove(); });
    connect(_source, &Graph::edgeAdded,         [this](const Graph*, const Edge* edge) { _edgesDifference[edge->id()].add(); });

    connect(&_target, &Graph::nodeWillBeRemoved,[this](const Graph*, const Node* node) { _nodesDifference[node->id()].remove(); });
    connect(&_target, &Graph::nodeAdded,        [this](const Graph*, const Node* node) { _nodesDifference[node->id()].add(); });
    connect(&_target, &Graph::edgeWillBeRemoved,[this](const Graph*, const Edge* edge) { _edgesDifference[edge->id()].remove(); });
    connect(&_target, &Graph::edgeAdded,        [this](const Graph*, const Edge* edge) { _edgesDifference[edge->id()].add(); });

    setTransform(std::make_unique<IdentityTransform>());
}

void TransformedGraph::setTransform(std::unique_ptr<GraphTransform> graphTransform)
{
    _graphTransform = std::move(graphTransform);
    rebuild();
}

void TransformedGraph::rebuild()
{
    emit graphWillChange(this);

    _target.performTransaction([this](MutableGraph&)
    {
        _graphTransform->apply(*_source, *this);
    });

    //FIXME filter components here
    _filteredComponentIds = _target.componentIds();

    emit graphChanged(this);
}

void TransformedGraph::onTargetGraphChanged(const Graph*)
{
    // Let everything know what changed; note the signals won't necessarily happen in the order
    // in which the changes originally occurred, but adding nodes and edges, then removing edges
    // and nodes ensures that the receivers get a sane view at all times
    for(auto nodeId : _source->nodeIds())
    {
        if(_nodesDifference[nodeId].added())
            emit nodeAdded(this, &_source->nodeById(nodeId));
    }

    for(auto edgeId : _source->edgeIds())
    {
        if(_edgesDifference[edgeId].added())
            emit edgeAdded(this, &_source->edgeById(edgeId));
        else if(_edgesDifference[edgeId].removed())
            emit edgeWillBeRemoved(this, &_source->edgeById(edgeId));
    }

    for(auto nodeId : _source->nodeIds())
    {
        if(_nodesDifference[nodeId].removed())
            emit nodeWillBeRemoved(this, &_source->nodeById(nodeId));
    }

    _nodesDifference.resetElements();
    _edgesDifference.resetElements();
}
