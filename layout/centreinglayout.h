#ifndef CENTERINGLAYOUT_H
#define CENTERINGLAYOUT_H

#include "layout.h"

class CentreingLayout : public NodeLayout
{
    Q_OBJECT
public:
    CentreingLayout(const ReadOnlyGraph& graph, NodePositions& positions) :
        NodeLayout(graph, positions)
    {}

    void executeReal();
};


#endif // CENTERINGLAYOUT_H
