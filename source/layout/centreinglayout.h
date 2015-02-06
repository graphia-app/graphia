#ifndef CENTERINGLAYOUT_H
#define CENTERINGLAYOUT_H

#include "layout.h"

class CentreingLayout : public NodeLayout
{
    Q_OBJECT
public:
    CentreingLayout(const ImmutableGraph& graph,
                    NodePositions& positions) :
        NodeLayout(graph, positions)
    {}

    void executeReal(uint64_t);
};


#endif // CENTERINGLAYOUT_H
