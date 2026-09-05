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

#include "shared/attributes/valuetype.h"
#include "shared/graph/grapharray.h"
#include "shared/utils/progressable.h"

#include <QString>
#include <QVariant>

#include <cstddef>
#include <map>
#include <memory>
#include <vector>

class IGraphModel;
class IMutableGraph;

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

    void generateElementIdMapping(E elementId);

public:
    void initialise(IMutableGraph& mutableGraph);

    QString exposedAttributeName(const QString& vectorName) const override;

    // The order this returns the names in is important; it should match the input file
    std::vector<QString> exposedAttributeNames() const override;

    void setElementIdForIndex(E elementId, size_t index) override
    {
        _indexes->set(elementId, {true, index});
        _indexToElementIdMap[index] = elementId;
    }

    E elementIdForIndex(size_t index) const override;

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

    void remove(const QString& name) override;

    bool setAttributeType(IGraphModel& graphModel, const QString& attributeName, UserDataVector::Type type);
    bool setAttributeDescription(IGraphModel& graphModel, const QString& attributeName, const QString& description);

    std::vector<QString> exposeAsAttributes(IGraphModel& graphModel);

    const UserDataVector* vectorForAttributeName(const QString& attributeName);
    UserDataVector removeByAttributeName(const QString& attributeName);

    json save(const IMutableGraph&, const std::vector<E>& elementIds, Progressable& progressable) const;
    bool load(const json& jsonObject, Progressable& progressable);
};

using UserNodeData = UserElementData<NodeId>;
using UserEdgeData = UserElementData<EdgeId>;

#endif // USERELEMENTDATA_H
