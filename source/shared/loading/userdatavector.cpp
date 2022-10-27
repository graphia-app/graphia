/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

bool UserDataVector::set(size_t index, const QString& newValue)
{
    bool changed = false;

    if(index >= _values.size())
    {
        _values.resize(index + 1);
        changed = true;
    }

    auto& value = _values.at(index);
    changed = changed || (value != newValue);
    value = newValue;

    updateType(value);

    if(type() == Type::Int)
    {
        int intValue = value.toInt();
        _intMin = std::min(_intMin, intValue);
        _intMax = std::max(_intMax, intValue);
    }
    else if(type() == Type::Float)
    {
        double floatValue = value.toDouble();
        _floatMin = std::min(_floatMin, floatValue);
        _floatMax = std::max(_floatMax, floatValue);
    }

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

    switch(type())
    {
    default:
    case Type::Unknown: jsonObject["type"] = "Unknown"; break;
    case Type::String:  jsonObject["type"] = "String"; break;
    case Type::Int:     jsonObject["type"] = "Int"; break;
    case Type::Float:   jsonObject["type"] = "Float"; break;
    }

    if(_intMin != std::numeric_limits<int>::max() && _intMax != std::numeric_limits<int>::lowest())
    {
        jsonObject["intMin"] = _intMin;
        jsonObject["intMax"] = _intMax;
    }

    if(_floatMin != std::numeric_limits<double>::max() && _floatMax != std::numeric_limits<double>::lowest())
    {
        jsonObject["floatMin"] = _floatMin;
        jsonObject["floatMax"] = _floatMax;
    }

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

    if(!jsonObject["type"].is_string())
        return false;

    if(jsonObject["type"] == "String")
        setType(Type::String);
    else if(jsonObject["type"] == "Int")
        setType(Type::Int);
    else if(jsonObject["type"] == "Float")
        setType(Type::Float);
    else
        setType(Type::Unknown);

    if(u::contains(jsonObject, "intMin") && u::contains(jsonObject, "intMax"))
    {
        if(!jsonObject["intMin"].is_number() || !jsonObject["intMax"].is_number())
            return false;

        _intMin = jsonObject["intMin"];
        _intMax = jsonObject["intMax"];
    }

    if(u::contains(jsonObject, "floatMin") && u::contains(jsonObject, "floatMax"))
    {
        if(!jsonObject["floatMin"].is_number() || !jsonObject["floatMax"].is_number())
            return false;

        _floatMin = jsonObject["floatMin"];
        _floatMax = jsonObject["floatMax"];
    }

    if(!jsonObject["values"].is_array())
        return false;

    _values.clear();
    for(const auto& value : jsonObject["values"])
        _values.push_back(value);

    return true;
}
