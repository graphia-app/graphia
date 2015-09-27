#include "graphmodel.h"
#include "../utils/utils.h"

GraphModel::GraphModel(const QString &name) :
    _graph(),
    _transformedGraph(_graph),
    _nodePositions(_graph),
    _nodeVisuals(_graph),
    _edgeVisuals(_graph),
    _nodeNames(_graph),
    _name(name)
{
    connect(&_transformedGraph, &Graph::graphChanged, this, &GraphModel::onGraphChanged, Qt::DirectConnection);
}

const float NODE_SIZE = 0.6f;
const float EDGE_SIZE = 0.2f;

void GraphModel::onGraphChanged(const Graph* graph)
{
    for(auto nodeId : graph->nodeIds())
    {
        _nodeVisuals[nodeId]._size = NODE_SIZE;
        _nodeVisuals[nodeId]._color = graph->typeOf(nodeId) == NodeIdSetCollection::Type::Not ?
                    Qt::GlobalColor::blue : Qt::GlobalColor::red;
        _nodeVisuals[nodeId]._initialised = true;
    }

    for(auto edgeId : graph->edgeIds())
    {
        _edgeVisuals[edgeId]._size = EDGE_SIZE;
        _edgeVisuals[edgeId]._color = graph->typeOf(edgeId) == EdgeIdSetCollection::Type::Not ?
                    Qt::GlobalColor::white : Qt::GlobalColor::red;
        _edgeVisuals[edgeId]._initialised = true;
    }
}

