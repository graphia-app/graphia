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

    // When visualisations are created as a result of a transform,
    // this is an index with which the GraphModel can be interrogated
    int _transformIndex = -1;

    void apply(const QStringList& visualisations,
               const QStringList& previousVisualisations);

    QStringList patchedVisualisations() const;

public:
    ApplyVisualisationsCommand(GraphModel* graphModel,
                               Document* document,
                               QStringList previousVisualisations,
                               QStringList visualisations,
                               int transformIndex = -1);

    QString description() const override;
    QString verb() const override;

    bool execute() override;
    void undo() override;
};

#endif // APPLYVISUALISATIONSSCOMMAND_H
