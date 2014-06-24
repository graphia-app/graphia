#include "genericgraphmodel.h"
#include "../utils.h"

GenericGraphModel::GenericGraphModel(const QString &name) :
    _graph(),
    _name(name)
{
    _graph = std::make_shared<Graph>();
    _nodePositions = std::make_shared<NodePositions>(_graph);
    _nodeVisuals = std::make_shared<NodeVisuals>(_graph);
    _edgeVisuals = std::make_shared<EdgeVisuals>(_graph);

    connect(_graph.get(), &Graph::nodeAdded, this, &GenericGraphModel::onNodeAdded, Qt::DirectConnection);
    connect(_graph.get(), &Graph::edgeAdded, this, &GenericGraphModel::onEdgeAdded, Qt::DirectConnection);
}

const float NODE_SIZE = 0.6f;
const float EDGE_SIZE = 0.1f;

void GenericGraphModel::onNodeAdded(const Graph*, NodeId nodeId)
{
    auto& nodeVisuals = *_nodeVisuals;

    if(!nodeVisuals[nodeId]._initialised)
    {
        nodeVisuals[nodeId]._size = NODE_SIZE + Utils::rand(-0.3f, 0.4f);
        nodeVisuals[nodeId]._color = Utils::randQColor();
        nodeVisuals[nodeId]._outlineColor.setAlphaF(0.0f);
        nodeVisuals[nodeId]._initialised = true;
    }
}

void GenericGraphModel::onEdgeAdded(const Graph*, EdgeId edgeId)
{
    auto& edgeVisuals = *_edgeVisuals;

    if(!edgeVisuals[edgeId]._initialised)
    {
        edgeVisuals[edgeId]._size = EDGE_SIZE + Utils::rand(-0.05f, 0.05f);
        edgeVisuals[edgeId]._color = Utils::randQColor();
        edgeVisuals[edgeId]._outlineColor.setAlphaF(0.0f);
        edgeVisuals[edgeId]._initialised = true;
    }
}

