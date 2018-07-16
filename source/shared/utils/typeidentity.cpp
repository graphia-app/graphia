#include "typeidentity.h"

#include <QString>

void TypeIdentity::updateType(const QString& value)
{
    // If the value is empty we can't determine its type
    if(value.isEmpty())
        return;

    bool conversionSucceeded = false;

    auto intValue = value.toInt(&conversionSucceeded);
    Q_UNUSED(intValue); // Keep cppcheck happy
    bool isInt = conversionSucceeded;

    auto doubleValue = value.toDouble(&conversionSucceeded);
    Q_UNUSED(doubleValue); // Keep cppcheck happy
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
}
