#include "selectionmanager.h"

#include <algorithm>
#include <utility>

//#define EXPENSIVE_DEBUG_CHECKS

SelectionManager::SelectionManager(const GraphModel& graphModel) :
    QObject(),
    _graphModel(graphModel)
{
    connect(&_graphModel.graph(), &Graph::nodeRemoved,
    [this](const Graph*, NodeId nodeId)
    {
       _deletedNodes.push_back(nodeId);
    });

    connect(&graphModel.graph(), &Graph::graphChanged,
    [this]
    {
        deselectNodes(_deletedNodes);
        _deletedNodes.clear();
    });
}

NodeIdSet SelectionManager::selectedNodes() const
{
#ifdef EXPENSIVE_DEBUG_CHECKS
    // Assertion that our selection doesn't contain things that aren't in the graph
    Q_ASSERT(std::all_of(_selectedNodes.begin(), _selectedNodes.end(),
        [this](ElementId<NodeId> nodeId)
        {
            auto& nodeIds = _graphModel.graph().nodeIds();
            return u::contains(nodeIds, nodeId);
        }));
#endif

    return _selectedNodeIds;
}

NodeIdSet SelectionManager::unselectedNodes() const
{
    auto& nodeIds = _graphModel.graph().nodeIds();
    auto unselectedNodeIds = NodeIdSet(nodeIds.begin(), nodeIds.end());
    unselectedNodeIds.erase(_selectedNodeIds.begin(), _selectedNodeIds.end());

    return unselectedNodeIds;
}

bool SelectionManager::selectNode(NodeId nodeId)
{
    return selectNodes(_graphModel.graph().mergedNodeIdsForNodeId(nodeId));
}

bool SelectionManager::deselectNode(NodeId nodeId)
{
    return deselectNodes(_graphModel.graph().mergedNodeIdsForNodeId(nodeId));
}

void SelectionManager::toggleNode(NodeId nodeId)
{
    if(nodeIsSelected(nodeId))
        deselectNode(nodeId);
    else
        selectNode(nodeId);
}

bool SelectionManager::nodeIsSelected(NodeId nodeId) const
{
#ifdef EXPENSIVE_DEBUG_CHECKS
    Q_ASSERT(u::contains(_graphModel.graph().nodeIds(), nodeId));
#endif
    return u::contains(_selectedNodeIds, nodeId);
}

bool SelectionManager::selectAllNodes()
{
    return selectNodes(_graphModel.graph().nodeIds(), false);
}

bool SelectionManager::clearNodeSelection()
{
    bool selectionWillChange = !_selectedNodeIds.empty();
    _selectedNodeIds.clear();

    if(selectionWillChange)
        emit selectionChanged(this);

    return selectionWillChange;
}

void SelectionManager::invertNodeSelection()
{
    toggleNodes(_graphModel.graph().nodeIds());
}

const QString SelectionManager::numNodesSelectedAsString() const
{
    int selectionSize = static_cast<int>(selectedNodes().size());

    if(selectionSize == 1)
    {
        auto nodeId = *selectedNodes().begin();
        auto& nodeName = _graphModel.nodeNames()[nodeId];

        if(!nodeName.isEmpty())
            return QString(tr("Selected %1")).arg(nodeName);

        return tr("1 Node Selected");
    }
    else if(selectionSize > 1)
        return QString(tr("%1 Nodes Selected")).arg(selectionSize);

    return QString();
}
