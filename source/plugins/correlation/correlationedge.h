#ifndef CORRELATIONEDGE_H
#define CORRELATIONEDGE_H

#include "shared/graph/elementid.h"

struct CorrelationEdge
{
    NodeId _source;
    NodeId _target;
    double _r = 0.0;
};

#endif // CORRELATIONEDGE_H
