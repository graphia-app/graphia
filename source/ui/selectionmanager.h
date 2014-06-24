#ifndef SELECTIONMANAGER_H
#define SELECTIONMANAGER_H

#include "../graph/graph.h"

#include <QObject>

#include <memory>

class SelectionManager : public QObject
{
    Q_OBJECT
public:
    SelectionManager(std::shared_ptr<const ReadOnlyGraph> graph) : QObject(), _graph(graph) {}

    ElementIdSet<NodeId> selectedNodes() const;
    ElementIdSet<NodeId> unselectedNodes() const;

    bool selectNode(NodeId nodeId);
    bool selectNodes(const ElementIdSet<NodeId>& nodeIds);
    template<typename InputIterator> bool selectNodes(InputIterator first,
                                                      InputIterator last);

    bool deselectNode(NodeId nodeId);
    bool deselectNodes(const ElementIdSet<NodeId>& nodeIds);
    template<typename InputIterator> bool deselectNodes(InputIterator first,
                                                        InputIterator last);

    void toggleNode(NodeId nodeId);
    void toggleNodes(const ElementIdSet<NodeId>& nodeIds);
    template<typename InputIterator> void toggleNodes(InputIterator first,
                                                      InputIterator last);

    bool nodeIsSelected(NodeId nodeId) const;

    bool setSelectedNodes(const ElementIdSet<NodeId>& nodeIds);

    bool selectAllNodes();
    bool clearNodeSelection();
    void invertNodeSelection();

private:
    std::shared_ptr<const ReadOnlyGraph> _graph;

    ElementIdSet<NodeId> _selectedNodes;

signals:
    void selectionChanged(const SelectionManager& selectionManager) const;
};

#endif // SELECTIONMANAGER_H
