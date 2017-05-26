#include "applyvisualisationscommand.h"

#include <QObject>

#include "graph/graphmodel.h"
#include "ui/document.h"

ApplyVisualisationsCommand::ApplyVisualisationsCommand(GraphModel* graphModel,
                                                       Document* document,
                                                       QStringList previousVisualisations,
                                                       QStringList visualisations) :
    _graphModel(graphModel),
    _document(document),
    _previousVisualisations(std::move(previousVisualisations)),
    _visualisations(std::move(visualisations))
{}

QString ApplyVisualisationsCommand::description() const
{
    return QObject::tr("Apply Visualisations");
}

QString ApplyVisualisationsCommand::verb() const
{
    return QObject::tr("Applying Visualisations");
}

void ApplyVisualisationsCommand::apply(const QStringList& visualisations)
{
    _graphModel->buildVisualisations(visualisations);

    _document->executeOnMainThread([this, visualisations]
    {
        _document->setVisualisations(visualisations);
    }, "setVisualisations");
}

bool ApplyVisualisationsCommand::execute()
{
    apply(_visualisations);
    return true;
}

void ApplyVisualisationsCommand::undo()
{
    apply(_previousVisualisations);
}
