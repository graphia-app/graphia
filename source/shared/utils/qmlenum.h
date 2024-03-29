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

#ifndef QMLENUM_H
#define QMLENUM_H

#include <QQmlEngine>
#include <QCoreApplication>
#include <QTimer>
#include <QVariant>

#include <string>

#ifndef APP_URI
#define APP_URI "uri.missing"
#endif
#ifndef APP_MAJOR_VERSION
#define APP_MAJOR_VERSION (-1)
#endif
#ifndef APP_MINOR_VERSION
#define APP_MINOR_VERSION (-1)
#endif

constexpr bool static_strcmp(char const* a, char const* b)
{
    return std::char_traits<char>::length(a) == std::char_traits<char>::length(b) &&
        std::char_traits<char>::compare(a, b, std::char_traits<char>::length(a)) == 0;
}

// Defining an enumeration that's usable in QML is awkward, so
// here is a macro to make it easier:

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

#define _REFLECTOR(x) x ## _reflector
#define QML_ENUM_PROPERTY(x) _REFLECTOR(x)::Enum

#define DEFINE_QML_ENUM(_Q_GADGET, ENUM_NAME, ...) \
    static_assert(static_strcmp(#_Q_GADGET, "Q_GADGET"), \
        "First parameter to DEFINE_QML_ENUM must be Q_GADGET"); \
    class _REFLECTOR(ENUM_NAME) \
    { \
        _Q_GADGET \
    public: \
        enum class Enum {__VA_ARGS__, Max}; Q_ENUM(Enum) \
        static void initialise() \
        { \
            static bool initialised = false; \
            if(initialised) \
                return; \
            initialised = true; \
            qmlRegisterUncreatableMetaObject(_REFLECTOR(ENUM_NAME)::staticMetaObject, \
                APP_URI, \
                APP_MAJOR_VERSION, \
                APP_MINOR_VERSION, \
                #ENUM_NAME, {}); \
            qRegisterMetaType<_REFLECTOR(ENUM_NAME)::Enum>(#ENUM_NAME); \
        } \
    }; \
    static void ENUM_NAME ## _initialiser() \
    { \
        if(!QCoreApplication::instance()->startingUp()) \
        { \
            /* This will only occur from a DLL, where we need to delay the \
            initialisation until later so we can guarantee it occurs \
            after any static initialisation */ \
            QTimer::singleShot(0, [] { _REFLECTOR(ENUM_NAME)::initialise(); }); \
        } \
        else \
            _REFLECTOR(ENUM_NAME)::initialise(); \
    } \
    Q_COREAPP_STARTUP_FUNCTION(ENUM_NAME ## _initialiser) \
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

/*
Example:

DEFINE_QML_ENUM(
    Q_GADGET, Enumeration,
    First,
    Second,
    Third)

Note: the first parameter must be Q_GADGET, so that the build system knows
to generate a moc_ file, and because the scanner is a bit rubbish, Q_GADGET
must be the first thing on a line
*/

#endif // QMLENUM_H

