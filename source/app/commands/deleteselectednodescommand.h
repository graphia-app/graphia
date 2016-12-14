#ifndef DELETENODESCOMMAND_H
#define DELETENODESCOMMAND_H

#include "command.h"

#include "graph/graph.h"

#include <vector>

class GraphModel;
class SelectionManager;

class DeleteSelectedNodesCommand : public Command
{
private:
    GraphModel* _graphModel;
    SelectionManager* _selectionManager;

    const NodeIdSet _nodeIds;
    std::vector<Edge> _edges;

public:
    DeleteSelectedNodesCommand(GraphModel* graphModel,
                               SelectionManager* selectionManager);

    bool execute();
    void undo();
};

#endif // DELETENODESCOMMAND_H
