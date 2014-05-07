#include "selectionmanager.h"

QSet<NodeId> SelectionManager::selectedNodes()
{
    // Assertion that our selection doesn't contain things that aren't in the graph
    Q_ASSERT(_graph->nodeIds().toSet().contains(_selectedNodes));
    return _selectedNodes;
}

QSet<NodeId> SelectionManager::unselectedNodes()
{
    return _graph->nodeIds().toSet().subtract(_selectedNodes);
}

void SelectionManager::selectNode(NodeId nodeId)
{
    bool selectionWillChange = !_selectedNodes.contains(nodeId);
    _selectedNodes.insert(nodeId);

    if(selectionWillChange)
        emit selectionChanged();
}

void SelectionManager::selectNodes(QSet<NodeId> nodeIds)
{
    bool selectionWillChange = !_selectedNodes.contains(nodeIds);
    _selectedNodes.unite(nodeIds);

    if(selectionWillChange)
        emit selectionChanged();
}

void SelectionManager::deselectNode(NodeId nodeId)
{
    bool selectionWillChange = _selectedNodes.contains(nodeId);
    _selectedNodes.remove(nodeId);

    if(selectionWillChange)
        emit selectionChanged();
}

void SelectionManager::deselectNodes(QSet<NodeId> nodeIds)
{
    QSet<NodeId> intersection(_selectedNodes);
    intersection.intersect(nodeIds);

    bool selectionWillChange = !intersection.empty();
    _selectedNodes.subtract(nodeIds);

    if(selectionWillChange)
        emit selectionChanged();
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

    emit selectionChanged();
}

bool SelectionManager::nodeIsSelected(NodeId nodeId)
{
    Q_ASSERT(_graph->nodeIds().contains(nodeId));
    return _selectedNodes.contains(nodeId);
}

void SelectionManager::selectAllNodes()
{
    selectNodes(_graph->nodeIds().toSet());
}

void SelectionManager::clearNodeSelection()
{
    bool selectionWillChange = !_selectedNodes.empty();
    _selectedNodes.clear();

    if(selectionWillChange)
        emit selectionChanged();
}

void SelectionManager::invertNodeSelection()
{
    toggleNodes(_graph->nodeIds().toSet());
}
