#include "typeidentity.h"

#include <QString>

void TypeIdentity::updateType(const QString& value)
{
    // If the value is empty we can't determine its type
    if(value.isEmpty())
        return;

    bool conversionSucceeded = false;

    value.toInt(&conversionSucceeded);
    bool isInt = conversionSucceeded;

    value.toDouble(&conversionSucceeded);
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
