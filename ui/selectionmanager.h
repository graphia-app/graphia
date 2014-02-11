#ifndef SELECTIONMANAGER_H
#define SELECTIONMANAGER_H

#include "../graph/graph.h"

#include <QSet>

class SelectionManager
{
public:
    SelectionManager(const ReadOnlyGraph& graph) : _graph(&graph) {}

    QSet<NodeId> selectedNodes();

    void selectNode(NodeId nodeId);
    void selectNodes(QSet<NodeId> nodeIds);

    void deselectNode(NodeId nodeId);
    void deselectNodes(QSet<NodeId> nodeIds);

    void toggleNode(NodeId nodeId);
    void toggleNodes(QSet<NodeId> nodeIds);

    bool nodeIsSelected(NodeId nodeId);

    void resetNodeSelection();

private:
    const ReadOnlyGraph* _graph;

    QSet<NodeId> _selectedNodes;
};

#endif // SELECTIONMANAGER_H
