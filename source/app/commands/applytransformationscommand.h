#ifndef APPLYTRANSFORMATIONSCOMMAND_H
#define APPLYTRANSFORMATIONSCOMMAND_H

#include "shared/commands/icommand.h"

#include "shared/graph/elementid.h"

#include <QStringList>

class GraphModel;
class SelectionManager;
class Document;

class ApplyTransformationsCommand : public ICommand
{
private:
    GraphModel* _graphModel = nullptr;
    SelectionManager* _selectionManager = nullptr;
    Document* _document = nullptr;

    QStringList _previousTransformations;
    QStringList _transformations;

    const NodeIdSet _selectedNodeIds;

    void doTransform(const QStringList& transformations);

public:
    ApplyTransformationsCommand(GraphModel* graphModel,
                                SelectionManager* selectionManager,
                                Document* document,
                                QStringList previousTransformations,
                                QStringList transformations);

    QString description() const;
    QString verb() const;

    bool execute();
    void undo();
};

#endif // APPLYTRANSFORMATIONSCOMMAND_H
