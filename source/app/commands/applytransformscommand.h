/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#ifndef APPLYTRANSFORMSCOMMAND_H
#define APPLYTRANSFORMSCOMMAND_H

#include "shared/commands/icommand.h"

#include "shared/graph/elementid.h"
#include "shared/graph/elementid_containers.h"

#include <QStringList>

class GraphModel;
class Document;

class ApplyTransformsCommand : public ICommand
{
private:
    GraphModel* _graphModel = nullptr;
    Document* _document = nullptr;

    QStringList _previousTransformations;
    QStringList _transformations;

    void doTransform(const QStringList& transformations,
                     const QStringList& previousTransformations);

public:
    ApplyTransformsCommand(GraphModel* graphModel,
                           Document* document,
                           QStringList previousTransformations,
                           QStringList transformations);

    QString description() const override;
    QString verb() const override;

    QString debugDescription() const override;

    bool execute() override;
    void undo() override;

    void cancel() override;
    bool cancellable() const override { return true; }
};

#endif // APPLYTRANSFORMSCOMMAND_H
