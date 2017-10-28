#ifndef ELEMENTIDDISTINCTSETCOLLECTION_DEBUG_H
#define ELEMENTIDDISTINCTSETCOLLECTION_DEBUG_H

#include "elementiddistinctsetcollection.h"

#include <QDebug>

template<typename C, typename T> QDebug operator<<(QDebug d, const ElementIdDistinctSet<C, T>& set)
{
    d << "[";
    for(auto id : set)
        d << id;
    d << "]";

    return d;
}

template<typename T> QDebug operator<<(QDebug d, const ElementIdDistinctSets<T>& set)
{
    d << "[";
    for(auto id : set)
        d << id;
    d << "]";

    return d;
}

#endif // ELEMENTIDDISTINCTSETCOLLECTION_DEBUG_H
