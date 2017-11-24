#ifndef SELECTIONMANAGER_H
#define SELECTIONMANAGER_H

#include "shared/ui/iselectionmanager.h"
#include "shared/utils/container.h"

#include <QObject>

#include <memory>

class GraphModel;

class SelectionManager : public QObject, public ISelectionManager
{
    Q_OBJECT
public:
    explicit SelectionManager(const GraphModel& graphModel);

    NodeIdSet selectedNodes() const override;
    NodeIdSet unselectedNodes() const override;

    bool selectNode(NodeId nodeId) override;
    bool selectNodes(const NodeIdSet& nodeIds) override;
    bool selectNodes(const std::vector<NodeId>& nodeIds);

    bool deselectNode(NodeId nodeId) override;
    bool deselectNodes(const NodeIdSet& nodeIds) override;
    bool deselectNodes(const std::vector<NodeId>& nodeIds);

    void toggleNode(NodeId nodeId);

    bool nodeIsSelected(NodeId nodeId) const override;

    bool setSelectedNodes(const NodeIdSet& nodeIds) override;

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

    template<typename Fn>
    bool callFnAndMaybeEmit(Fn&& fn)
    {
        bool selectionWillChange = fn();

        if(!signalsSuppressed() && selectionWillChange)
            emit selectionChanged(this);

        return selectionWillChange;
    }

signals:
    void selectionChanged(const SelectionManager* selectionManager) const;
};

#endif // SELECTIONMANAGER_H
