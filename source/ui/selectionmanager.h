#ifndef SELECTIONMANAGER_H
#define SELECTIONMANAGER_H

#include "../graph/graph.h"

#include <QObject>
#include <QSet>

class SelectionManager : public QObject
{
    Q_OBJECT
public:
    SelectionManager(const ReadOnlyGraph& graph) : _graph(&graph) {}

    QSet<NodeId> selectedNodes() const;
    QSet<NodeId> unselectedNodes() const;

    bool selectNode(NodeId nodeId);
    bool selectNodes(const QSet<NodeId>& nodeIds);

    bool deselectNode(NodeId nodeId);
    bool deselectNodes(const QSet<NodeId>& nodeIds);

    void toggleNode(NodeId nodeId);
    void toggleNodes(const QSet<NodeId>& nodeIds);

    bool nodeIsSelected(NodeId nodeId) const;

    bool setSelectedNodes(const QSet<NodeId>& nodeIds);

    bool selectAllNodes();
    bool clearNodeSelection();
    void invertNodeSelection();

private:
    const ReadOnlyGraph* _graph;

    QSet<NodeId> _selectedNodes;

signals:
    void selectionChanged(const SelectionManager& selectionManager) const;
};

#endif // SELECTIONMANAGER_H
