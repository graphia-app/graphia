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

#include "userelementdata.h"

#include "shared/attributes/iattribute.h"
#include "shared/graph/igraphmodel.h"
#include "shared/graph/imutablegraph.h"
#include "shared/utils/container.h"
#include "shared/utils/json.h"
#include "shared/utils/string.h"

#include <QDebug>
#include <QObject>
#include <QVariantMap>

#include <algorithm>
#include <memory>
#include <type_traits>
#include <utility>

template<typename E>
void UserElementData<E>::generateElementIdMapping(E elementId)
{
    if(haveIndexFor(elementId))
        return;

    _indexes->set(elementId, {true, numValues()});
    _indexToElementIdMap[numValues()] = elementId;
}

template<typename E>
void UserElementData<E>::initialise(IMutableGraph& mutableGraph)
{
    _indexes = std::make_unique<ElementIdArray<E, Index>>(mutableGraph);
}

template<typename E>
QString UserElementData<E>::exposedAttributeName(const QString& vectorName) const
{
    if(_exposedAsAttributes.contains(vectorName))
        return _exposedAsAttributes.at(vectorName);

    return {};
}

template<typename E>
std::vector<QString> UserElementData<E>::exposedAttributeNames() const
{
    auto attributeNames = vectorNames();

    std::transform(attributeNames.begin(), attributeNames.end(), attributeNames.begin(),
        [this](const auto& vectorName) { return _exposedAsAttributes.at(vectorName); });

    return attributeNames;
}

template<typename E>
bool UserElementData<E>::setValueBy(E elementId, const AttributeDeclaration& declaration, const QString& value)
{
    add(declaration);
    return setValueBy(elementId, declaration._name, value);
}

template<typename E>
E UserElementData<E>::elementIdForIndex(size_t index) const
{
    if(u::contains(_indexToElementIdMap, index))
        return _indexToElementIdMap.at(index);

    // This can happen if the user has deleted some nodes then saved and reloaded
    // In this case the ElementIds may no longer exist for the index in question
    return {};
}

template<typename E>
void UserElementData<E>::remove(const QString& name)
{
    UserData::remove(name);

    // name may refer to the value in _inverseExposedAsAttributes, so erase that last
    const auto attributeName = _exposedAsAttributes.at(name);
    _exposedAttributeOverrides.erase(attributeName);
    _exposedAsAttributes.erase(name);
    _inverseExposedAsAttributes.erase(attributeName);
}

template<typename E>
bool UserElementData<E>::setAttributeType(IGraphModel& graphModel,
    const QString& attributeName, UserDataVector::Type type)
{
    if(!_inverseExposedAsAttributes.contains(attributeName))
        return false;

    if(!graphModel.attributeExists(attributeName))
        return false;

    auto userDataVectorName = _inverseExposedAsAttributes.at(attributeName);
    auto* userDataVector = vector(userDataVectorName);
    auto* attribute = graphModel.attributeByName(attributeName);

    if(!userDataVector->canConvertTo(type))
        return false;

    // Reset all flags that will be set below
    attribute->resetFlag(AttributeFlag::AutoRange);
    attribute->resetFlag(AttributeFlag::FindShared);

    switch(type)
    {
    case UserDataVector::Type::Float:
        attribute->setFloatValueFn(
        [this, userDataVectorName](E elementId)
        {
            return valueBy(elementId, userDataVectorName).toFloat();
        })
        .setSetValueFn([this, userDataVectorName](E elementId, const QString& value)
        {
            if(!value.isEmpty() && !u::isNumeric(value))
            {
                qDebug() << "UserDataVector setValueFn can't convert" << value << "to float, ignoring";
                return;
            }

            setValueBy(elementId, userDataVectorName, value);
        })
        .setFlag(AttributeFlag::AutoRange);
        break;

    case UserDataVector::Type::Int:
        attribute->setIntValueFn(
        [this, userDataVectorName](E elementId)
        {
            return valueBy(elementId, userDataVectorName).toInt();
        })
        .setSetValueFn([this, userDataVectorName](E elementId, const QString& value)
        {
            if(!value.isEmpty() && !u::isInteger(value))
            {
                qDebug() << "UserDataVector setValueFn can't convert" << value << "to integer, ignoring";
                return;
            }

            setValueBy(elementId, userDataVectorName, value);
        })
        .setFlag(AttributeFlag::AutoRange);
        break;

    case UserDataVector::Type::Unknown:
    // Treat the unknown type as strings; the usual reason for this
    // happening is if the entire vector is empty
    case UserDataVector::Type::String:
        attribute->setStringValueFn(
        [this, userDataVectorName](E elementId)
        {
            return valueBy(elementId, userDataVectorName).toString();
        })
        .setSetValueFn([this, userDataVectorName](E elementId, const QString& value)
        {
            setValueBy(elementId, userDataVectorName, value);
        })
        .setFlag(AttributeFlag::FindShared);
        break;

    default: break;
    }

    _exposedAttributeOverrides[attributeName]._type = attribute->valueType();

    return true;
}

