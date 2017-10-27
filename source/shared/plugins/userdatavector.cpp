#include "userdatavector.h"

#include "shared/utils/container.h"

void UserDataVector::set(size_t index, const QString& value)
{
    if(index >= _values.size())
        _values.resize(index + 1);

    _values.at(index) = value;

    // If the value is empty we can't determine its type
    if(value.isEmpty())
        return;

    bool conversionSucceeded = false;

    int intValue = value.toInt(&conversionSucceeded);
    bool isInt = conversionSucceeded;

    double floatValue = value.toDouble(&conversionSucceeded);
    bool isFloat = conversionSucceeded;

    switch(_type)
    {
    default:
    case Type::Unknown:
        if(isInt)
            _type = Type::Int;
        else if(isFloat)
            _type = Type::Float;
        else
            _type = Type::String;

        break;

    case Type::Int:
        if(!isInt)
        {
            if(isFloat)
                _type = Type::Float;
            else
                _type = Type::String;
        }

        break;

    case Type::Float:
        if(isFloat || isInt)
            _type = Type::Float;
        else
            _type = Type::String;

        break;

    case Type::String:
        _type = Type::String;

        break;
    }

    if(isInt)
    {
        _intMin = std::min(_intMin, intValue);
        _intMax = std::max(_intMax, intValue);
    }

    if(isFloat)
    {
        _floatMin = std::min(_floatMin, floatValue);
        _floatMax = std::max(_floatMax, floatValue);
    }
}

QString UserDataVector::get(size_t index) const
{
    if(index >= _values.size())
        return {};

    return _values.at(index);
}

json UserDataVector::save() const
{
    json jsonObject;

    switch(_type)
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

    jsonObject["values"] = _values;

    return jsonObject;
}

bool UserDataVector::load(const QString& name, const json& jsonObject)
{
    _name = name;

    if(!jsonObject["type"].is_string())
        return false;

    if(jsonObject["type"] == "Unknown")
        _type = Type::Unknown;
    else if(jsonObject["type"] == "String")
        _type = Type::String;
    else if(jsonObject["type"] == "Int")
        _type = Type::Int;
    else if(jsonObject["type"] == "Float")
        _type = Type::Float;
    else
        _type = Type::Unknown;

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
