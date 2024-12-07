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

#ifndef USERELEMENTDATA_H
#define USERELEMENTDATA_H

#include "shared/loading/iuserelementdata.h"
#include "shared/loading/userdata.h"

#include "shared/graph/grapharray.h"
#include "shared/graph/imutablegraph.h"
#include "shared/graph/igraphmodel.h"
#include "shared/attributes/iattribute.h"
#include "shared/utils/container.h"
#include "shared/utils/progressable.h"
#include "shared/utils/string.h"

#include <map>
#include <set>
#include <memory>

#include <QDebug>

template<typename E>
class UserElementData : public IUserElementData<E>, public UserData
{
private:
    struct Index
    {
        bool _set = false;
        size_t _value = 0;
    };

    std::unique_ptr<ElementIdArray<E, Index>> _indexes;
    std::map<size_t, E> _indexToElementIdMap;
    std::map<QString, QString> _exposedAsAttributes;
    std::map<QString, QString> _inverseExposedAsAttributes;

    struct AttributeOverride
    {
        ValueType _type = ValueType::Unknown;
        QString _description;
    };

    std::map<QString, AttributeOverride> _exposedAttributeOverrides;

    void generateElementIdMapping(E elementId)
    {
        if(haveIndexFor(elementId))
            return;

        _indexes->set(elementId, {true, numValues()});
        _indexToElementIdMap[numValues()] = elementId;
    }

public:
    void initialise(IMutableGraph& mutableGraph)
    {
        _indexes = std::make_unique<ElementIdArray<E, Index>>(mutableGraph);
    }

    QString exposedAttributeName(const QString& vectorName) const override
    {
        if(_exposedAsAttributes.contains(vectorName))
            return _exposedAsAttributes.at(vectorName);

        return {};
    }

    // The order this returns the names in is important; it should match the input file
    std::vector<QString> exposedAttributeNames() const override
    {
        auto attributeNames = vectorNames();

        std::transform(attributeNames.begin(), attributeNames.end(), attributeNames.begin(),
            [this](const auto& vectorName) { return _exposedAsAttributes.at(vectorName); });

        return attributeNames;
    }

    void setElementIdForIndex(E elementId, size_t index) override
    {
        _indexes->set(elementId, {true, index});
        _indexToElementIdMap[index] = elementId;
    }

    E elementIdForIndex(size_t index) const override
    {
        if(u::contains(_indexToElementIdMap, index))
            return _indexToElementIdMap.at(index);

        // This can happen if the user has deleted some nodes then saved and reloaded
        // In this case the ElementIds may no longer exist for the index in question
        return {};
    }

    bool haveIndexFor(E elementId) const
    {
        return _indexes->get(elementId)._set; // NOLINT readability-redundant-smartptr-get
    }

    size_t indexFor(E elementId) const override
    {
        return _indexes->get(elementId)._value; // NOLINT readability-redundant-smartptr-get
    }

    bool setValueBy(E elementId, const QString& name, const QString& value) override
    {
        generateElementIdMapping(elementId);
        return setValue(indexFor(elementId), name, value);
    }

    QVariant valueBy(E elementId, const QString& name) const override
    {
        if(!haveIndexFor(elementId))
            return {};

        return value(indexFor(elementId), name);
    }

    void remove(const QString& name) override
    {
        UserData::remove(name);
        _inverseExposedAsAttributes.erase(_exposedAsAttributes.at(name));
        _exposedAttributeOverrides.erase(name);
        _exposedAsAttributes.erase(name);
    }

    bool setAttributeType(IGraphModel& graphModel, const QString& attributeName, UserDataVector::Type type)
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
                if(!u::isNumeric(value))
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
                if(!u::isInteger(value))
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

    bool setAttributeDescription(IGraphModel& graphModel, const QString& attributeName, const QString& description)
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

    std::vector<QString> exposeAsAttributes(IGraphModel& graphModel)
    {
        std::vector<QString> createdAttributeNames;

        for(const auto& userDataVectorName : *this)
        {
            const auto* userDataVector = vector(userDataVectorName);

            if(_exposedAsAttributes.contains(userDataVectorName))
                continue;

            QString attributeName;

            auto& attribute = graphModel.createAttribute(userDataVectorName, &attributeName)
                .setFlag(AttributeFlag::Searchable)
                .setUserDefined(true);

            _exposedAsAttributes.emplace(userDataVectorName, attributeName);
            _inverseExposedAsAttributes.emplace(attributeName, userDataVectorName);
            const auto& attributeOverride =
                _exposedAttributeOverrides.emplace(attributeName, AttributeOverride()).first->second;
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
                QObject::tr("%1 is a user defined attribute.").arg(userDataVectorName);

            setAttributeDescription(graphModel, attributeName, description);
        }

        return createdAttributeNames;
    }

    const UserDataVector* vectorForAttributeName(const QString& attributeName)
    {
        if(!_inverseExposedAsAttributes.contains(attributeName))
            return nullptr;

        const auto& vectorName = _inverseExposedAsAttributes.at(attributeName);
        return vector(vectorName);
    }

    UserDataVector removeByAttributeName(const QString& attributeName)
    {
        UserDataVector removee;

        if(_inverseExposedAsAttributes.contains(attributeName))
        {
            const auto& vectorName = _inverseExposedAsAttributes.at(attributeName);
            removee = std::move(*vector(vectorName));
            remove(vectorName);
        }

        return removee;
    }

    json save(const IMutableGraph&, const std::vector<E>& elementIds, Progressable& progressable) const
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
        }

        jsonObject["attributes"] = jsonAttributes;

        return jsonObject;
    }

    bool load(const json& jsonObject, Progressable& progressable)
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
            }
        }

        return true;
    }
};

using UserNodeData = UserElementData<NodeId>;
using UserEdgeData = UserElementData<EdgeId>;

#endif // USERELEMENTDATA_H
