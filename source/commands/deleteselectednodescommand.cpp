#include "deleteselectednodescommand.h"

#include <QObject>

#include "../graph/graphmodel.h"
#include "../ui/selectionmanager.h"

DeleteSelectedNodesCommand::DeleteSelectedNodesCommand(std::shared_ptr<GraphModel> graphModel,
                                       std::shared_ptr<SelectionManager> selectionManager) :
    Command(),
    _graphModel(graphModel),
    _selectionManager(selectionManager),
    _nodes(_selectionManager->selectedNodes())
{
    int numSelectedNodes = _selectionManager->selectedNodes().size();

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
    _edges = _graphModel->graph().edgesForNodes(_nodes);
    _selectionManager->clearNodeSelection(false);
    _graphModel->graph().removeNodes(_nodes);
    return true;
}

void DeleteSelectedNodesCommand::undo()
{
    _graphModel->graph().performTransaction(
        [this](Graph& graph)
        {
            graph.addNodes(_nodes);
            graph.addEdges(_edges);
        });

    _selectionManager->selectNodes(_nodes);
}
