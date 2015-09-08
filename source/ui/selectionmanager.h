#ifndef SELECTIONMANAGER_H
#define SELECTIONMANAGER_H

#include "../graph/graphmodel.h"

#include <QObject>

#include <memory>

class SelectionManager : public QObject
{
    Q_OBJECT
public:
    SelectionManager(const GraphModel& graph);

    NodeIdSet selectedNodes() const;
    NodeIdSet unselectedNodes() const;

    bool selectNode(NodeId nodeId);
    bool selectNodes(const NodeIdSet& nodeIds);
    template<typename InputIterator> bool selectNodes(InputIterator first,
                                                      InputIterator last);

    bool deselectNode(NodeId nodeId);
    bool deselectNodes(const NodeIdSet& nodeIds);
    template<typename InputIterator> bool deselectNodes(InputIterator first,
                                                        InputIterator last);

    void toggleNode(NodeId nodeId);
    void toggleNodes(const NodeIdSet& nodeIds);
    template<typename InputIterator> void toggleNodes(InputIterator first,
                                                      InputIterator last);

    bool nodeIsSelected(NodeId nodeId) const;

    bool setSelectedNodes(const NodeIdSet& nodeIds);

    bool selectAllNodes();
    bool clearNodeSelection();
    void invertNodeSelection();

    const QString numNodesSelectedAsString() const;

private:
    const GraphModel& _graphModel;

    NodeIdSet _selectedNodes;

signals:
    void selectionChanged(const SelectionManager* selectionManager) const;
};

#endif // SELECTIONMANAGER_H
