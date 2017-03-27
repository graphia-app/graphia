#include "userdatavector.h"

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
