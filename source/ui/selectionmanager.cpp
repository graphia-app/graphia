#include "selectionmanager.h"

template<typename T> QSet<T> setForVector(QVector<T> vector)
{
    //FIXME this is probably sub-optimal
    return vector.toList().toSet();
}

QSet<NodeId> SelectionManager::selectedNodes()
{
    // Assertion that our selection doesn't contain things that aren't in the graph
    Q_ASSERT(setForVector(_graph->nodeIds()).contains(_selectedNodes));
    return _selectedNodes;
}

QSet<NodeId> SelectionManager::unselectedNodes()
{
    return setForVector(_graph->nodeIds()).subtract(_selectedNodes);
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
    selectNodes(setForVector(_graph->nodeIds()));
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
    toggleNodes(setForVector(_graph->nodeIds()));
}
