#ifndef SELECTIONMANAGER_H
#define SELECTIONMANAGER_H

#include "../graph/graph.h"

#include <QObject>

#include <memory>

class SelectionManager : public QObject
{
    Q_OBJECT
public:
    SelectionManager(const ReadOnlyGraph& graph);

    ElementIdSet<NodeId> selectedNodes() const;
    ElementIdSet<NodeId> unselectedNodes() const;

    bool selectNode(NodeId nodeId, bool notify = true);
    bool selectNodes(const ElementIdSet<NodeId>& nodeIds, bool notify = true);
    template<typename InputIterator> bool selectNodes(InputIterator first,
                                                      InputIterator last,
                                                      bool notify = true);

    bool deselectNode(NodeId nodeId, bool notify = true);
    bool deselectNodes(const ElementIdSet<NodeId>& nodeIds, bool notify = true);
    template<typename InputIterator> bool deselectNodes(InputIterator first,
                                                        InputIterator last,
                                                        bool notify = true);

    void toggleNode(NodeId nodeId, bool notify = true);
    void toggleNodes(const ElementIdSet<NodeId>& nodeIds, bool notify = true);
    template<typename InputIterator> void toggleNodes(InputIterator first,
                                                      InputIterator last,
                                                      bool notify = true);

    bool nodeIsSelected(NodeId nodeId) const;

    bool setSelectedNodes(const ElementIdSet<NodeId>& nodeIds, bool notify = true);

    bool selectAllNodes(bool notify = true);
    bool clearNodeSelection(bool notify = true);
    void invertNodeSelection(bool notify = true);

private:
    const ReadOnlyGraph& _graph;

    ElementIdSet<NodeId> _selectedNodes;

signals:
    void selectionChanged(const SelectionManager* selectionManager) const;
};

#endif // SELECTIONMANAGER_H
