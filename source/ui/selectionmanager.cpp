#include "selectionmanager.h"

#include <algorithm>
#include <utility>

//#define EXPENSIVE_DEBUG_CHECKS

SelectionManager::SelectionManager(const ImmutableGraph& graph) :
    QObject(),
    _graph(graph)
{}

ElementIdSet<NodeId> SelectionManager::selectedNodes() const
{
#ifdef EXPENSIVE_DEBUG_CHECKS
    // Assertion that our selection doesn't contain things that aren't in the graph
    Q_ASSERT(std::all_of(_selectedNodes.begin(), _selectedNodes.end(),
        [this](ElementId<NodeId> nodeId)
        {
            auto& nodeIds = _graph.nodeIds();
            return std::find(nodeIds.begin(), nodeIds.end(), nodeId) != nodeIds.end();
        }));
#endif

    return _selectedNodes;
}

ElementIdSet<NodeId> SelectionManager::unselectedNodes() const
{
    auto& nodeIds = _graph.nodeIds();
    auto unselectedNodeIds = ElementIdSet<NodeId>(nodeIds.begin(), nodeIds.end());
    unselectedNodeIds.erase(_selectedNodes.begin(), _selectedNodes.end());

    return unselectedNodeIds;
}

bool SelectionManager::selectNode(NodeId nodeId)
{
    auto result = _selectedNodes.insert(nodeId);

    if(result.second)
        emit selectionChanged(this);

    return result.second;
}

bool SelectionManager::selectNodes(const ElementIdSet<NodeId>& nodeIds)
{
    return selectNodes(nodeIds.begin(), nodeIds.end());
}

template<typename InputIterator> bool SelectionManager::selectNodes(InputIterator first,
                                                                    InputIterator last)
{
    auto oldSize = _selectedNodes.size();
    _selectedNodes.insert(first, last);
    bool selectionDidChange = _selectedNodes.size() > oldSize;

    if(selectionDidChange)
        emit selectionChanged(this);

    return selectionDidChange;
}

bool SelectionManager::deselectNode(NodeId nodeId)
{
    bool selectionWillChange = _selectedNodes.erase(nodeId) > 0;

    if(selectionWillChange)
        emit selectionChanged(this);

    return selectionWillChange;
}

bool SelectionManager::deselectNodes(const ElementIdSet<NodeId>& nodeIds)
{
    return deselectNodes(nodeIds.begin(), nodeIds.end());
}

template<typename InputIterator> bool SelectionManager::deselectNodes(InputIterator first,
                                                                      InputIterator last)
{
    bool selectionWillChange = _selectedNodes.erase(first, last) != first;

    if(selectionWillChange)
        emit selectionChanged(this);

    return selectionWillChange;
}

void SelectionManager::toggleNode(NodeId nodeId)
{
    if(nodeIsSelected(nodeId))
        deselectNode(nodeId);
    else
        selectNode(nodeId);
}

void SelectionManager::toggleNodes(const ElementIdSet<NodeId>& nodeIds)
{
    toggleNodes(nodeIds.begin(), nodeIds.end());
}

template<typename InputIterator> void SelectionManager::toggleNodes(InputIterator first,
                                                                    InputIterator last)
{
    ElementIdSet<NodeId> difference;
    for(auto i = first; i != last; i++)
    {
        auto nodeId = *i;
        if(_selectedNodes.find(nodeId) == _selectedNodes.end())
            difference.insert(nodeId);
    }

    _selectedNodes = std::move(difference);

    emit selectionChanged(this);
}

bool SelectionManager::nodeIsSelected(NodeId nodeId) const
{
#ifdef EXPENSIVE_DEBUG_CHECKS
    Q_ASSERT(std::find(_graph.nodeIds().begin(),
                       _graph.nodeIds().end(),
                       nodeId) != _graph.nodeIds().end());
#endif
    return _selectedNodes.find(nodeId) != _selectedNodes.end();
}

bool SelectionManager::setSelectedNodes(const ElementIdSet<NodeId>& nodeIds)
{
    bool selectionWillChange = (_selectedNodes != nodeIds);
    _selectedNodes = std::move(nodeIds);

    if(selectionWillChange)
        emit selectionChanged(this);

    return selectionWillChange;
}

bool SelectionManager::selectAllNodes()
{
    auto& nodeIds = _graph.nodeIds();
    return selectNodes(nodeIds.begin(), nodeIds.end());
}

bool SelectionManager::clearNodeSelection()
{
    bool selectionWillChange = !_selectedNodes.empty();
    _selectedNodes.clear();

    if(selectionWillChange)
        emit selectionChanged(this);

    return selectionWillChange;
}

void SelectionManager::invertNodeSelection()
{
    auto& nodeIds = _graph.nodeIds();
    toggleNodes(nodeIds.begin(), nodeIds.end());
}

const QString SelectionManager::numNodesSelectedAsString() const
{
    int selectionSize = static_cast<int>(selectedNodes().size());
    if(selectionSize > 1)
        return QString(tr("%1 Nodes Selected")).arg(selectionSize);
    else if(selectionSize == 1)
        return tr("1 Node Selected");

    return QString();
}
