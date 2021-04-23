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
    TabularData* data, int keyColumnIndex, std::vector<int> importColumnIndices, bool replace) :
    _graphModel(graphModel), _keyAttributeName(keyAttributeName), _data(std::move(*data)),
    _keyColumnIndex(keyColumnIndex), _importColumnIndices(std::move(importColumnIndices)),
    _replace(replace)
{}

size_t ImportAttributesCommand::numAttributes() const
{
    return _createdAttributeNames.size() + _replacedUserDataVectors.size();
}

QString ImportAttributesCommand::firstAttributeName() const
{
    if(!_createdAttributeNames.empty())
        return _createdAttributeNames.front();

    if(!_replacedUserDataVectors.empty())
        return _replacedUserDataVectors.begin()->first;

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

    for(const auto& [vectorName, vector] : _replacedUserDataVectors)
        text.append(QStringLiteral("\n  %1 (replaced)").arg(vectorName));

    return text;
}

bool ImportAttributesCommand::execute()
{
    auto tracker = _graphModel->attributeChangesTracker();

    const auto* keyAttribute = _graphModel->attributeByName(_keyAttributeName);
    Q_ASSERT(keyAttribute != nullptr);

    auto createAttributes = [&](const auto& elementIds)
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
            {
                _replacedUserDataVectors[name] = *existingVector;
                existingVector->clear();
                tracker->setAttributeValuesChanged(name);
            }
            else
            {
                name = u::findUniqueName(userData->vectorNames(), name);
                _createdVectorNames.emplace(name);
            }

            for(const auto& [row, elementId] : map)
            {
                auto value = _data.valueAt(columnIndex, row);
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
    auto tracker = _graphModel->attributeChangesTracker();

    for(const auto& attributeName : _createdAttributeNames)
        _graphModel->removeAttribute(attributeName);

    const auto* keyAttribute = _graphModel->attributeByName(_keyAttributeName);
    Q_ASSERT(keyAttribute != nullptr);

    auto undoUserData = [&](auto& userData)
    {
        for(const auto& vectorName : _createdVectorNames)
            userData.remove(vectorName);

        for(auto&& [vectorName, vector] : _replacedUserDataVectors)
        {
            userData.setVector(vectorName, std::move(vector));
            tracker->setAttributeValuesChanged(vectorName);
        }
    };

    if(keyAttribute->elementType() == ElementType::Node)
        undoUserData(_graphModel->userNodeData());
    else if(keyAttribute->elementType() == ElementType::Edge)
        undoUserData(_graphModel->userEdgeData());
}
