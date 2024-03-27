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

#ifndef ENUMREFLECTION_H
#define ENUMREFLECTION_H

#include <QString>

template<typename T> struct EnumStrings
{
    static QString values[];
    static size_t size;
};

#define DECLARE_REFLECTED_ENUM(E) \
    template<> struct EnumStrings<E> \
    { \
        static QString values[]; \
        static size_t size; \
    };

#define DEFINE_REFLECTED_ENUM(E, ...)  \
    QString EnumStrings<E>::values[] = \
    {__VA_ARGS__}; \
    size_t EnumStrings<E>::size = \
        sizeof(EnumStrings<E>::values) / sizeof(EnumStrings<E>::values[0]);

template<typename T>
const QString& enumToString(T value)
{
    auto index = static_cast<size_t>(value);
    Q_ASSERT(index < EnumStrings<T>::size);
    return EnumStrings<T>::values[index];
}

template<typename T>
T stringToEnum(const QString& value)
{
    for(size_t i = 0; i < EnumStrings<T>::size; i++)
    {
        if(EnumStrings<T>::values[i] == value)
            return static_cast<T>(i);
    }

    return T();
}

#endif // ENUMREFLECTION_H

