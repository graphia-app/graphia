#include "deleteselectednodescommand.h"

#include <QObject>

#include "../graph/mutablegraph.h"
#include "../graph/graphmodel.h"
#include "../ui/selectionmanager.h"

DeleteSelectedNodesCommand::DeleteSelectedNodesCommand(std::shared_ptr<GraphModel> graphModel,
                                                       std::shared_ptr<SelectionManager> selectionManager) :
    Command(),
    _graphModel(graphModel),
    _selectionManager(selectionManager),
    _nodeIds(_selectionManager->selectedNodes())
{
    int numSelectedNodes = static_cast<int>(_selectionManager->selectedNodes().size());

    if(numSelectedNodes > 1)
    {
        setDescription(QObject::tr("Delete Nodes"));
        setVerb(QObject::tr("Deleting Nodes"));
        setPastParticiple(QString(QObject::tr("%1 Nodes Deleted")).arg(numSelectedNodes));
    }
    else
    {
        setDescription(QObject::tr("Delete Node"));
        setVerb(QObject::tr("Deleting Node"));
        setPastParticiple(QObject::tr("Node Deleted"));
    }
}

bool DeleteSelectedNodesCommand::execute()
{
    _edges = _graphModel->mutableGraph().edgesForNodes(_nodes);
    _selectionManager->clearNodeSelection();
    _graphModel->mutableGraph().removeNodes(_nodeIds);
    return true;
}

void DeleteSelectedNodesCommand::undo()
{
    _graphModel->mutableGraph().performTransaction(
        [this](MutableGraph& graph)
        {
            graph.addNodes(_nodeIds);
            graph.addEdges(_edges);
        });

    _selectionManager->selectNodes(_nodeIds);
}
