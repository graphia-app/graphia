/* Copyright © 2013-2024 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "selectionmanager.h"

#include "app/graph/graph.h"
#include "app/graph/graphmodel.h"

#include <algorithm>
#include <utility>
#include <array>

//#define EXPENSIVE_DEBUG_CHECKS

SelectionManager::SelectionManager(const GraphModel& graphModel) :
    _graphModel(&graphModel)
{
    connect(&_graphModel->graph(), &Graph::nodeRemoved,
    [this](const Graph*, NodeId nodeId)
    {
       _deletedNodes.push_back(nodeId);
    });

    connect(&graphModel.graph(), &Graph::graphChanged,
    [this]
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
    const std::unique_lock<std::recursive_mutex> lock(_mutex);

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
    const std::unique_lock<std::recursive_mutex> lock(_mutex);

    const auto& nodeIds = _graphModel->graph().nodeIds();
    auto unselectedNodeIds = NodeIdSet(nodeIds.begin(), nodeIds.end());
    unselectedNodeIds.erase(_selectedNodeIds.begin(), _selectedNodeIds.end());

    return unselectedNodeIds;
}

//FIXME http://en.cppreference.com/w/cpp/container/unordered_set/merge will be useful here
template<typename C> bool _selectNodes(const GraphModel& graphModel, NodeIdSet& selectedNodeIds,
    NodeIdSet& mask, const C& nodeIds, bool selectMergedNodes = true)
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
            {
                if(mask.empty() || u::contains(mask, mergedNodeId))
                    newSelectedNodeIds.insert(mergedNodeId);
            }
        }
    }
    else
    {
        for(auto nodeId : nodeIds)
        {
            if(mask.empty() || u::contains(mask, nodeId))
                newSelectedNodeIds.insert(nodeId);
        }
    }

    auto oldSize = selectedNodeIds.size();
    selectedNodeIds.insert(newSelectedNodeIds.begin(), newSelectedNodeIds.end());
    return selectedNodeIds.size() > oldSize;
}

bool SelectionManager::selectNodes(const NodeIdSet& nodeIds)
{
    return callFnAndMaybeEmit([this, &nodeIds]
    {
        return _selectNodes(*_graphModel, _selectedNodeIds, _nodeIdsMask, nodeIds, true);
    });
}

bool SelectionManager::selectNodes(const std::vector<NodeId>& nodeIds)
{
    return callFnAndMaybeEmit([this, &nodeIds]
    {
        return _selectNodes(*_graphModel, _selectedNodeIds, _nodeIdsMask, nodeIds, true);
    });
}

bool SelectionManager::selectNode(NodeId nodeId)
{
    return callFnAndMaybeEmit([this, nodeId]
    {
        return _selectNodes(*_graphModel, _selectedNodeIds, _nodeIdsMask, std::array<NodeId, 1>{{nodeId}}, true);
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

template<typename C> void _toggleNodes(NodeIdSet& selectedNodeIds, NodeIdSet& mask, const C& nodeIds)
{
    NodeIdSet difference;
    for(auto nodeId : nodeIds)
    {
        if(!u::contains(selectedNodeIds, nodeId))
        {
            if(mask.empty() || u::contains(mask, nodeId))
                difference.insert(nodeId);
        }
    }

    selectedNodeIds = std::move(difference);
}

bool SelectionManager::toggleNode(NodeId nodeId)
{
    if(nodeIsSelected(nodeId))
    {
        deselectNode(nodeId);
        return false;
    }

    selectNode(nodeId);
    return true;
}

bool SelectionManager::nodeIsSelected(NodeId nodeId) const
{
    const std::unique_lock<std::recursive_mutex> lock(_mutex);

#ifdef EXPENSIVE_DEBUG_CHECKS
    Q_ASSERT(u::contains(_graphModel->graph().nodeIds(), nodeId));
#endif
    return u::contains(_selectedNodeIds, nodeId);
}

bool SelectionManager::selectAllNodes()
{
    return callFnAndMaybeEmit([this]
    {
        bool nodesDeselected = false;

        // If there is a mask in place, selecting all might actually need some deselection first
        if(!_nodeIdsMask.empty() && !_selectedNodeIds.empty())
        {
            NodeIdSet deselectedNodeIds;
            for(auto nodeId : _selectedNodeIds)
            {
                if(!u::contains(_nodeIdsMask, nodeId))
                    deselectedNodeIds.insert(nodeId);
            }

            nodesDeselected = _deselectNodes(*_graphModel, _selectedNodeIds, deselectedNodeIds, true);
        }

        return _selectNodes(*_graphModel, _selectedNodeIds, _nodeIdsMask,
            _graphModel->graph().nodeIds(), true) || nodesDeselected;
    });
}

bool SelectionManager::clearNodeSelection()
{
    return callFnAndMaybeEmit([this]
    {
        const bool selectionWillChange = !_selectedNodeIds.empty();
        _selectedNodeIds.clear();
        return selectionWillChange;
    });
}

void SelectionManager::invertNodeSelection()
{
    const std::unique_lock<std::recursive_mutex> lock(_mutex);

    _toggleNodes(_selectedNodeIds, _nodeIdsMask, _graphModel->graph().nodeIds());

    if(!signalsSuppressed())
        emit selectionChanged(this);
}

void SelectionManager::setNodesMask(const NodeIdSet& nodeIds, bool applyMask)
{
    _nodeIdsMask = nodeIds;

    if(applyMask)
    {
        std::vector<NodeId> nodeIdsToDeselect;

        for(const auto& selectedNodeId : _selectedNodeIds)
        {
            if(!u::contains(_nodeIdsMask, selectedNodeId))
                nodeIdsToDeselect.emplace_back(selectedNodeId);
        }

        deselectNodes(nodeIdsToDeselect);
    }

    emit nodesMaskChanged();
}

void SelectionManager::setNodesMask(const std::vector<NodeId>& nodeIds, bool applyMask)
{
    setNodesMask(NodeIdSet(nodeIds.begin(), nodeIds.end()), applyMask);
}

QString SelectionManager::numNodesSelectedAsString() const
{
    const int selectionSize = static_cast<int>(selectedNodes().size());

    if(selectionSize == 1)
    {
        auto nodeId = *selectedNodes().begin();
        const auto& nodeName = _graphModel->nodeNames()[nodeId];

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
    const bool suppressSignals = _suppressSignals;
    _suppressSignals = false;
    return suppressSignals;
}
