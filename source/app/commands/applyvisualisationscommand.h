#ifndef APPLYVISUALISATIONSSCOMMAND_H
#define APPLYVISUALISATIONSSCOMMAND_H

#include "shared/commands/icommand.h"

#include "shared/graph/elementid.h"

#include <QStringList>

class GraphModel;
class Document;

class ApplyVisualisationsCommand : public ICommand
{
private:
    GraphModel* _graphModel = nullptr;
    Document* _document = nullptr;

    QStringList _previousVisualisations;
    QStringList _visualisations;

    void apply(const QStringList& visualisations,
               const QStringList& previousVisualisations);

public:
    ApplyVisualisationsCommand(GraphModel* graphModel,
                               Document* document,
                               QStringList previousVisualisations,
                               QStringList visualisations);

    QString description() const override;
    QString verb() const override;

    bool execute() override;
    void undo() override;
};

#endif // APPLYVISUALISATIONSSCOMMAND_H
