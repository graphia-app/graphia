/* Copyright © 2013-2021 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

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

    QString debugDescription() const override;

    bool execute() override;
    void undo() override;

    void replaces(const ICommand* replacee) override;
};

#endif // APPLYVISUALISATIONSSCOMMAND_H
