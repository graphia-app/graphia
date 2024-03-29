/* Copyright © 2013-2024 Graphia Technologies Ltd.
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
#include "shared/attributes/valuetype.h"

#include <QVariant>

using namespace Qt::Literals::StringLiterals;

EditAttributeCommand::EditAttributeCommand(GraphModel* graphModel, const QString& attributeName,
    const AttributeEdits& edits, ValueType newType, const QString& description) :
    _graphModel(graphModel), _attributeName(attributeName), _edits(edits),
    _newType(newType), _newDescription(description)
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
    auto text = u"%1 %2"_s.arg(description(), _attributeName);

    if(_newType != ValueType::Unknown)
        text += u" %1"_s.arg(QVariant::fromValue(_newType).toString());

    auto appendDetails = [&text](const auto& edits, auto& reverseEdits)
    {
        int maxEdits = 10;

        for(const auto& [elementId, value] : edits)
        {
            auto old = reverseEdits.at(elementId);
            text += u"\n  %1 -> %2"_s.arg(old, value);

            if(--maxEdits == 0)
            {
                text += u"\n  ..."_s;
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

    auto doEdits = [this, attribute](const auto& edits, auto& reverseEdits, auto& userData)
    {
        if(_newType != ValueType::Unknown && _newType != attribute->valueType())
        {
            const auto* userDataVector = userData.vectorForAttributeName(_attributeName);
            if(userDataVector == nullptr)
                return false;

            const UserDataVector::Type type = UserDataVector::equivalentTypeFor(_newType);

            if(!userDataVector->canConvertTo(type))
                return false;

            _originalType = attribute->valueType();

            const bool success = userData.setAttributeType(*_graphModel, _attributeName, type);
            Q_ASSERT(success);
        }

        if(_newDescription != attribute->description())
        {
            _originalDescription = attribute->description();

            const bool success = userData.setAttributeDescription(*_graphModel, _attributeName, _newDescription);
            Q_ASSERT(success);
        }

        for(const auto& [elementId, value] : edits)
        {
            reverseEdits[elementId] = attribute->stringValueOf(elementId);
            attribute->setValueOf(elementId, value);
        }

        return true;
    };

    if(attribute->elementType() == ElementType::Node)
        return doEdits(_edits._nodeValues, _reverseEdits._nodeValues, _graphModel->userNodeData());

    if(attribute->elementType() == ElementType::Edge)
        return doEdits(_edits._edgeValues, _reverseEdits._edgeValues, _graphModel->userEdgeData());

    return false;
}

void EditAttributeCommand::undo()
{
    const AttributeChangesTracker tracker(_graphModel);

    const auto* attribute = _graphModel->attributeByName(_attributeName);

    auto undoEdits = [this, attribute](auto& reverseEdits, auto& userData)
    {
        for(const auto& [elementId, value] : reverseEdits)
            attribute->setValueOf(elementId, value);

        reverseEdits.clear();

        if(_originalType != ValueType::Unknown)
        {
            const UserDataVector::Type type = UserDataVector::equivalentTypeFor(_originalType);
            const bool success = userData.setAttributeType(*_graphModel, _attributeName, type);
            Q_ASSERT(success);
            _originalType = ValueType::Unknown;
        }

        const bool success = userData.setAttributeDescription(*_graphModel, _attributeName, _originalDescription);
        Q_ASSERT(success);
        _originalDescription = {};
    };

    if(attribute->elementType() == ElementType::Node)
        undoEdits(_reverseEdits._nodeValues, _graphModel->userNodeData());
    else if(attribute->elementType() == ElementType::Edge)
        undoEdits(_reverseEdits._edgeValues, _graphModel->userEdgeData());
}
