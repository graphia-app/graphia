#include "selectionmanager.h"

#include "graph/graph.h"
#include "graph/graphmodel.h"

#include <algorithm>
#include <utility>
#include <array>

//#define EXPENSIVE_DEBUG_CHECKS

SelectionManager::SelectionManager(const GraphModel& graphModel) :
    QObject(),
    _graphModel(&graphModel)
{
    connect(&_graphModel->graph(), &Graph::nodeRemoved,
    this, [this](const Graph*, NodeId nodeId)
    {
       _deletedNodes.push_back(nodeId);
    });

    connect(&graphModel.graph(), &Graph::graphChanged,
    this, [this]
    {
        if(!_deletedNodes.empty())
        {
            deselectNodes(_deletedNodes);
            _deletedNodes.clear();
        }
    });
}

NodeIdSet SelectionManager::selectedNodes() const
{
#ifdef EXPENSIVE_DEBUG_CHECKS
    // Assertion that our selection doesn't contain things that aren't in the graph
    Q_ASSERT(std::all_of(_selectedNodes.begin(), _selectedNodes.end(),
        [this](ElementId<NodeId> nodeId)
        {
            auto& nodeIds = _graphModel->graph().nodeIds();
            return u::contains(nodeIds, nodeId);
        }));
#endif

    return _selectedNodeIds;
}

NodeIdSet SelectionManager::unselectedNodes() const
{
    auto& nodeIds = _graphModel->graph().nodeIds();
    auto unselectedNodeIds = NodeIdSet(nodeIds.begin(), nodeIds.end());
    unselectedNodeIds.erase(_selectedNodeIds.begin(), _selectedNodeIds.end());

    return unselectedNodeIds;
}

//FIXME http://en.cppreference.com/w/cpp/container/unordered_set/merge will be useful here
template<typename C> bool _selectNodes(const GraphModel& graphModel, NodeIdSet& selectedNodeIds,
    const C& nodeIds, bool selectMergedNodes = true)
{
    NodeIdSet newSelectedNodeIds;

    if(selectMergedNodes)
    {
        for(auto nodeId : nodeIds)
        {
            if(graphModel.graph().typeOf(nodeId) == MultiElementType::Tail)
                continue;

            auto mergedNodeIds = graphModel.graph().mergedNodeIdsForNodeId(nodeId);

            for(auto mergedNodeId : mergedNodeIds)
                newSelectedNodeIds.insert(mergedNodeId);
        }
    }
    else
    {
        for(auto nodeId : nodeIds)
            newSelectedNodeIds.insert(nodeId);
    }

    auto oldSize = selectedNodeIds.size();
    selectedNodeIds.insert(newSelectedNodeIds.begin(), newSelectedNodeIds.end());
    return selectedNodeIds.size() > oldSize;
}

bool SelectionManager::selectNodes(const NodeIdSet& nodeIds)
{
    return callFnAndMaybeEmit([this, &nodeIds]
    {
        return _selectNodes(*_graphModel, _selectedNodeIds, nodeIds, true);
    });
}

bool SelectionManager::selectNodes(const std::vector<NodeId>& nodeIds)
{
    return callFnAndMaybeEmit([this, &nodeIds]
    {
        return _selectNodes(*_graphModel, _selectedNodeIds, nodeIds, true);
    });
}

bool SelectionManager::selectNode(NodeId nodeId)
{
    return callFnAndMaybeEmit([this, nodeId]
    {
        return _selectNodes(*_graphModel, _selectedNodeIds, std::array<NodeId, 1>{{nodeId}}, true);
    });
}


template<typename C> bool _deselectNodes(const GraphModel& graphModel, NodeIdSet& selectedNodeIds,
    const C& nodeIds, bool deselectMergedNodes = true)
{
    bool selectionWillChange = false;

    if(deselectMergedNodes)
    {
        for(auto nodeId : nodeIds)
        {
            if(graphModel.graph().typeOf(nodeId) == MultiElementType::Tail)
                continue;

            auto mergedNodeIds = graphModel.graph().mergedNodeIdsForNodeId(nodeId);

            for(auto mergedNodeId : mergedNodeIds)
                selectionWillChange |= (selectedNodeIds.erase(mergedNodeId) > 0);
        }
    }
    else
    {
        for(auto nodeId : nodeIds)
            selectionWillChange |= (selectedNodeIds.erase(nodeId) > 0);
    }

    return selectionWillChange;
}

bool SelectionManager::deselectNode(NodeId nodeId)
{
    return callFnAndMaybeEmit([this, nodeId]
    {
        return _deselectNodes(*_graphModel, _selectedNodeIds, std::array<NodeId, 1>{{nodeId}}, true);
    });
}

bool SelectionManager::deselectNodes(const NodeIdSet& nodeIds)
{
    return callFnAndMaybeEmit([this, &nodeIds]
    {
        return _deselectNodes(*_graphModel, _selectedNodeIds, nodeIds, true);
    });
}

bool SelectionManager::deselectNodes(const std::vector<NodeId>& nodeIds)
{
    return callFnAndMaybeEmit([this, &nodeIds]
    {
        return _deselectNodes(*_graphModel, _selectedNodeIds, nodeIds, true);
    });
}

template<typename C> void _toggleNodes(NodeIdSet& selectedNodeIds, const C& nodeIds)
{
    NodeIdSet difference;
    for(auto nodeId : nodeIds)
    {
        if(!u::contains(selectedNodeIds, nodeId))
            difference.insert(nodeId);
    }

    selectedNodeIds = std::move(difference);
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
    Q_ASSERT(u::contains(_graphModel->graph().nodeIds(), nodeId));
#endif
    return u::contains(_selectedNodeIds, nodeId);
}

bool SelectionManager::setSelectedNodes(const NodeIdSet& nodeIds)
{
    return callFnAndMaybeEmit([this, &nodeIds]
    {
        bool selectionWillChange = u::setsDiffer(_selectedNodeIds, nodeIds);
        _selectedNodeIds = nodeIds;
        return selectionWillChange;
    });
}

bool SelectionManager::selectAllNodes()
{
    return callFnAndMaybeEmit([this]
    {
        return _selectNodes(*_graphModel, _selectedNodeIds, _graphModel->graph().nodeIds(), true);
    });
}

bool SelectionManager::clearNodeSelection()
{
    return callFnAndMaybeEmit([this]
    {
        bool selectionWillChange = !_selectedNodeIds.empty();
        _selectedNodeIds.clear();
        return selectionWillChange;
    });
}

void SelectionManager::invertNodeSelection()
{
    _toggleNodes(_selectedNodeIds, _graphModel->graph().nodeIds());

    if(!signalsSuppressed())
        emit selectionChanged(this);
}

QString SelectionManager::numNodesSelectedAsString() const
{
    int selectionSize = static_cast<int>(selectedNodes().size());

    if(selectionSize == 1)
    {
        auto nodeId = *selectedNodes().begin();
        auto& nodeName = _graphModel->nodeNames()[nodeId];

        if(!nodeName.isEmpty())
            return QString(tr("Selected %1")).arg(nodeName);

        return tr("1 Node Selected");
    }

    if(selectionSize > 1)
        return QString(tr("%1 Nodes Selected")).arg(selectionSize);

    return {};
}

void SelectionManager::suppressSignals()
{
    _suppressSignals = true;
}

bool SelectionManager::signalsSuppressed()
{
    bool suppressSignals = _suppressSignals;
    _suppressSignals = false;
    return suppressSignals;
}
