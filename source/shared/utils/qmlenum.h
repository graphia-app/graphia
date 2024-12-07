/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#ifndef QMLENUM_H
#define QMLENUM_H

#include <QObject>
#include <QQmlEngine>
#include <QVariant>

/*
Defining an enumeration that's available in both C++ and QML
is awkward, so here is a macro to make it easier. Example:

DEFINE_QML_ENUM(Enumeration,
    First,
    Second,
    Third)
*/

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

#define _REFLECTOR(x) x ## _reflector
#define QML_ENUM_PROPERTY(x) _REFLECTOR(x)::Enum

#define DEFINE_QML_ENUM(ENUM_NAME, ...) \
    class _REFLECTOR(ENUM_NAME) : public QObject \
    { \
        Q_OBJECT \
        QML_NAMED_ELEMENT(ENUM_NAME) \
        QML_UNCREATABLE("") \
        public: \
        enum Enum {__VA_ARGS__, Max}; Q_ENUM(Enum) \
    }; \
    inline bool operator&(QML_ENUM_PROPERTY(ENUM_NAME) lhs, \
        QML_ENUM_PROPERTY(ENUM_NAME) rhs) \
    { \
        /* Allow QML_ENUMs to be compared for intersection, via \
        the & operator. We return a truthy value rather than a \
        bitwise combination since the latter cannot be used in \
        a boolean context, which is the most common situation */ \
        return static_cast<bool>(static_cast<int>(lhs) & \
            static_cast<int>(rhs)); \
    } \
    using ENUM_NAME = QML_ENUM_PROPERTY(ENUM_NAME)

template<typename QmlEnumType>
QmlEnumType normaliseQmlEnum(auto v)
{
    auto zero = static_cast<QmlEnumType>(0);
    auto cast = static_cast<QmlEnumType>(v);

    if(cast < zero || cast >= QmlEnumType::Max)
        return zero;

    return cast;
}

template<typename QmlEnumType>
QmlEnumType qmlEnumFor(const QVariant& v)
{
    bool success;
    int i = v.toInt(&success);

    if(!success)
    {
        auto s = v.toString();
        if(!s.isEmpty())
        {
            auto metaEnum = QMetaEnum::fromType<QmlEnumType>();
            i = metaEnum.keyToValue(s.toUtf8().constData());
        }
    }

    return normaliseQmlEnum<QmlEnumType>(i);
}

// NOLINTEND(cppcoreguidelines-macro-usage)

#endif // QMLENUM_H

