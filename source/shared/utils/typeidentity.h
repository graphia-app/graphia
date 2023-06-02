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

#include <array>

class QString;

class TypeIdentity
{
public:
    enum class Type
    {
        Unknown = -1,
        Int = 0,
        Float,
        String
    };

private:
    std::array<size_t, 3> _typeCounts = {};

    size_t count(Type type) const;
    void increment(Type type);
    void decrement(Type type);

public:
    void updateType(const QString& value, const QString& previousValue = {});

    template<typename C>
    void updateType(const C& values)
    {
        _typeCounts = {};
        for(const auto& value : values)
            updateType(value);
    }

    Type type() const;

    bool canConvertTo(Type type) const;
    static Type equivalentTypeFor(ValueType valueType);
};

#endif // TYPEIDENTITY_H
