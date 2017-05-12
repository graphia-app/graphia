#include "applytransformationscommand.h"

#include <QObject>

#include "graph/graphmodel.h"
#include "ui/selectionmanager.h"
#include "ui/document.h"

ApplyTransformationsCommand::ApplyTransformationsCommand(GraphModel* graphModel,
                                                         SelectionManager* selectionManager, Document* document,
                                                         QStringList previousTransformations,
                                                         QStringList transformations) :
    _graphModel(graphModel),
    _selectionManager(selectionManager),
    _document(document),
    _previousTransformations(std::move(previousTransformations)),
    _transformations(std::move(transformations)),
    _selectedNodeIds(_selectionManager->selectedNodes())
{}

QString ApplyTransformationsCommand::description() const
{
    return QObject::tr("Apply Transformations");
}

QString ApplyTransformationsCommand::verb() const
{
    return QObject::tr("Applying Transformations");
}

void ApplyTransformationsCommand::doTransform(const QStringList& transformations)
{
    _graphModel->buildTransforms(transformations, this);

    _document->executeOnMainThread([this, transformations]
    {
        _document->setTransforms(transformations);
    }, "setTransforms");
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
