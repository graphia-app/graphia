#include "applytransformationscommand.h"

#include <QObject>

#include "graph/graphmodel.h"
#include "ui/selectionmanager.h"
#include "ui/document.h"

ApplyTransformationsCommand::ApplyTransformationsCommand(GraphModel* graphModel,
                                                         SelectionManager* selectionManager, Document* document,
                                                         const QStringList& previousTransformations,
                                                         const QStringList& transformations) :
    Command(),
    _graphModel(graphModel),
    _selectionManager(selectionManager),
    _document(document),
    _previousTransformations(previousTransformations),
    _transformations(transformations),
    _selectedNodeIds(_selectionManager->selectedNodes())
{
    setDescription(QObject::tr("Apply Transformations"));
    setVerb(QObject::tr("Applying Transformations"));
}

void ApplyTransformationsCommand::doTransform(const QStringList& transformations)
{
    _graphModel->buildTransforms(transformations);
    executeSynchronouslyOnCompletion([&](Command&) { _document->setTransforms(transformations); });
}

bool ApplyTransformationsCommand::execute()
{
    doTransform(_transformations);
    return true;
}

void ApplyTransformationsCommand::undo()
{
    doTransform(_previousTransformations);

    // Restore the selection to what it was prior to the transformation
    _selectionManager->selectNodes(_selectedNodeIds);
}
