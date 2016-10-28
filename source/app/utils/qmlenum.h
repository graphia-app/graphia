#ifndef QMLENUM_H
#define QMLENUM_H

#include <QtQml>

#include "../application.h"

// Defining an enumeration that's usable in QML is awkward, so
// here are a couple of macros to make it easier:

// Stick this one in a header...
#define DEFINE_QML_ENUM(ENUM_NAME, ...) \
    class ENUM_NAME \
    { \
        Q_GADGET \
        Q_ENUMS(Enum) \
        struct Constructor \
        { Constructor() { qmlRegisterUncreatableType<ENUM_NAME>( \
            Application::uri(), \
            Application::majorVersion(), \
            Application::minorVersion(), \
            #ENUM_NAME, QString()); } }; \
        static Constructor constructor; \
    public: \
        enum Enum \
        {__VA_ARGS__}; \
    }

// ...and this one in a compilation unit
#define REGISTER_QML_ENUM(ENUM_NAME) \
    ENUM_NAME::Constructor ENUM_NAME::constructor

// Example:
//
// file.h:
// DEFINE_QML_ENUM(Enumeration,
//      First,
//      Second,
//      Third)
//
// file.cpp:
// REGISTER_QML_ENUM(Enumeration)

#endif // QMLENUM_H

