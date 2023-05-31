/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

#include "typeidentity.h"

#include <QString>

void TypeIdentity::updateType(const QString& value)
{
    // If the value is empty we can't determine its type
    if(value.isEmpty())
        return;

    bool conversionSucceeded = false;

    auto intValue = value.toLongLong(&conversionSucceeded);
    Q_UNUSED(intValue); // Keep cppcheck happy
    const bool isInt = conversionSucceeded;

    auto doubleValue = value.toDouble(&conversionSucceeded);
    Q_UNUSED(doubleValue); // Keep cppcheck happy
    const bool isFloat = conversionSucceeded;

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

bool TypeIdentity::canConvertTo(Type type) const
{
    if(type == _type)
        return true;

    switch(_type)
    {
    case Type::Float:
        return type == Type::String;

    case Type::String:
        return false;

    default: return true;
    }

    return true;
}

TypeIdentity::Type TypeIdentity::equivalentTypeFor(ValueType valueType)
{
    switch(valueType)
    {
    default:
    case ValueType::Unknown:    return Type::Unknown;
    case ValueType::Int:        return Type::Int;
    case ValueType::Float:      return Type::Float;
    case ValueType::String:     return Type::String;
    }

    return Type::Unknown;
}
