#ifndef ISELECTIONMANAGER_H
#define ISELECTIONMANAGER_H

#include "../graph/elementid.h"

class ISelectionManager
{
public:
    virtual ~ISelectionManager() = default;

    virtual NodeIdSet selectedNodes() const = 0;
    virtual NodeIdSet unselectedNodes() const = 0;

    virtual bool selectNode(NodeId nodeId) = 0;
    virtual bool selectNodes(const NodeIdSet& nodeIds) = 0;

    virtual bool deselectNode(NodeId nodeId) = 0;
    virtual bool deselectNodes(const NodeIdSet& nodeIds) = 0;

    virtual bool nodeIsSelected(NodeId nodeId) const = 0;

    virtual bool setSelectedNodes(const NodeIdSet& nodeIds) = 0;

    virtual bool selectAllNodes() = 0;
    virtual bool clearNodeSelection() = 0;
    virtual void invertNodeSelection() = 0;
};

#endif // ISELECTIONMANAGER_H
