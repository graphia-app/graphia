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
    _nodeVisuals[nodeId].size = NODE_SIZE; //NODE_SIZE + Utils::rand(-0.3f, 0.4f);
    _nodeVisuals[nodeId].color = Qt::GlobalColor::blue; //Utils::randQColor();
    _nodeVisuals[nodeId].outlineColor.setAlphaF(0.0f);
}

void GenericGraphModel::onEdgeAdded(const Graph*, EdgeId edgeId)
{
    _edgeVisuals[edgeId].size = EDGE_SIZE; //EDGE_SIZE + Utils::rand(-0.05f, 0.05f);
    _edgeVisuals[edgeId].color = Qt::GlobalColor::green; //Utils::randQColor();
    _edgeVisuals[edgeId].outlineColor.setAlphaF(0.0f);
}

