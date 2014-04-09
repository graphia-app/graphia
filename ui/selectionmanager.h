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

    QSet<NodeId> selectedNodes();
    QSet<NodeId> unselectedNodes();

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

signals:
    void selectionChanged() const;
};

#endif // SELECTIONMANAGER_H
