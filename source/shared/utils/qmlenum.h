/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

#include <string>

#ifndef APP_URI
#define APP_URI "uri.missing"
#endif
#ifndef APP_MAJOR_VERSION
#define APP_MAJOR_VERSION -1
#endif
#ifndef APP_MINOR_VERSION
#define APP_MINOR_VERSION -1
#endif

constexpr bool static_strcmp(char const* a, char const* b)
{
    return std::char_traits<char>::length(a) == std::char_traits<char>::length(b) &&
        std::char_traits<char>::compare(a, b, std::char_traits<char>::length(a)) == 0;
}

// Defining an enumeration that's usable in QML is awkward, so
// here is a macro to make it easier:

#define _REFLECTOR(x) x ## _reflector /* NOLINT cppcoreguidelines-macro-usage */
#define QML_ENUM_PROPERTY(x) _REFLECTOR(x)::Enum /* NOLINT cppcoreguidelines-macro-usage */

#define DEFINE_QML_ENUM(_Q_GADGET, ENUM_NAME, ...) /* NOLINT cppcoreguidelines-macro-usage */ \
    static_assert(static_strcmp(#_Q_GADGET, "Q_GADGET"), \
        "First parameter to DEFINE_QML_ENUM must be Q_GADGET"); \
    class _REFLECTOR(ENUM_NAME) \
    { \
        _Q_GADGET /* NOLINT */ \
    public: \
        enum class Enum {__VA_ARGS__}; Q_ENUM(Enum) \
        static void initialise() \
        { \
            static bool initialised = false; \
            if(initialised) \
                return; \
            initialised = true; \
            qmlRegisterUncreatableType<_REFLECTOR(ENUM_NAME)>( \
                APP_URI, \
                APP_MAJOR_VERSION, \
                APP_MINOR_VERSION, \
                #ENUM_NAME, QString()); \
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
    using ENUM_NAME = QML_ENUM_PROPERTY(ENUM_NAME) /* NOLINT */

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

