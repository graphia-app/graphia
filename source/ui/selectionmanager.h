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
        return selectNodes(nodeIds.begin(), nodeIds.end());
    }

    template<typename InputIterator> bool selectNodes(InputIterator first,
                                                      InputIterator last)
    {
        auto oldSize = _selectedNodeIds.size();
        _selectedNodeIds.insert(first, last);
        bool selectionWillChange = _selectedNodeIds.size() > oldSize;

        if(selectionWillChange)
            emit selectionChanged(this);

        return selectionWillChange;
    }

    bool deselectNode(NodeId nodeId);

    template<typename C> bool deselectNodes(const C& nodeIds)
    {
        return deselectNodes(nodeIds.begin(), nodeIds.end());
    }

    template<typename InputIterator> bool deselectNodes(InputIterator first,
                                                        InputIterator last)
    {
        bool selectionWillChange = _selectedNodeIds.erase(first, last) != first;

        if(selectionWillChange)
            emit selectionChanged(this);

        return selectionWillChange;
    }

    void toggleNode(NodeId nodeId);

    template<typename C> void toggleNodes(const C& nodeIds)
    {
        toggleNodes(nodeIds.begin(), nodeIds.end());
    }

    template<typename InputIterator> void toggleNodes(InputIterator first,
                                                      InputIterator last)
    {
        NodeIdSet difference;
        for(auto i = first; i != last; ++i)
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
