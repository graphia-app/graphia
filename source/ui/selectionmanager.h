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

    void selectNode(NodeId nodeId);
    void selectNodes(QSet<NodeId> nodeIds);

    void deselectNode(NodeId nodeId);
    void deselectNodes(QSet<NodeId> nodeIds);

    void toggleNode(NodeId nodeId);
    void toggleNodes(QSet<NodeId> nodeIds);

    bool nodeIsSelected(NodeId nodeId) const;

    void selectAllNodes();
    void clearNodeSelection();
    void invertNodeSelection();

private:
    const ReadOnlyGraph* _graph;

    QSet<NodeId> _selectedNodes;

signals:
    void selectionChanged(const SelectionManager& selectionManager) const;
};

#endif // SELECTIONMANAGER_H
