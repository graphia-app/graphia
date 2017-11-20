#ifndef SELECTIONMANAGER_H
#define SELECTIONMANAGER_H

#include "graph/graphmodel.h"
#include "shared/ui/iselectionmanager.h"
#include "shared/utils/container.h"

#include <QObject>

#include <memory>

class SelectionManager : public QObject, public ISelectionManager
{
    Q_OBJECT
public:
    explicit SelectionManager(const GraphModel& graphModel);

    NodeIdSet selectedNodes() const override;
    NodeIdSet unselectedNodes() const override;

    bool selectNode(NodeId nodeId) override;

    //FIXME http://en.cppreference.com/w/cpp/container/unordered_set/merge will be useful here
    template<typename C> bool selectNodes(const C& nodeIds, bool selectMergedNodes = true)
    {
        NodeIdSet newSelectedNodeIds;

        if(selectMergedNodes)
        {
            for(auto nodeId : nodeIds)
            {
                if(_graphModel->graph().typeOf(nodeId) == MultiElementType::Tail)
                    continue;

                auto mergedNodeIds = _graphModel->graph().mergedNodeIdsForNodeId(nodeId);

                for(auto mergedNodeId : mergedNodeIds)
                    newSelectedNodeIds.insert(mergedNodeId);
            }
        }
        else
        {
            for(auto nodeId : nodeIds)
                newSelectedNodeIds.insert(nodeId);
        }

        auto oldSize = _selectedNodeIds.size();
        _selectedNodeIds.insert(newSelectedNodeIds.begin(), newSelectedNodeIds.end());
        bool selectionWillChange = _selectedNodeIds.size() > oldSize;

        if(selectionWillChange && !signalsSuppressed())
            emit selectionChanged(this);

        return selectionWillChange;
    }

    bool selectNodes(const NodeIdSet& nodeIds) override
    {
        return selectNodes(nodeIds, true);
    }

    bool deselectNode(NodeId nodeId) override;

    template<typename C> bool deselectNodes(const C& nodeIds, bool deselectMergedNodes = true)
    {
        bool selectionWillChange = false;

        if(deselectMergedNodes)
        {
            for(auto nodeId : nodeIds)
            {
                if(_graphModel->graph().typeOf(nodeId) == MultiElementType::Tail)
                    continue;

                auto mergedNodeIds = _graphModel->graph().mergedNodeIdsForNodeId(nodeId);

                for(auto mergedNodeId : mergedNodeIds)
                    selectionWillChange |= (_selectedNodeIds.erase(mergedNodeId) > 0);
            }
        }
        else
        {
            for(auto nodeId : nodeIds)
                selectionWillChange |= (_selectedNodeIds.erase(nodeId) > 0);
        }

        if(selectionWillChange && !signalsSuppressed())
            emit selectionChanged(this);

        return selectionWillChange;
    }

    bool deselectNodes(const NodeIdSet& nodeIds) override
    {
        return deselectNodes(nodeIds, true);
    }

    void toggleNode(NodeId nodeId);

    template<typename C> void toggleNodes(const C& nodeIds)
    {
        NodeIdSet difference;
        for(auto nodeId : nodeIds)
        {
            if(!u::contains(_selectedNodeIds, nodeId))
                difference.insert(nodeId);
        }

        _selectedNodeIds = std::move(difference);

        if(!signalsSuppressed())
            emit selectionChanged(this);
    }

    bool nodeIsSelected(NodeId nodeId) const override;

    bool setSelectedNodes(const NodeIdSet& nodeIds) override
    {
        bool selectionWillChange = u::setsDiffer(_selectedNodeIds, nodeIds);
        _selectedNodeIds = nodeIds;

        if(selectionWillChange && !signalsSuppressed())
            emit selectionChanged(this);

        return selectionWillChange;
    }

    bool selectAllNodes() override;
    bool clearNodeSelection() override;
    void invertNodeSelection() override;

    int numNodesSelected() const { return static_cast<int>(_selectedNodeIds.size()); }
    QString numNodesSelectedAsString() const;

    void suppressSignals();
    bool signalsSuppressed();

private:
    const GraphModel* _graphModel = nullptr;

    NodeIdSet _selectedNodeIds;

    // Temporary storage for NodeIds that have been deleted
    std::vector<NodeId> _deletedNodes;

    bool _suppressSignals = false;

signals:
    void selectionChanged(const SelectionManager* selectionManager) const;
};

#endif // SELECTIONMANAGER_H
