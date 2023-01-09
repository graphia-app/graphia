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

#include "editattributecommand.h"

#include "graph/graphmodel.h"

EditAttributeCommand::EditAttributeCommand(GraphModel* graphModel, const QString& attributeName,
    const AttributeEdits& edits) :
    _graphModel(graphModel), _attributeName(attributeName), _edits(edits)
{}

QString EditAttributeCommand::description() const
{
    return QObject::tr("Edit Attribute");
}

QString EditAttributeCommand::verb() const
{
    return QObject::tr("Editing Attribute");
}

QString EditAttributeCommand::pastParticiple() const
{
    return QObject::tr("Attribute %1 Edited").arg(Attribute::prettify(_attributeName));
}

QString EditAttributeCommand::debugDescription() const
{
    auto text = QStringLiteral("%1 %2").arg(description(), _attributeName);

    auto appendDetails = [&text](const auto& edits, auto& reverseEdits)
    {
        int maxEdits = 10;

        for(const auto& [elementId, value] : edits)
        {
            auto old = reverseEdits.at(elementId);
            text += QStringLiteral("\n  %1 -> %2").arg(old, value);

            if(--maxEdits == 0)
            {
                text += QStringLiteral("\n  ...");
                break;
            }
        }
    };

    const auto* attribute = _graphModel->attributeByName(_attributeName);

    if(attribute->elementType() == ElementType::Node)
        appendDetails(_edits._nodeValues, _reverseEdits._nodeValues);
    else if(attribute->elementType() == ElementType::Edge)
        appendDetails(_edits._edgeValues, _reverseEdits._edgeValues);

    return text;
}

bool EditAttributeCommand::execute()
{
    const AttributeChangesTracker tracker(_graphModel);

    const auto* attribute = _graphModel->attributeByName(_attributeName);

    Q_ASSERT(attribute != nullptr);
    if(attribute == nullptr)
        return false;

    auto doEdits = [attribute](const auto& edits, auto& reverseEdits)
    {
        for(const auto& [elementId, value] : edits)
        {
            reverseEdits[elementId] = attribute->stringValueOf(elementId);
            attribute->setValueOf(elementId, value);
        }
    };

    if(attribute->elementType() == ElementType::Node)
        doEdits(_edits._nodeValues, _reverseEdits._nodeValues);
    else if(attribute->elementType() == ElementType::Edge)
        doEdits(_edits._edgeValues, _reverseEdits._edgeValues);

    return true;
}

void EditAttributeCommand::undo()
{
    const AttributeChangesTracker tracker(_graphModel);

    const auto* attribute = _graphModel->attributeByName(_attributeName);

    auto undoEdits = [attribute](auto& reverseEdits)
    {
        for(const auto& [elementId, value] : reverseEdits)
            attribute->setValueOf(elementId, value);

        reverseEdits.clear();
    };

    if(attribute->elementType() == ElementType::Node)
        undoEdits(_reverseEdits._nodeValues);
    else if(attribute->elementType() == ElementType::Edge)
        undoEdits(_reverseEdits._edgeValues);
}
