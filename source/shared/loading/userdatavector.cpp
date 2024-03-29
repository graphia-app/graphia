/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

#include "userdatavector.h"

#include "shared/utils/container.h"

QStringList UserDataVector::toStringList() const
{
    QStringList list;
    list.reserve(static_cast<int>(numValues()));

    for(const auto& value : _values)
        list.append(value);

    return list;
}

size_t UserDataVector::numUniqueValues() const
{
    auto v = _values;
    std::sort(v.begin(), v.end());
    auto last = std::unique(v.begin(), v.end());
    v.erase(last, v.end());

    return v.size();
}

bool UserDataVector::set(size_t index, const QString& value)
{
    bool changed = false;

    if(index >= _values.size())
    {
        _values.resize(index + 1);
        changed = true;
    }

    auto& previousValue = _values.at(index);
    changed = changed || (previousValue != value);
    updateType(value, previousValue);
    previousValue = value;

    return changed;
}

QString UserDataVector::get(size_t index) const
{
    if(index >= _values.size())
        return {};

    return _values.at(index);
}

json UserDataVector::save(const std::vector<size_t>& indexes) const
{
    json jsonObject;

    if(!indexes.empty())
    {
        json jsonValues = json::array();

        for(auto index : indexes)
        {
            if(index < _values.size())
                jsonValues.push_back(_values.at(index));
            else
                jsonValues.push_back(QString());
        }

        jsonObject["values"] = jsonValues;
    }
    else
        jsonObject["values"] = _values;

    return jsonObject;
}

bool UserDataVector::load(const QString& name, const json& jsonObject)
{
    _name = name;

    if(!jsonObject["values"].is_array())
        return false;

    _values.clear();
    for(const auto& value : jsonObject["values"])
        _values.push_back(value);

    updateType(_values);

    return true;
}
