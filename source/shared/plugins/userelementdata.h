#ifndef USERELEMENTDATA_H
#define USERELEMENTDATA_H

#include "userdata.h"

#include "shared/graph/grapharray.h"
#include "shared/graph/imutablegraph.h"
#include "shared/graph/igraphmodel.h"
#include "shared/attributes/iattribute.h"
#include "shared/utils/container.h"

#include <map>
#include <memory>

template<typename E>
class UserElementData : public UserData
{
private:
    struct Index
    {
        bool _set = false;
        size_t _row = 0;
    };

    std::unique_ptr<ElementIdArray<E, Index>> _indexes;
    std::map<size_t, E> _rowToElementIdMap;

    void generateElementIdMapping(E elementId)
    {
        if(_indexes->get(elementId)._set)
        {
            // Already got one
            return;
        }

        _indexes->set(elementId, {true, static_cast<size_t>(numValues())});
        _rowToElementIdMap[numValues()] = elementId;
    }

public:
    void initialise(IMutableGraph& mutableGraph)
    {
        _indexes = std::make_unique<ElementIdArray<E, Index>>(mutableGraph);
    }

    void setElementIdForRowIndex(E elementId, size_t row)
    {
        _indexes->set(elementId, {true, row});
        _rowToElementIdMap[row] = elementId;
    }

    E elementIdForRowIndex(size_t row) const
    {
        Q_ASSERT(u::contains(_rowToElementIdMap, row));
        return _rowToElementIdMap.at(row);
    }

    size_t rowIndexFor(E elementId) const
    {
        return _indexes->get(elementId)._row;
    }

    void setValueBy(E elementId, const QString& name, const QString& value)
    {
        generateElementIdMapping(elementId);
        setValue(rowIndexFor(elementId), name, value);
    }

    QVariant valueBy(E elementId, const QString& name) const
    {
        return value(rowIndexFor(elementId), name);
    }

    void exposeAsAttributes(IGraphModel& graphModel)
    {
        for(const auto& userDataVector : *this)
        {
            const auto& userDataVectorName = userDataVector.name();
            auto& attribute = graphModel.createAttribute(userDataVectorName)
                    .setSearchable(true)
                    .setUserDefined(true);

            switch(userDataVector.type())
            {
            case UserDataVector::Type::Float:
                attribute.setFloatValueFn([this, userDataVectorName](E elementId)
                        {
                            return valueBy(elementId, userDataVectorName).toFloat();
                        })
                        .setFlag(AttributeFlag::AutoRangeMutable);
                break;

            case UserDataVector::Type::Int:
                attribute.setIntValueFn([this, userDataVectorName](E elementId)
                        {
                            return valueBy(elementId, userDataVectorName).toInt();
                        })
                        .setFlag(AttributeFlag::AutoRangeMutable);
                break;

            case UserDataVector::Type::String:
                attribute.setStringValueFn([this, userDataVectorName](E elementId)
                        {
                            return valueBy(elementId, userDataVectorName).toString();
                        });
                break;

            default: break;
            }

            bool hasMissingValues = std::any_of(userDataVector.begin(), userDataVector.end(),
                                                [](const auto& v) { return v.isEmpty(); });

            if(hasMissingValues)
            {
                attribute.setValueMissingFn([this, userDataVectorName](E elementId)
                {
                   return valueBy(elementId, userDataVectorName).toString().isEmpty();
                });
            }

            attribute.setDescription(QString(QObject::tr("%1 is a user defined attribute.")).arg(userDataVectorName));
        }
    }

    json save(const IMutableGraph&, const ProgressFn& progressFn) const
    {
        json jsonObject = UserData::save(progressFn);

        json jsonIndexes;
        for(auto index : *_indexes)
            jsonIndexes.emplace_back(index._row);

        jsonObject["indexes"] = jsonIndexes;

        return jsonObject;
    }

    bool load(const json& jsonObject, const ProgressFn& progressFn)
    {
        if(!UserData::load(jsonObject, progressFn))
            return false;

        _indexes->resetElements();
        _rowToElementIdMap.clear();

        if(!u::contains(jsonObject, "indexes") || !jsonObject["indexes"].is_array())
            return false;

        const auto& indexes = jsonObject["indexes"];

        E elementId(0);
        for(const auto& index : indexes)
            setElementIdForRowIndex(elementId++, index);

        return true;
    }
};

using UserNodeData = UserElementData<NodeId>;
using UserEdgeData = UserElementData<EdgeId>;

#endif // USERELEMENTDATA_H
