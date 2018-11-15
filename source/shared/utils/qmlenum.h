#ifndef QMLENUM_H
#define QMLENUM_H

#include <QQmlEngine>
#include <QCoreApplication>
#include <QTimer>

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
    return (*a != 0 && *b != 0) ?
                (*a == *b && static_strcmp(a + 1, b + 1)) : // NOLINT
                (*a == 0 && *b == 0);
}

// Defining an enumeration that's usable in QML is awkward, so
// here is a macro to make it easier:

#define _REFLECTOR(x) x ## _reflector
#define QML_ENUM_PROPERTY(x) _REFLECTOR(x)::Enum

#define DEFINE_QML_ENUM(_Q_GADGET, ENUM_NAME, ...) \
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

DEFINE_QML_ENUM(Q_GADGET, Enumeration,
     First,
     Second,
     Third)

Note: the first parameter must be Q_GADGET, so that the build system knows
to generate a moc_ file
*/

#endif // QMLENUM_H

