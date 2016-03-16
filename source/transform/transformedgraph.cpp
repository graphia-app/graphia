#include "transformedgraph.h"

#include "../graph/componentmanager.h"

#include "../utils/utils.h"
#include "../utils/cpp1x_hacks.h"

#include <functional>

TransformedGraph::TransformedGraph(const Graph& source) :
    Graph(),
    _source(&source),
    _nodesState(source),
    _edgesState(source),
    _previousNodesState(source),
    _previousEdgesState(source)
{
    connect(_source, &Graph::graphChanged, [this] { rebuild(); });
    connect(&_target, &Graph::graphChanged, this, &TransformedGraph::onTargetGraphChanged, Qt::DirectConnection);
    enableComponentManagement();

    // These connections allow us to track what changes, so we can then
    // re-emit a canonical set of signals once the transform is complete
    connect(_source, &Graph::nodeRemoved,  [this](const Graph*, NodeId nodeId) { _nodesState[nodeId].remove(); });
    connect(_source, &Graph::nodeAdded,    [this](const Graph*, NodeId nodeId) { _nodesState[nodeId].add(); });
    connect(_source, &Graph::edgeRemoved,  [this](const Graph*, EdgeId edgeId) { _edgesState[edgeId].remove(); });
    connect(_source, &Graph::edgeAdded,    [this](const Graph*, EdgeId edgeId) { _edgesState[edgeId].add(); });

    connect(&_target, &Graph::nodeRemoved, [this](const Graph*, NodeId nodeId) { _nodesState[nodeId].remove(); });
    connect(&_target, &Graph::nodeAdded,   [this](const Graph*, NodeId nodeId) { _nodesState[nodeId].add(); reserveNodeId(nodeId); });
    connect(&_target, &Graph::edgeRemoved, [this](const Graph*, EdgeId edgeId) { _edgesState[edgeId].remove(); });
    connect(&_target, &Graph::edgeAdded,   [this](const Graph*, EdgeId edgeId) { _edgesState[edgeId].add(); reserveEdgeId(edgeId); });

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

    setPhase(tr("Transforming"));
    _target.performTransaction([this](MutableGraph&)
    {
        _graphTransform->applyFromSource(*_source, *this);
    });

    emit graphChanged(this);
    clearPhase();
}

void TransformedGraph::onTargetGraphChanged(const Graph*)
{
    // Let everything know what changed; note the signals won't necessarily happen in the order
    // in which the changes originally occurred, but adding nodes and edges, then removing edges
    // and nodes ensures that the receivers get a sane view at all times
    for(NodeId nodeId(0); nodeId < _nodesState.size(); ++nodeId)
    {
        if(!_previousNodesState[nodeId].added() && _nodesState[nodeId].added())
            emit nodeAdded(this, nodeId);
    }

    for(EdgeId edgeId(0); edgeId < _edgesState.size(); ++edgeId)
    {
        if(!_previousEdgesState[edgeId].added() && _edgesState[edgeId].added())
            emit edgeAdded(this, edgeId);
        else if(!_previousEdgesState[edgeId].removed() && _edgesState[edgeId].removed())
            emit edgeRemoved(this, edgeId);
    }

    for(NodeId nodeId(0); nodeId < _nodesState.size(); ++nodeId)
    {
        if(!_previousNodesState[nodeId].removed() && _nodesState[nodeId].removed())
            emit nodeRemoved(this, nodeId);
    }

    _previousNodesState = _nodesState;
    _previousEdgesState = _edgesState;

    _nodesState.resetElements();
    _edgesState.resetElements();
}
