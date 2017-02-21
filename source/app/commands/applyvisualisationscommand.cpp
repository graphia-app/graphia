#include "applyvisualisationscommand.h"

#include <QObject>

#include "graph/graphmodel.h"
#include "ui/document.h"

ApplyVisualisationsCommand::ApplyVisualisationsCommand(GraphModel* graphModel,
                                                       Document* document,
                                                       const QStringList& previousVisualisations,
                                                       const QStringList& visualisations) :
    _graphModel(graphModel),
    _document(document),
    _previousVisualisations(previousVisualisations),
    _visualisations(visualisations)
{}

QString ApplyVisualisationsCommand::description() const
{
    return QObject::tr("Apply Visualisations");
}

QString ApplyVisualisationsCommand::verb() const
{
    return QObject::tr("Applying Visualisations");
}

void ApplyVisualisationsCommand::doTransform(const QStringList& visualisations)
{
    _graphModel->buildVisualisations(visualisations);

    // This needs to happen on the main thread
    QMetaObject::invokeMethod(_document, "setVisualisations", Q_ARG(const QStringList&, visualisations));
}

bool ApplyVisualisationsCommand::execute()
{
    doTransform(_visualisations);
    return true;
}

void ApplyVisualisationsCommand::undo()
{
    doTransform(_previousVisualisations);
}
