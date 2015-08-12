#include "transformedgraph.h"

#include "../utils/utils.h"
#include "../utils/cpp1x_hacks.h"

#include <functional>

TransformedGraph::TransformedGraph(const MutableGraph& source) :
    Graph(true),
    _source(&source),
    _nodesDifference(source),
    _edgesDifference(source)
{
    connect(_source, &Graph::graphChanged, [this](const Graph*) { rebuild(); });

    connect(_source, &Graph::nodeWillBeRemoved,         [this](const Graph*, const Node* node) { _nodesDifference[node->id()].remove(); });
    connect(_source, &Graph::nodeAdded,                 [this](const Graph*, const Node* node) { _nodesDifference[node->id()].add(); });
    connect(_source, &Graph::edgeWillBeRemoved,         [this](const Graph*, const Edge* edge) { _edgesDifference[edge->id()].remove(); });
    connect(_source, &Graph::edgeAdded,                 [this](const Graph*, const Edge* edge) { _edgesDifference[edge->id()].add(); });

    connect(&_target, &Graph::nodeWillBeRemoved,        [this](const Graph*, const Node* node) { _nodesDifference[node->id()].remove(); });
    connect(&_target, &Graph::nodeAdded,                [this](const Graph*, const Node* node) { _nodesDifference[node->id()].add(); });
    connect(&_target, &Graph::edgeWillBeRemoved,        [this](const Graph*, const Edge* edge) { _edgesDifference[edge->id()].remove(); });
    connect(&_target, &Graph::edgeAdded,                [this](const Graph*, const Edge* edge) { _edgesDifference[edge->id()].add(); });

    setTransform(std::make_unique<IdentityTransform>());
}

void TransformedGraph::setTransform(std::unique_ptr<GraphTransform> graphTransform)
{
    _graphTransform = std::move(graphTransform);
    rebuild();
}

void TransformedGraph::contractEdge(EdgeId edgeId)
{
    // Can't contract an edge that doesn't exist
    if(!_target.containsEdgeId(edgeId))
        return;

    auto edge = _target.edgeById(edgeId);
    auto nodeIdToRemove = edge.sourceId() > edge.targetId() ? edge.sourceId() : edge.targetId();
    auto nodeIdToKeep = edge.oppositeId(nodeIdToRemove);
    auto nodeToRemove = _target.nodeById(nodeIdToRemove);

    for(auto edgeIdToMove : nodeToRemove.edgeIds())
    {
        if(edgeIdToMove != edgeId)
        {
            auto edgeToMove = _target.edgeById(edgeIdToMove);
            auto otherNodeId = edgeToMove.oppositeId(nodeIdToRemove);
            auto adjacentNodeIds = _target.adjacentNodeIds(otherNodeId);

            _target.removeEdge(edgeIdToMove);

            // If otherNodeId is not already connected to nodeIdToKeep
            if(adjacentNodeIds.find(nodeIdToKeep) == adjacentNodeIds.end())
            {
                if(edgeToMove.sourceId() == otherNodeId)
                    _target.addEdge(edgeIdToMove, otherNodeId, nodeIdToKeep);
                else
                    _target.addEdge(edgeIdToMove, nodeIdToKeep, otherNodeId);
            }
            else
            {
                //FIXME edgeIdToMove and edge between otherNodeId and nodeIdToKeep becomes a multiedge
            }
        }
    }

    _target.removeNode(nodeIdToRemove);
    //FIXME nodeIdToKeep and nodeIdToRemove becomes a multinode
}

void TransformedGraph::rebuild()
{
    _target.performTransaction([this](MutableGraph&)
    {
        _graphTransform->apply(*_source, *this);
    });

    emit graphWillChange(this);

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

    emit graphChanged(this);
}