template<typename E>
bool UserElementData<E>::setAttributeDescription(IGraphModel& graphModel,
    const QString& attributeName, const QString& description)
{
    if(!_inverseExposedAsAttributes.contains(attributeName))
        return false;

    if(!graphModel.attributeExists(attributeName))
        return false;

    auto* attribute = graphModel.attributeByName(attributeName);
    attribute->setDescription(description);
    _exposedAttributeOverrides[attributeName]._description = attribute->description();

    return true;
}

template<typename E>
std::vector<QString> UserElementData<E>::exposeAsAttributes(IGraphModel& graphModel)
{
    QString clashHint;

    // If an exposed attribute would clash, disambiguate with a prefix
    if constexpr(std::is_same_v<E, NodeId>)
        clashHint = QObject::tr("Node %1");
    else if constexpr(std::is_same_v<E, EdgeId>)
        clashHint = QObject::tr("Edge %1");

    std::vector<QString> createdAttributeNames;

    for(const auto& userDataVectorName : *this)
    {
        const auto* userDataVector = vector(userDataVectorName);

        if(_exposedAsAttributes.contains(userDataVectorName))
            continue;

        QString attributeName;

        auto savedOverride = std::ranges::find_if(_exposedAttributeOverrides,
            [&userDataVectorName](const auto& exposedAttributeOverride)
            {
                return exposedAttributeOverride.second._vectorName == userDataVectorName;
            });

        if(savedOverride != _exposedAttributeOverrides.end())
            attributeName = savedOverride->first;
        else if(graphModel.attributeIsDeclared(userDataVectorName) && !vectorIsDeclared(userDataVectorName))
            attributeName = QObject::tr("User %1").arg(userDataVectorName);
        else
            attributeName = userDataVectorName;

        auto& attribute = graphModel.createAttribute(attributeName, &attributeName, clashHint)
            .setFlag(AttributeFlag::Searchable)
            .setUserDefined(true);

        _exposedAsAttributes.emplace(userDataVectorName, attributeName);
        _inverseExposedAsAttributes.emplace(attributeName, userDataVectorName);
        auto& attributeOverride =
            _exposedAttributeOverrides.emplace(attributeName, AttributeOverride()).first->second;
        attributeOverride._vectorName = userDataVectorName;
        createdAttributeNames.emplace_back(attributeName);

        auto type = attributeOverride._type != ValueType::Unknown ?
            UserDataVector::equivalentTypeFor(attributeOverride._type) :
            userDataVector->type();

        setAttributeType(graphModel, attributeName, type);

        attribute.setValueMissingFn([this, userDataVectorName](E elementId)
        {
            if(!haveIndexFor(elementId))
                return false;

            return value(indexFor(elementId), userDataVectorName).toString().isEmpty();
        });

        attribute.setMetaDataFn([this, userDataVectorName]
        {
            ValueType valueType;
            switch(vector(userDataVectorName)->type())
            {
            default:
            case UserDataVector::Type::Unknown: valueType = ValueType::Unknown; break;
            case UserDataVector::Type::String:  valueType = ValueType::String; break;
            case UserDataVector::Type::Int:     valueType = ValueType::Int; break;
            case UserDataVector::Type::Float:   valueType = ValueType::Float; break;
            }

            return QVariantMap{{u"userDataType"_s, static_cast<int>(valueType)}};
        });

        auto description = !attributeOverride._description.isEmpty() ?
            attributeOverride._description :
            QObject::tr("%1 is a user defined attribute.").arg(graphModel.attributeDisplayName(attributeName));

        setAttributeDescription(graphModel, attributeName, description);
    }

    return createdAttributeNames;
}

template<typename E>
const UserDataVector* UserElementData<E>::vectorForAttributeName(const QString& attributeName)
{
    if(!_inverseExposedAsAttributes.contains(attributeName))
        return nullptr;

    const auto& vectorName = _inverseExposedAsAttributes.at(attributeName);
    return vector(vectorName);
}

