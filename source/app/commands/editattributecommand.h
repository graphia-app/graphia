/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

#ifndef EDITATTRIBUTECOMMAND_H
#define EDITATTRIBUTECOMMAND_H

#include "app/attributes/attributeedits.h"

#include "shared/commands/icommand.h"
#include "shared/attributes/valuetype.h"

#include <QString>

class GraphModel;

class EditAttributeCommand : public ICommand
{
private:
    GraphModel* _graphModel = nullptr;

    QString _attributeName;
    AttributeEdits _edits;
    ValueType _newType = ValueType::Unknown;
    AttributeEdits _reverseEdits;
    ValueType _originalType = ValueType::Unknown;

public:
    EditAttributeCommand(GraphModel* graphModel, const QString& attributeName,
        const AttributeEdits& edits, ValueType newType = ValueType::Unknown);

    QString description() const override;
    QString verb() const override;
    QString pastParticiple() const override;

    QString debugDescription() const override;

    bool execute() override;
    void undo() override;
};

#endif // EDITATTRIBUTECOMMAND_H
