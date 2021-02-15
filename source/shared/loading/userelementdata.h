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

#ifndef USERELEMENTDATA_H
#define USERELEMENTDATA_H

#include "userdata.h"

#include "shared/graph/grapharray.h"
#include "shared/graph/imutablegraph.h"
#include "shared/graph/igraphmodel.h"
#include "shared/attributes/iattribute.h"
#include "shared/utils/container.h"
#include "shared/utils/progressable.h"

#include <map>
#include <set>
#include <memory>

template<typename E>
class UserElementDataMapping
{
private:
    struct Index
    {
        bool _set = false;
        size_t _value = 0;
    };

    std::unique_ptr<ElementIdArray<E, Index>> _indexes;
    std::map<size_t, E> _indexToElementIdMap;

    int _numMappings = 0;

protected:
    void setNumMappings(int numMappings) { _numMappings = numMappings; }

public:
    void initialise(IMutableGraph& mutableGraph)
    {
        _indexes = std::make_unique<ElementIdArray<E, Index>>(mutableGraph);
    }

    void resetMapping()
    {
        _indexes->resetElements();
        _indexToElementIdMap.clear();
    }

    void setElementIdForIndex(E elementId, size_t index)
    {
        _indexes->set(elementId, {true, index});
        _indexToElementIdMap[index] = elementId;
    }

    E elementIdForIndex(size_t index) const
    {
        if(u::contains(_indexToElementIdMap, index))
            return _indexToElementIdMap.at(index);

        // This can happen if the user has deleted some nodes then saved and reloaded
        // In this case the ElementIds may no longer exist for the index in question
        return {};
    }

    bool haveIndexFor(E elementId) const
    {
        return _indexes->get(elementId)._set;
    }

    size_t indexFor(E elementId) const
    {
        return _indexes->get(elementId)._value;
    }

    int numMappings() const { return _numMappings; }
};

using UserNodeDataMapping = UserElementDataMapping<NodeId>;
using UserEdgeDataMapping = UserElementDataMapping<EdgeId>;

template<typename E>
class UserElementData : public UserElementDataMapping<E>, public UserData
{
private:
    std::set<QString> _exposedAsAttributes;

public:
    void setValueBy(E elementId, const QString& name, const QString& value)
    {
        if(!this->haveIndexFor(elementId))
            this->setElementIdForIndex(elementId, static_cast<size_t>(numValues()));

        setValue(this->indexFor(elementId), name, value);
        this->setNumMappings(numValues());
    }

    QVariant valueBy(E elementId, const QString& name) const
    {
        if(!this->haveIndexFor(elementId))
            return {};

        return value(this->indexFor(elementId), name);
    }

    void remove(const QString& name) override
    {
        UserData::remove(name);
        _exposedAsAttributes.erase(name);
    }

    std::vector<QString> exposeAsAttributes(IGraphModel& graphModel)
    {
        std::vector<QString> createdAttributeNames;

        for(const auto& [name, userDataVector] : *this)
        {
            // https://stackoverflow.com/questions/46114214/lambda-implicit-capture-fails-with-variable-declared-from-structured-binding
            const auto& userDataVectorName = name;

            if(u::contains(_exposedAsAttributes, userDataVectorName))
                continue;

            _exposedAsAttributes.emplace(userDataVectorName);

            QString attributeName;

            auto& attribute = graphModel.createAttribute(userDataVectorName, &attributeName)
                .setFlag(AttributeFlag::Searchable)
                .setUserDefined(true);

            createdAttributeNames.emplace_back(attributeName);

            switch(userDataVector.type())
            {
            case UserDataVector::Type::Float:
                attribute.setFloatValueFn(
                [this, userDataVectorName](E elementId)
                {
                    return valueBy(elementId, userDataVectorName).toFloat();
                })
                .setFlag(AttributeFlag::AutoRange);
                break;

            case UserDataVector::Type::Int:
                attribute.setIntValueFn(
                [this, userDataVectorName](E elementId)
                {
                    return valueBy(elementId, userDataVectorName).toInt();
                })
                .setFlag(AttributeFlag::AutoRange);
                break;

            case UserDataVector::Type::Unknown:
            // Treat the unknown type as strings; the usual reason for this
            // happening is if the entire vector is empty
            case UserDataVector::Type::String:
                attribute.setStringValueFn(
                [this, userDataVectorName](E elementId)
                {
                    return valueBy(elementId, userDataVectorName).toString();
                })
                .setFlag(AttributeFlag::FindShared);
                break;

            default: break;
            }

            attribute.setValueMissingFn([this, userDataVectorName](E elementId)
            {
                if(!this->haveIndexFor(elementId))
                    return false;

                return value(this->indexFor(elementId), userDataVectorName).toString().isEmpty();
            });

            attribute.setDescription(QString(QObject::tr("%1 is a user defined attribute.")).arg(userDataVectorName));
        }

        return createdAttributeNames;
    }

    json save(const IMutableGraph&, const std::vector<E>& elementIds, Progressable& progressable) const
    {
        std::vector<size_t> indexes;
        json jsonIds = json::array();

        for(auto elementId : elementIds)
        {
            if(this->haveIndexFor(elementId))
            {
                jsonIds.push_back(elementId);
                indexes.push_back(this->indexFor(elementId));
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

        this->resetMapping();

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
        for(const auto& id : ids)
        {
            E elementId = id.get<int>();
            this->setElementIdForIndex(elementId, index++);
        }

        this->setNumMappings(numValues());

        return true;
    }
};

using UserNodeData = UserElementData<NodeId>;
using UserEdgeData = UserElementData<EdgeId>;

#endif // USERELEMENTDATA_H
