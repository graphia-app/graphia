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

#ifndef REMOVEATTRIBUTESCOMMAND_H
#define REMOVEATTRIBUTESCOMMAND_H

#include "shared/commands/icommand.h"

#include "shared/loading/userelementdata.h"

#include <QString>
#include <QStringList>

#include <vector>

class GraphModel;

class RemoveAttributesCommand : public ICommand
{
private:
    GraphModel* _graphModel = nullptr;

    QStringList _attributeNames;
    std::vector<UserNodeData::RemovedVector> _removedUserNodeData;
    std::vector<UserEdgeData::RemovedVector> _removedUserEdgeData;

public:
    RemoveAttributesCommand(GraphModel* graphModel, const QStringList& attributeNames);

    QString description() const override;
    QString verb() const override;
    QString pastParticiple() const override;

    QString debugDescription() const override;

    bool execute() override;
    void undo() override;
};

#endif // REMOVEATTRIBUTESCOMMAND_H
