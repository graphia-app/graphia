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

void ApplyVisualisationsCommand::apply(const QStringList& visualisations,
                                       const QStringList& previousVisualisations)
{
    _graphModel->buildVisualisations(visualisations);

    _document->executeOnMainThreadAndWait(
    [this, newVisualisations = cancelled() ? previousVisualisations : visualisations]
    {
        _document->setVisualisations(newVisualisations);
    }, QStringLiteral("setVisualisations"));
}

bool ApplyVisualisationsCommand::execute()
{
    apply(_visualisations, _previousVisualisations);
    return true;
}

void ApplyVisualisationsCommand::undo()
{
    apply(_previousVisualisations, _visualisations);
}
