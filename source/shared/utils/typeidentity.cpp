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

#include "typeidentity.h"

#include <QString>

static TypeIdentity::Type typeOf(const QString& value)
{
    // If the value is empty we can't determine its type
    if(value.isEmpty())
        return TypeIdentity::Type::Unknown;

    bool conversionSucceeded = false;

    auto intValue = value.toLongLong(&conversionSucceeded);
    Q_UNUSED(intValue); // Keep cppcheck happy
    if(conversionSucceeded)
        return TypeIdentity::Type::Int;

    auto doubleValue = value.toDouble(&conversionSucceeded);
    Q_UNUSED(doubleValue); // Keep cppcheck happy
    if(conversionSucceeded)
        return TypeIdentity::Type::Float;

    return TypeIdentity::Type::String;
}

size_t TypeIdentity::count(Type type) const
{
    return _typeCounts.at(static_cast<size_t>(type));
}

void TypeIdentity::increment(Type type)
{
    auto& count = _typeCounts.at(static_cast<size_t>(type));
    count++;
}

void TypeIdentity::decrement(Type type)
{
    auto& count = _typeCounts.at(static_cast<size_t>(type));
    Q_ASSERT(count > 0);
    count--;
}

void TypeIdentity::updateType(const QString& value, const QString& previousValue)
{
    auto previousType = typeOf(previousValue);
    auto type = typeOf(value);

    if(previousType != Type::Unknown)
        decrement(previousType);

    if(type != Type::Unknown)
        increment(type);
}

TypeIdentity::Type TypeIdentity::type() const
{
    if(count(Type::String) > 0)
        return Type::String;

    if(count(Type::Float) > 0)
        return Type::Float;

    if(count(Type::Int) > 0)
        return Type::Int;

    return Type::Unknown;
}

bool TypeIdentity::canConvertTo(Type type) const
{
    if(type == this->type())
        return true;

    switch(this->type())
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
