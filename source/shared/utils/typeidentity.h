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

#ifndef TYPEIDENTITY_H
#define TYPEIDENTITY_H

#include "shared/attributes/valuetype.h"

class QString;

class TypeIdentity
{
public:
    enum class Type
    {
        Unknown,
        String,
        Int,
        Float
    };

private:
    Type _type = Type::Unknown;

public:
    void updateType(const QString& value);

    template<typename C>
    bool updateType(const C& values)
    {
        auto oldType = type();

        setType(Type::Unknown);
        for(const auto& value : values)
            updateType(value);

        return type() != oldType;
    }

    void setType(Type type) { _type = type; }
    Type type() const { return _type; }


    bool canConvertTo(Type type) const;
    static Type equivalentTypeFor(ValueType valueType);
};

#endif // TYPEIDENTITY_H
