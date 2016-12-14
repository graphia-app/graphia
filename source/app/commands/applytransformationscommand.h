#ifndef APPLYTRANSFORMATIONSCOMMAND_H
#define APPLYTRANSFORMATIONSCOMMAND_H

#include "command.h"

#include "shared/graph/elementid.h"

#include <QStringList>

class GraphModel;
class SelectionManager;
class Document;

class ApplyTransformationsCommand : public Command
{
private:
    GraphModel* _graphModel;
    SelectionManager* _selectionManager;
    Document* _document;

    QStringList _previousTransformations;
    QStringList _transformations;

    const NodeIdSet _selectedNodeIds;

    void doTransform(const QStringList& transformations);

public:
    ApplyTransformationsCommand(GraphModel* graphModel,
                                SelectionManager* selectionManager,
                                Document* document,
                                const QStringList& previousTransformations,
                                const QStringList& transformations);

    bool execute();
    void undo();
};

#endif // APPLYTRANSFORMATIONSCOMMAND_H
