#include "genericgraphmodel.h"
#include "../utils.h"

GenericGraphModel::GenericGraphModel(const QString &name) :
    _graph(),
    _nodePositions(_graph),
    _componentPositions(_graph),
    _nodeVisuals(_graph),
    _edgeVisuals(_graph),
    _name(name)
{
    connect(&_graph, &Graph::nodeAdded, this, &GenericGraphModel::onNodeAdded, Qt::DirectConnection);
    connect(&_graph, &Graph::edgeAdded, this, &GenericGraphModel::onEdgeAdded, Qt::DirectConnection);
}

const float NODE_SIZE = 0.6f;
const float EDGE_SIZE = 0.1f;

void GenericGraphModel::onNodeAdded(const Graph*, NodeId nodeId)
{
    if(!_nodeVisuals[nodeId]._initialised)
    {
        _nodeVisuals[nodeId]._size = NODE_SIZE + Utils::rand(-0.3f, 0.4f);
        _nodeVisuals[nodeId]._color = Utils::randQColor();
        _nodeVisuals[nodeId]._outlineColor.setAlphaF(0.0f);
        _nodeVisuals[nodeId]._initialised = true;
    }
}

void GenericGraphModel::onEdgeAdded(const Graph*, EdgeId edgeId)
{
    if(!_edgeVisuals[edgeId]._initialised)
    {
        _edgeVisuals[edgeId]._size = EDGE_SIZE + Utils::rand(-0.05f, 0.05f);
        _edgeVisuals[edgeId]._color = Utils::randQColor();
        _edgeVisuals[edgeId]._outlineColor.setAlphaF(0.0f);
        _edgeVisuals[edgeId]._initialised = true;
    }
}

