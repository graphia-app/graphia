/* Copyright © 2013-2020 Graphia Technologies Ltd.
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

#include "importattributescommand.h"

#include "graph/graphmodel.h"
#include "graph/mutablegraph.h"

#include "shared/utils/string.h"
#include "shared/loading/userelementdata.h"

#include <map>

ImportAttributesCommand::ImportAttributesCommand(GraphModel* graphModel, const QString& keyAttributeName,
    TabularData* data, int keyColumnIndex, std::vector<int> importColumnIndices) :
    _graphModel(graphModel), _keyAttributeName(keyAttributeName), _data(std::move(*data)),
    _keyColumnIndex(keyColumnIndex), _importColumnIndices(std::move(importColumnIndices))
{
    _multipleAttributes = (_importColumnIndices.size() > 1);
}

QString ImportAttributesCommand::description() const
{
    return _multipleAttributes ? QObject::tr("Import Attributes") : QObject::tr("Import Attribute");
}

QString ImportAttributesCommand::verb() const
{
    return _multipleAttributes ? QObject::tr("Importing Attributes") : QObject::tr("Importing Attribute");
}

QString ImportAttributesCommand::pastParticiple() const
{
    return _multipleAttributes ?
        QObject::tr("%1 Attributes Imported").arg(_createdAttributeNames.size()) :
        QObject::tr("Attribute %1 Imported").arg(_createdAttributeNames.front());
}

QString ImportAttributesCommand::debugDescription() const
{
    QString text = description();

    for(const auto& attributeName : _createdAttributeNames)
        text.append(QStringLiteral("\n  %1").arg(attributeName));

    return text;
}

bool ImportAttributesCommand::execute()
{
    auto tracker = _graphModel->attributeChangesTracker();

    const auto* keyAttribute = _graphModel->attributeByName(_keyAttributeName);
    Q_ASSERT(keyAttribute != nullptr);

    auto createAttributes = [this, keyAttribute](const auto& elementIds, auto& userData)
    {
        using ElementId = typename std::remove_reference_t<decltype(elementIds)>::value_type;

        std::map<size_t, ElementId> map;

        int elementIdsProcessed = 0;

        for(auto elementId : elementIds)
        {
            auto attributeValue = keyAttribute->stringValueOf(elementId);

            for(size_t row = 1; row < _data.numRows(); row++)
            {
                auto keyColumnValue = _data.valueAt(_keyColumnIndex, row);

                if(attributeValue == keyColumnValue)
                {
                    map[row] = elementId;
                    break;
                }
            }

            setProgress((++elementIdsProcessed * 100) / elementIds.size());
        }

        setProgress(-1);

        Q_ASSERT(!map.empty());
        if(map.empty())
            return std::vector<QString>();

        for(auto columnIndex : _importColumnIndices)
        {
            auto name = _data.valueAt(columnIndex, 0);
            name = u::findUniqueName(userData.vectorNames(), name);

            for(const auto [row, elementId] : map)
            {
                auto value = _data.valueAt(columnIndex, row);
                userData.setValueBy(elementId, name, value);

                _createdVectorNames.emplace(name);
            }
        }

        return userData.exposeAsAttributes(*_graphModel);
    };

    if(keyAttribute->elementType() == ElementType::Node)
        _createdAttributeNames = createAttributes(_graphModel->mutableGraph().nodeIds(), _graphModel->userNodeData());
    else if(keyAttribute->elementType() == ElementType::Edge)
        _createdAttributeNames = createAttributes(_graphModel->mutableGraph().edgeIds(), _graphModel->userEdgeData());

    return true;
}

void ImportAttributesCommand::undo()
{
    auto tracker = _graphModel->attributeChangesTracker();

    for(const auto& attributeName : _createdAttributeNames)
        _graphModel->removeAttribute(attributeName);

    const auto* keyAttribute = _graphModel->attributeByName(_keyAttributeName);
    Q_ASSERT(keyAttribute != nullptr);

    for(const auto& vectorName : _createdVectorNames)
    {
        if(keyAttribute->elementType() == ElementType::Node)
            _graphModel->userNodeData().remove(vectorName);
        else if(keyAttribute->elementType() == ElementType::Edge)
            _graphModel->userEdgeData().remove(vectorName);
    }
}
