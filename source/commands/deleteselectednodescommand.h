#ifndef DELETENODESCOMMAND_H
#define DELETENODESCOMMAND_H

#include "command.h"

#include "../graph/graph.h"

#include <memory>

class GraphModel;
class SelectionManager;

class DeleteSelectedNodesCommand : public Command
{
private:
    std::shared_ptr<GraphModel> _graphModel;
    std::shared_ptr<SelectionManager> _selectionManager;

    const ElementIdSet<NodeId> _nodes;
    std::vector<Edge> _edges;


public:
    DeleteSelectedNodesCommand(std::shared_ptr<GraphModel> graphModel,
                               std::shared_ptr<SelectionManager> selectionManager);

    bool execute();
    void undo();
};

#endif // DELETENODESCOMMAND_H
