#include "userdatavector.h"

#include <QJsonArray>

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

QJsonObject UserDataVector::save() const
{
    QJsonObject jsonObject;

    switch(_type)
    {
    default:
    case Type::Unknown: jsonObject["type"] = "Unknown"; break;
    case Type::String:  jsonObject["type"] = "String"; break;
    case Type::Int:     jsonObject["type"] = "Int"; break;
    case Type::Float:   jsonObject["type"] = "Float"; break;
    }

    jsonObject["name"] = _name;
    jsonObject["intMin"] = _intMin;
    jsonObject["intMax"] = _intMax;
    jsonObject["floatMin"] = _floatMin;
    jsonObject["floatMax"] = _floatMax;

    QJsonArray jsonValues;
    for(const auto& value : _values)
        jsonValues.append(value);

    jsonObject["values"] = jsonValues;

    return jsonObject;
}

bool UserDataVector::load(const QJsonObject& jsonObject)
{
    if(!jsonObject["type"].isString())
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

    if(!jsonObject["name"].isString())
        return false;

    _name = jsonObject["name"].toString();

    if(jsonObject.contains("intMin") && jsonObject.contains("intMax"))
    {
        if(!jsonObject["intMin"].isDouble() || !jsonObject["intMax"].isDouble())
            return false;

        _intMin = jsonObject["intMin"].toInt();
        _intMax = jsonObject["intMax"].toInt();
    }

    if(jsonObject.contains("floatMin") && jsonObject.contains("floatMax"))
    {
        if(!jsonObject["floatMin"].isDouble() || !jsonObject["floatMax"].isDouble())
            return false;

        _floatMin = jsonObject["floatMin"].toDouble();
        _floatMax = jsonObject["floatMax"].toDouble();
    }

    if(!jsonObject["values"].isArray())
        return false;

    for(const auto& jsonValue : jsonObject["values"].toArray())
        _values.emplace_back(jsonValue.toString());

    return true;
}
