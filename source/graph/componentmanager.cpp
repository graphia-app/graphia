#include "componentmanager.h"
#include "../graph/grapharray.h"

ComponentManager::ComponentManager(const Graph& graph)
{
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
