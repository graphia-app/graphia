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
    GraphModel* _graphModel;
    Document* _document;

    QStringList _previousVisualisations;
    QStringList _visualisations;

    void doTransform(const QStringList& visualisations);

public:
    ApplyVisualisationsCommand(GraphModel* graphModel,
                               Document* document,
                               const QStringList& previousVisualisations,
                               const QStringList& visualisations);

    QString description() const;
    QString verb() const;

    bool execute();
    void undo();
};

#endif // APPLYVISUALISATIONSSCOMMAND_H
