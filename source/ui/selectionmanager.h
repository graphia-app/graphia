#ifndef SELECTIONMANAGER_H
#define SELECTIONMANAGER_H

#include "../graph/graphmodel.h"
#include "../utils/utils.h"

#include <QObject>

#include <memory>

class SelectionManager : public QObject
{
    Q_OBJECT
public:
    SelectionManager(const GraphModel& graphModel);

    NodeIdSet selectedNodes() const;
    NodeIdSet unselectedNodes() const;

    bool selectNode(NodeId nodeId);

    template<typename C> bool selectNodes(const C& nodeIds)
    {
        NodeIdSet newSelectedNodeIds;

        for(auto nodeId : nodeIds)
        {
            auto multiNodeIds = _graphModel.graph().multiNodesForNodeId(nodeId);
            newSelectedNodeIds.insert(multiNodeIds.begin(), multiNodeIds.end());
        }

        bool selectionWillChange = u::setsDiffer(_selectedNodeIds, newSelectedNodeIds);
        _selectedNodeIds = newSelectedNodeIds;

        if(selectionWillChange)
            emit selectionChanged(this);

        return selectionWillChange;
    }

    bool deselectNode(NodeId nodeId);

    template<typename C> bool deselectNodes(const C& nodeIds)
    {
        bool selectionWillChange = _selectedNodeIds.erase(nodeIds.begin(), nodeIds.end()) != nodeIds.begin();

        if(selectionWillChange)
            emit selectionChanged(this);

        return selectionWillChange;
    }

    void toggleNode(NodeId nodeId);

    template<typename C> void toggleNodes(const C& nodeIds)
    {
        NodeIdSet difference;
        for(auto i = nodeIds.begin(); i != nodeIds.end(); ++i)
        {
            auto nodeId = *i;
            if(!u::contains(_selectedNodeIds, nodeId))
                difference.insert(nodeId);
        }

        _selectedNodeIds = std::move(difference);

        emit selectionChanged(this);
    }

    bool nodeIsSelected(NodeId nodeId) const;

    template<typename C> bool setSelectedNodes(const C& nodeIds)
    {
        bool selectionWillChange = u::setsDiffer(_selectedNodeIds, nodeIds);
        _selectedNodeIds = std::move(nodeIds);

        if(selectionWillChange)
            emit selectionChanged(this);

        return selectionWillChange;
    }

    bool selectAllNodes();
    bool clearNodeSelection();
    void invertNodeSelection();

    const QString numNodesSelectedAsString() const;

private:
    const GraphModel& _graphModel;

    NodeIdSet _selectedNodeIds;

signals:
    void selectionChanged(const SelectionManager* selectionManager) const;
};

#endif // SELECTIONMANAGER_H
