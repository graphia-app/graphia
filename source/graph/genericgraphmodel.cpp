#include "genericgraphmodel.h"
#include "../utils/utils.h"

GenericGraphModel::GenericGraphModel(const QString &name) :
    _graph(),
    _graphTransformer(_graph),
    _nodePositions(_graph),
    _nodeVisuals(_graph),
    _edgeVisuals(_graph),
    _name(name)
{
    connect(&_graph, &Graph::nodeAdded, this, &GenericGraphModel::onNodeAdded, Qt::DirectConnection);
    connect(&_graph, &Graph::edgeAdded, this, &GenericGraphModel::onEdgeAdded, Qt::DirectConnection);
}

const float NODE_SIZE = 0.6f;
const float EDGE_SIZE = 0.2f;

void GenericGraphModel::onNodeAdded(const Graph*, const Node* node)
{
    if(!_nodeVisuals[node->id()]._initialised)
    {
        _nodeVisuals[node->id()]._size = NODE_SIZE + Utils::rand(-0.3f, 0.4f);
        _nodeVisuals[node->id()]._color = Utils::randQColor();
        _nodeVisuals[node->id()]._initialised = true;
    }
}

void GenericGraphModel::onEdgeAdded(const Graph*, const Edge* edge)
{
    if(!_edgeVisuals[edge->id()]._initialised)
    {
        _edgeVisuals[edge->id()]._size = EDGE_SIZE + Utils::rand(-0.05f, 0.05f);
        _edgeVisuals[edge->id()]._color = Utils::randQColor();
        _edgeVisuals[edge->id()]._initialised = true;
    }
}

