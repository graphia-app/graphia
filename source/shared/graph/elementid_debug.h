#ifndef ELEMENTID_DEBUG_H
#define ELEMENTID_DEBUG_H

#include "elementid.h"
#include "elementid_containers.h"

#include <QDebug>
#include <QString>

template<typename T> QDebug operator<<(QDebug d, const ElementId<T>& id)
{
    QString idString = id.isNull() ? QStringLiteral("Null") : QString::number(id);
    d << idString;

    return d;
}

template<typename T> QDebug operator<<(QDebug d, const ElementIdSet<T>& idSet)
{
    d << "[";
    for(auto id : idSet)
        d << id;
    d << "]";

    return d;
}

#endif // ELEMENTID_DEBUG_H
