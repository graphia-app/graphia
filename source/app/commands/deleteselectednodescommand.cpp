#include "deleteselectednodescommand.h"

#include <QObject>

#include "graph/mutablegraph.h"
#include "graph/graphmodel.h"
#include "ui/selectionmanager.h"

DeleteSelectedNodesCommand::DeleteSelectedNodesCommand(GraphModel* graphModel,
                                                       SelectionManager* selectionManager) :
    _graphModel(graphModel),
    _selectionManager(selectionManager),
    _nodeIds(_selectionManager->selectedNodes())
{
    _numSelectedNodes = static_cast<int>(_selectionManager->selectedNodes().size());
}

QString DeleteSelectedNodesCommand::description() const
{
    return _numSelectedNodes > 1 ? QObject::tr("Delete Nodes") : QObject::tr("Delete Node");
}

QString DeleteSelectedNodesCommand::verb() const
{
    return _numSelectedNodes > 1 ? QObject::tr("Deleting Nodes") : QObject::tr("Deleting Node");
}

QString DeleteSelectedNodesCommand::pastParticiple() const
{
    return _numSelectedNodes > 1 ? QString(QObject::tr("%1 Nodes Deleted")).arg(_numSelectedNodes) :
                                   QObject::tr("Node Deleted");
}

bool DeleteSelectedNodesCommand::execute()
{
    _edges = _graphModel->mutableGraph().edgesForNodeIds(_nodeIds);
    _selectionManager->clearNodeSelection();
    _graphModel->mutableGraph().removeNodes(_nodeIds);
    return true;
}

void DeleteSelectedNodesCommand::undo()
{
    _graphModel->mutableGraph().performTransaction(
        [this](IMutableGraph& graph)
        {
            graph.addNodes(_nodeIds);
            graph.addEdges(_edges);
        });

    _selectionManager->selectNodes(_nodeIds);
}
