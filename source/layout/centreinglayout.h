#ifndef CENTERINGLAYOUT_H
#define CENTERINGLAYOUT_H

#include "layout.h"

class CentreingLayout : public Layout
{
    Q_OBJECT
public:
    CentreingLayout(const Graph& graph,
                    NodePositions& positions) :
        Layout(graph, positions)
    {}

    void executeReal(uint64_t);
};


#endif // CENTERINGLAYOUT_H
