#include "deletenodescommand.h"

#include <QObject>

#include "graph/mutablegraph.h"
#include "graph/graphmodel.h"
#include "ui/selectionmanager.h"

DeleteNodesCommand::DeleteNodesCommand(GraphModel* graphModel,
                                       SelectionManager* selectionManager,
                                       const NodeIdSet& nodeIds) :
    _graphModel(graphModel),
    _selectionManager(selectionManager),
    _selectedNodeIds(_selectionManager->selectedNodes()),
    _nodeIds(nodeIds)
{
    _multipleNodes = (_nodeIds.size() > 1);
}

QString DeleteNodesCommand::description() const
{
    return _multipleNodes ? QObject::tr("Delete Nodes") : QObject::tr("Delete Node");
}

QString DeleteNodesCommand::verb() const
{
    return _multipleNodes ? QObject::tr("Deleting Nodes") : QObject::tr("Deleting Node");
}

QString DeleteNodesCommand::pastParticiple() const
{
    return _multipleNodes ? QString(QObject::tr("%1 Nodes Deleted")).arg(_nodeIds.size()) :
                                    QObject::tr("Node Deleted");
}

bool DeleteNodesCommand::execute()
{
    _edges = _graphModel->mutableGraph().edgesForNodeIds(_nodeIds);
    _selectionManager->deselectNodes(_nodeIds);
    _graphModel->mutableGraph().removeNodes(_nodeIds);
    return true;
}

void DeleteNodesCommand::undo()
{
    _graphModel->mutableGraph().performTransaction(
        [this](IMutableGraph& graph)
        {
            graph.addNodes(_nodeIds);
            graph.addEdges(_edges);
        });

    _selectionManager->selectNodes(_selectedNodeIds);
}
