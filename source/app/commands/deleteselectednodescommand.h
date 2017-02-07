#ifndef DELETENODESCOMMAND_H
#define DELETENODESCOMMAND_H

#include "shared/commands/icommand.h"

#include "graph/graph.h"

#include <vector>

class GraphModel;
class SelectionManager;

class DeleteSelectedNodesCommand : public ICommand
{
private:
    GraphModel* _graphModel;
    SelectionManager* _selectionManager;

    int _numSelectedNodes = 0;
    const NodeIdSet _nodeIds;
    std::vector<Edge> _edges;

public:
    DeleteSelectedNodesCommand(GraphModel* graphModel,
                               SelectionManager* selectionManager);

    QString description() const;
    QString verb() const;
    QString pastParticiple() const;

    bool execute();
    void undo();
};

#endif // DELETENODESCOMMAND_H
