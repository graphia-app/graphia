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

#include "importattributescommand.h"

#include "graph/graphmodel.h"
#include "graph/mutablegraph.h"

#include "shared/utils/string.h"
#include "shared/loading/userelementdata.h"

ImportAttributesCommand::ImportAttributesCommand(GraphModel* graphModel, const QString& keyAttributeName,
    TabularData* data, int keyColumnIndex, const std::vector<int>& importColumnIndices, bool replace) :
    _graphModel(graphModel), _keyAttributeName(keyAttributeName), _data(std::move(*data)),
    _keyColumnIndex(keyColumnIndex), _importColumnIndices(importColumnIndices),
    _replace(replace)
{}

QString ImportAttributesCommand::firstAttributeName() const
{
    if(!_createdAttributeNames.empty())
        return _createdAttributeNames.front();

    if(!_replacedUserDataVectors.empty())
        return _replacedUserDataVectors.begin()->name();

    return {};
}

QString ImportAttributesCommand::description() const
{
    return multipleAttributes() ? QObject::tr("Import Attributes") : QObject::tr("Import Attribute");
}

QString ImportAttributesCommand::verb() const
{
    return multipleAttributes() ? QObject::tr("Importing Attributes") : QObject::tr("Importing Attribute");
}

QString ImportAttributesCommand::pastParticiple() const
{
    return multipleAttributes() ?
        QObject::tr("%1 Attributes Imported").arg(numAttributes()) :
        QObject::tr("Attribute %1 Imported").arg(firstAttributeName());
}

QString ImportAttributesCommand::debugDescription() const
{
    QString text = description();

    for(const auto& attributeName : _createdAttributeNames)
        text.append(QStringLiteral("\n  %1").arg(attributeName));

    for(const auto& vector : _replacedUserDataVectors)
        text.append(QStringLiteral("\n  %1 (replaced)").arg(vector.name()));

    return text;
}

bool ImportAttributesCommand::execute()
{
    AttributeChangesTracker tracker(_graphModel);

    const auto* keyAttribute = _graphModel->attributeByName(_keyAttributeName);
    Q_ASSERT(keyAttribute != nullptr);

    auto createAttributes = [&](const auto& elementIds)
    {
        using ElementId = typename std::remove_reference_t<decltype(elementIds)>::value_type;

        std::map<ElementId, size_t> map;

        int elementIdsProcessed = 0;

        for(auto elementId : elementIds)
        {
            auto attributeValue = keyAttribute->stringValueOf(elementId);

            for(size_t row = 1; row < _data.numRows(); row++)
            {
                auto keyColumnValue = _data.valueAt(_keyColumnIndex, row);

                if(attributeValue == keyColumnValue)
                {
                    map[elementId] = row;
                    break;
                }
            }

            setProgress((++elementIdsProcessed * 100) / static_cast<int>(elementIds.size()));
        }

        setProgress(-1);

        Q_ASSERT(!map.empty());
        if(map.empty())
            return std::vector<QString>();

        UserElementData<ElementId>* userData = nullptr;

        if constexpr(std::is_same_v<ElementId, NodeId>)
            userData = &_graphModel->userNodeData();
        else if constexpr(std::is_same_v<ElementId, EdgeId>)
            userData = &_graphModel->userEdgeData();

        for(auto columnIndex : _importColumnIndices)
        {
            auto name = _data.valueAt(columnIndex, 0);
            auto* existingVector = userData->vector(name);

            bool replace = _replace && existingVector != nullptr &&
                existingVector->type() == _data.typeIdentity(columnIndex).type();

            if(replace)
                _replacedUserDataVectors.emplace_back(*existingVector);
            else
            {
                name = u::findUniqueName(userData->vectorNames(), name);
                _createdVectorNames.emplace(name);
            }

            for(auto elementId : elementIds)
            {
                auto value = u::contains(map, elementId) ?
                    _data.valueAt(columnIndex, map.at(elementId)) : QString{};
                userData->setValueBy(elementId, name, value);
            }
        }

        return userData->exposeAsAttributes(*_graphModel);
    };

    if(keyAttribute->elementType() == ElementType::Node)
        _createdAttributeNames = createAttributes(_graphModel->mutableGraph().nodeIds());
    else if(keyAttribute->elementType() == ElementType::Edge)
        _createdAttributeNames = createAttributes(_graphModel->mutableGraph().edgeIds());

    return true;
}

void ImportAttributesCommand::undo()
{
    AttributeChangesTracker tracker(_graphModel);

    for(const auto& attributeName : _createdAttributeNames)
        _graphModel->removeAttribute(attributeName);

    _createdAttributeNames.clear();

    const auto* keyAttribute = _graphModel->attributeByName(_keyAttributeName);
    Q_ASSERT(keyAttribute != nullptr);

    auto undoUserData = [&](auto& userData)
    {
        for(const auto& vectorName : _createdVectorNames)
            userData.remove(vectorName);

        for(auto&& vector : _replacedUserDataVectors)
            userData.setVector(std::move(vector));
    };

    if(keyAttribute->elementType() == ElementType::Node)
        undoUserData(_graphModel->userNodeData());
    else if(keyAttribute->elementType() == ElementType::Edge)
        undoUserData(_graphModel->userEdgeData());

    _createdVectorNames.clear();
    _replacedUserDataVectors.clear();
}
