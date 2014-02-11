#include "selectionmanager.h"

QSet<NodeId> SelectionManager::selectedNodes()
{
    return _selectedNodes;
}

void SelectionManager::selectNode(NodeId nodeId)
{
    _selectedNodes.insert(nodeId);
}

void SelectionManager::selectNodes(QSet<NodeId> nodeIds)
{
    _selectedNodes.unite(nodeIds);
}

void SelectionManager::deselectNode(NodeId nodeId)
{
    _selectedNodes.remove(nodeId);
}

void SelectionManager::deselectNodes(QSet<NodeId> nodeIds)
{
    _selectedNodes.subtract(nodeIds);
}

void SelectionManager::toggleNode(NodeId nodeId)
{
    if(nodeIsSelected(nodeId))
        deselectNode(nodeId);
    else
        selectNode(nodeId);
}

void SelectionManager::toggleNodes(QSet<NodeId> nodeIds)
{
    QSet<NodeId> intersection(_selectedNodes);
    intersection.intersect(nodeIds);

    _selectedNodes.unite(nodeIds);
    _selectedNodes.subtract(intersection);
}

bool SelectionManager::nodeIsSelected(NodeId nodeId)
{
    return _selectedNodes.contains(nodeId);
}

void SelectionManager::resetNodeSelection()
{
    _selectedNodes.clear();
}
