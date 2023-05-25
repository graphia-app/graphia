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

    std::vector<QString> exposedAttributeNames() const override
    {
        return u::keysFor(_inverseExposedAsAttributes);
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
        _exposedAsAttributes.erase(name);
    }

    bool setAttributeType(IGraphModel& graphModel, const QString& attributeName, UserDataVector::Type type)
    {
        if(!_inverseExposedAsAttributes.contains(attributeName))
            return false;

        if(!graphModel.attributeExists(attributeName))
            return false;

        auto userDataVectorName = _inverseExposedAsAttributes.at(attributeName);
        const auto* userDataVector = vector(userDataVectorName);
        auto* attribute = graphModel.attributeByName(attributeName);

        // If the requested type doesn't match the detected
        // type, check that a conversion is possible
        if(type != userDataVector->type())
        {
            switch(userDataVector->type())
            {
            case UserDataVector::Type::Float:
                if(type == UserDataVector::Type::Int)
                {
                    qDebug() << "Can't convert int vector to float, ignoring";
                    return false;
                }
                break;

            case UserDataVector::Type::String:
                qDebug() << "Can't convert string vector to any other type, ignoring";
                return false;

            default: break;
            }
        }

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
            createdAttributeNames.emplace_back(attributeName);

            setAttributeType(graphModel, attributeName, userDataVector->type());

            attribute.setValueMissingFn([this, userDataVectorName](E elementId)
            {
                if(!haveIndexFor(elementId))
                    return false;

                return value(indexFor(elementId), userDataVectorName).toString().isEmpty();
            });

            attribute.setDescription(QString(QObject::tr("%1 is a user defined attribute.")).arg(userDataVectorName));
        }

        return createdAttributeNames;
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

        return true;
    }
};

using UserNodeData = UserElementData<NodeId>;
using UserEdgeData = UserElementData<EdgeId>;

#endif // USERELEMENTDATA_H
