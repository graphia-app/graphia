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

#include "removeattributescommand.h"

#include "app/graph/graphmodel.h"

#include "shared/loading/userelementdata.h"

#include <algorithm>
#include <iterator>

using namespace Qt::Literals::StringLiterals;

RemoveAttributesCommand::RemoveAttributesCommand(GraphModel* graphModel, const QStringList& attributeNames) :
    _graphModel(graphModel)
{
    std::transform(attributeNames.cbegin(), attributeNames.cend(),
        std::back_inserter(_attributeNames),
    [](const auto& attributeName)
    {
        return Attribute::parseAttributeName(attributeName)._name;
    });
}

QString RemoveAttributesCommand::description() const
{
    return _attributeNames.size() > 1 ? QObject::tr("Remove Attributes") : QObject::tr("Remove Attribute");
}

QString RemoveAttributesCommand::verb() const
{
    return _attributeNames.size() > 1 ? QObject::tr("Removing Attributes") : QObject::tr("Removing Attribute");
}

QString RemoveAttributesCommand::pastParticiple() const
{
    return _attributeNames.size() > 1 ?
        QObject::tr("%1 Attributes Removed").arg(_attributeNames.size()) :
        QObject::tr("Attribute %1 Removed").arg(_attributeNames.front());
}

QString RemoveAttributesCommand::debugDescription() const
{
    QString text = description();

    for(const auto& attributeName : _attributeNames)
        text.append(u"\n  %1"_s.arg(attributeName));

    return text;
}

bool RemoveAttributesCommand::execute()
{
    const AttributeChangesTracker tracker(_graphModel);

    for(const auto& attributeName : std::as_const(_attributeNames))
    {
        const auto* attribute = _graphModel->attributeByName(attributeName);

        Q_ASSERT(attribute->userDefined());
        if(!attribute->userDefined())
            continue;

        if(attribute->elementType() == ElementType::Node)
        {
            _removedNodeAttributeTypes[attributeName] = attribute->valueType();
            auto v = _graphModel->userNodeData().removeByAttributeName(attributeName);
            _removedUserNodeDataVectors.emplace_back(std::move(v));
        }
        else if(attribute->elementType() == ElementType::Edge)
        {
            _removedEdgeAttributeTypes[attributeName] = attribute->valueType();
            auto v = _graphModel->userEdgeData().removeByAttributeName(attributeName);
            _removedUserEdgeDataVectors.emplace_back(std::move(v));
        }

        _graphModel->removeAttribute(attributeName);
    }

    return true;
}

void RemoveAttributesCommand::undo()
{
    const AttributeChangesTracker tracker(_graphModel);

    for(auto&& vector : _removedUserNodeDataVectors)
        _graphModel->userNodeData().setVector(std::move(vector));

    for(auto&& vector : _removedUserEdgeDataVectors)
        _graphModel->userEdgeData().setVector(std::move(vector));

    _graphModel->userNodeData().exposeAsAttributes(*_graphModel);
    _graphModel->userEdgeData().exposeAsAttributes(*_graphModel);

    for(const auto& [attributeName, type] : _removedNodeAttributeTypes)
    {
        _graphModel->userNodeData().setAttributeType(*_graphModel,
            attributeName, UserDataVector::equivalentTypeFor(type));
    }

    for(const auto& [attributeName, type] : _removedEdgeAttributeTypes)
    {
        _graphModel->userEdgeData().setAttributeType(*_graphModel,
            attributeName, UserDataVector::equivalentTypeFor(type));
    }

    _removedUserNodeDataVectors.clear();
    _removedNodeAttributeTypes.clear();
    _removedUserEdgeDataVectors.clear();
    _removedEdgeAttributeTypes.clear();
}
