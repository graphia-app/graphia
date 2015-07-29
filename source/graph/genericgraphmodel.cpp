#include "genericgraphmodel.h"
#include "../utils/utils.h"

GenericGraphModel::GenericGraphModel(const QString &name) :
    _graph(),
    _nodePositions(_graph),
    _nodeVisuals(_graph),
    _edgeVisuals(_graph),
    _name(name)
{
    connect(&_graph, &MutableGraph::nodeAdded, this, &GenericGraphModel::onNodeAdded, Qt::DirectConnection);
    connect(&_graph, &MutableGraph::edgeAdded, this, &GenericGraphModel::onEdgeAdded, Qt::DirectConnection);
}

const float NODE_SIZE = 0.6f;
const float EDGE_SIZE = 0.2f;

void GenericGraphModel::onNodeAdded(const MutableGraph*, NodeId nodeId)
{
    if(!_nodeVisuals[nodeId]._initialised)
    {
        _nodeVisuals[nodeId]._size = NODE_SIZE + Utils::rand(-0.3f, 0.4f);
        _nodeVisuals[nodeId]._color = Utils::randQColor();
        _nodeVisuals[nodeId]._initialised = true;
    }
}

void GenericGraphModel::onEdgeAdded(const MutableGraph*, EdgeId edgeId)
{
    if(!_edgeVisuals[edgeId]._initialised)
    {
        _edgeVisuals[edgeId]._size = EDGE_SIZE + Utils::rand(-0.05f, 0.05f);
        _edgeVisuals[edgeId]._color = Utils::randQColor();
        _edgeVisuals[edgeId]._initialised = true;
    }
}

