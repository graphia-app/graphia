#ifndef APPLYTRANSFORMATIONSCOMMAND_H
#define APPLYTRANSFORMATIONSCOMMAND_H

#include "command.h"

#include "../graph/graph.h"
#include "../ui/graphtransformconfiguration.h"

#include <vector>

class GraphModel;
class SelectionManager;
class Document;

class ApplyTransformationsCommand : public Command
{
private:
    GraphModel* _graphModel;
    SelectionManager* _selectionManager;
    Document* _document;

    std::vector<GraphTransformConfiguration> _previousTransformations;
    std::vector<GraphTransformConfiguration> _transformations;

    const NodeIdSet _selectedNodeIds;

    void doTransform(const std::vector<GraphTransformConfiguration>& transformations);

public:
    ApplyTransformationsCommand(GraphModel* graphModel,
                                SelectionManager* selectionManager,
                                Document* document,
                                const std::vector<GraphTransformConfiguration>& previousTransformations,
                                const std::vector<GraphTransformConfiguration>& transformations);

    bool execute();
    void undo();
};

#endif // APPLYTRANSFORMATIONSCOMMAND_H
