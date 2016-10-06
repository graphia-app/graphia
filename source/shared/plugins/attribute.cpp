#include "attribute.h"

void Attribute::set(int index, const QString& value)
{
    Q_ASSERT(index < static_cast<int>(_values.size()));

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
            _type = Type::Integer;
        else if(isFloat)
            _type = Type::Float;
        else
            _type = Type::String;

        break;

    case Type::Integer:
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

    _values.at(index) = value;

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

const QString& Attribute::get(int index) const
{
    Q_ASSERT(index < static_cast<int>(_values.size()));
    return _values.at(index);
}

Attribute& Attributes::attributeByName(const QString& name)
{
    auto it = std::find_if(_attributes.begin(), _attributes.end(),
                           [&name](auto& v) { return v.name() == name; });
    Q_ASSERT(it != _attributes.end());
    return *it;
}

const Attribute& Attributes::attributeByName(const QString& name) const
{
    auto it = std::find_if(_attributes.begin(), _attributes.end(),
                           [&name](auto& v) { return v.name() == name; });
    Q_ASSERT(it != _attributes.end());
    return *it;
}

void Attributes::add(const QString& name)
{
    Q_ASSERT(_size > 0);
    _attributes.emplace_back(name, _size);

    emit attributeAdded(name);
}

void Attributes::setValue(int index, const QString& name, const QString& value)
{
    Q_ASSERT(index < _size);
    attributeByName(name).set(index, value);
}

const QString& Attributes::value(int index, const QString& name) const
{
    return attributeByName(name).get(index);
}
