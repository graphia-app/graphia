#ifndef QMLENUM_H
#define QMLENUM_H

#include <QQmlEngine>

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
    return (*a && *b) ? (*a == *b && static_strcmp(a + 1, b + 1)) : (*a == 0 && *b == 0);
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
        _Q_GADGET \
    public: \
        enum class Enum {__VA_ARGS__}; Q_ENUM(Enum) \
        struct Constructor \
        { Constructor() noexcept { static bool initialised = false; \
            if(initialised) return; \
            initialised = true; \
            qmlRegisterUncreatableType<_REFLECTOR(ENUM_NAME)>( \
            APP_URI, \
            APP_MAJOR_VERSION, \
            APP_MINOR_VERSION, \
            #ENUM_NAME, QString()); } }; \
    }; const _REFLECTOR(ENUM_NAME)::Constructor ENUM_NAME ## _constructor; \
    using ENUM_NAME = QML_ENUM_PROPERTY(ENUM_NAME)

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

