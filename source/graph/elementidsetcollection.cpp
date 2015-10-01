#include "elementidsetcollection.h"

template<typename C> QDebug debug(QDebug d, C& set)
{
    d << "[";
    for(auto id : set)
        d << id;
    d << "]";

    return d;
}

QDebug operator<<(QDebug d, NodeIdSetCollection::Set& set)
{
    return debug(d, set);
}

QDebug operator<<(QDebug d, EdgeIdSetCollection::Set& set)
{
    return debug(d, set);
}
