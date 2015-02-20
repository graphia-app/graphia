#include "componentmanager.h"
#include "../graph/grapharray.h"

#include <QtGlobal>

ComponentManager::ComponentManager(Graph& graph) :
    _debug(false)
{
    if(qgetenv("COMPONENTS_DEBUG").toInt())
        _debug = true;

    connect(&graph, &Graph::nodeAdded, this, &ComponentManager::onNodeAdded, Qt::DirectConnection);
    connect(&graph, &Graph::nodeWillBeRemoved, this, &ComponentManager::onNodeWillBeRemoved, Qt::DirectConnection);
    connect(&graph, &Graph::edgeAdded, this, &ComponentManager::onEdgeAdded, Qt::DirectConnection);
    connect(&graph, &Graph::edgeWillBeRemoved, this, &ComponentManager::onEdgeWillBeRemoved, Qt::DirectConnection);
    connect(&graph, &Graph::graphChanged, this, &ComponentManager::onGraphChanged, Qt::DirectConnection);

    connect(this, &ComponentManager::componentAdded, &graph, &Graph::componentAdded, Qt::DirectConnection);
    connect(this, &ComponentManager::componentWillBeRemoved, &graph, &Graph::componentWillBeRemoved, Qt::DirectConnection);
    connect(this, &ComponentManager::componentSplit, &graph, &Graph::componentSplit, Qt::DirectConnection);
    connect(this, &ComponentManager::componentsWillMerge, &graph, &Graph::componentsWillMerge, Qt::DirectConnection);
}

ComponentManager::~ComponentManager()
{
    // Let the ComponentArrays know that we're going away
    for(auto componentArray : _componentArrayList)
        componentArray->invalidate();
}

std::vector<NodeId> ComponentSplitSet::nodeIds() const
{
    std::vector<NodeId> nodeIds;

    for(auto componentId : _splitters)
    {
        auto component = _graph->componentById(componentId);
        nodeIds.insert(nodeIds.end(), component->nodeIds().begin(), component->nodeIds().end());
    }

    return nodeIds;
}

std::vector<NodeId> ComponentMergeSet::nodeIds() const
{
    std::vector<NodeId> nodeIds;

    for(auto componentId : _mergers)
    {
        auto component = _graph->componentById(componentId);
        nodeIds.insert(nodeIds.end(), component->nodeIds().begin(), component->nodeIds().end());
    }

    return nodeIds;
}