template<typename E>
typename UserElementData<E>::RemovedVector UserElementData<E>::removeByAttributeName(const QString& attributeName)
{
    RemovedVector removee;

    if(_inverseExposedAsAttributes.contains(attributeName))
    {
        const auto& vectorName = _inverseExposedAsAttributes.at(attributeName);
        removee._vector = std::move(*vector(vectorName));
        removee._attributeName = attributeName;
        removee._override = _exposedAttributeOverrides.at(attributeName);
        removee._position = vectorNamesPositionOf(vectorName);
        remove(vectorName);
    }

    return removee;
}

template<typename E>
void UserElementData<E>::restore(RemovedVector removed)
{
    _exposedAttributeOverrides[removed._attributeName] = std::move(removed._override);
    setVector(std::move(removed._vector), removed._position);
}

template<typename E>
json UserElementData<E>::save(const IMutableGraph &, const std::vector<E> &elementIds, Progressable &progressable) const
{
    std::vector<size_t> indexes;
    json jsonIds = json::array();

    for(auto elementId : elementIds)
    {
        auto index = _indexes->at(elementId);
        if(index._set)
        {
            jsonIds.push_back(elementId);
            indexes.push_back(index._value);
        }
    }

    json jsonObject = UserData::save(progressable, indexes);
    jsonObject["ids"] = jsonIds;

    json jsonAttributes = json::object(); // NOLINT misc-const-correctness
    for(const auto& [attributeName, override] : _exposedAttributeOverrides)
    {
        auto& jsonAttribute = jsonAttributes[attributeName.toStdString()];

        const char* typeString = nullptr;
        switch(override._type)
        {
        case ValueType::Int:    typeString = "Int"; break;
        case ValueType::Float:  typeString = "Float"; break;
        case ValueType::String: typeString = "String"; break;
        default: break;
        }

        if(typeString != nullptr)
            jsonAttribute["type"] = typeString;

        jsonAttribute["description"] = override._description.toStdString();

        if(!override._vectorName.isEmpty())
            jsonAttribute["vector"] = override._vectorName.toStdString();
    }

    jsonObject["attributes"] = jsonAttributes;

    return jsonObject;
}

template<typename E>
bool UserElementData<E>::load(const json &jsonObject, Progressable &progressable)
{
    if(!UserData::load(jsonObject, progressable))
        return false;

    const char* idsKey = "ids";
    if(!u::contains(jsonObject, idsKey) || !jsonObject[idsKey].is_array())
    {
        // version <= 3 files call it indexes, try that too
        idsKey = "indexes";
        if(!u::contains(jsonObject, idsKey) || !jsonObject[idsKey].is_array())
            return false;
    }

    const auto& ids = jsonObject[idsKey];

    size_t index = 0;

    if(!_indexes->empty())
    {
        auto it = std::max_element(_indexes->begin(), _indexes->end(),
                                   [](const auto& a, const auto& b)
                                   {
                                       if(!a._set && b._set)
                                           return true;

                                       return a._value < b._value;
                                   });

        if(it->_set)
            index = it->_value;
    }

    for(const auto& id : ids)
    {
        E elementId = id.get<int>();

        if(elementId.isNull() || static_cast<size_t>(elementId) >= _indexes->size())
            return false;

        if(!haveIndexFor(elementId))
            setElementIdForIndex(elementId, index++);
    }

    if(u::contains(jsonObject, "attributes"))
    {
        const auto& attributeTypes = jsonObject["attributes"];
        for(const auto& i : attributeTypes.items())
        {
            auto attributeName = QString::fromStdString(i.key());
            const auto& override = i.value();

            if(u::contains(override, "type"))
            {
                const auto& typeString = override["type"];
                ValueType type = ValueType::Unknown;

                if(typeString == "Int")
                    type = ValueType::Int;
                else if(typeString == "Float")
                    type = ValueType::Float;
                else if(typeString == "String")
                    type = ValueType::String;

                _exposedAttributeOverrides[attributeName]._type = type;
            }

            if(u::contains(override, "description"))
            {
                _exposedAttributeOverrides[attributeName]._description =
                    QString::fromStdString(override["description"]);
            }

            if(u::contains(override, "vector") && override["vector"].is_string())
                _exposedAttributeOverrides[attributeName]._vectorName = QString::fromStdString(override["vector"]);
        }
    }

    return true;
}

// clazy:excludeall=missing-qobject-macro
template class UserElementData<NodeId>;
template class UserElementData<EdgeId>;
