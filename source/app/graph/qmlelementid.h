#ifndef QMLELEMENTID_H
#define QMLELEMENTID_H

// This simply provides QML accessible version of the ElementId types

#include "application.h"

#include "shared/graph/elementid.h"

#include <QMetaType>
#include <QObject>

#define QML_TYPE(Type) Qml ## Type
#define QML_ELEMENTID(Type) \
    class QML_TYPE(Type) \
    { \
        Q_GADGET \
        Q_PROPERTY(int id READ id CONSTANT) \
        Q_PROPERTY(bool isNull READ isNull CONSTANT) \
    public: \
        QML_TYPE(Type)() = default; \
        QML_TYPE(Type)(Type id) : _id(id) {} \
        operator Type() const { return _id; } \
        bool isNull() const { return _id.isNull(); } \
    private: \
        Type _id; \
        int id() const { return _id; } \
    };

QML_ELEMENTID(NodeId)
QML_ELEMENTID(EdgeId)
QML_ELEMENTID(ComponentId)

inline void registerQmlElementIdTypes()
{
    qRegisterMetaType<QmlNodeId>("QmlNodeId");
    qRegisterMetaType<QmlNodeId>("QmlEdgeId");
    qRegisterMetaType<QmlNodeId>("QmlComponentId");
}

Q_COREAPP_STARTUP_FUNCTION(registerQmlElementIdTypes)

#endif // QMLELEMENTID_H
