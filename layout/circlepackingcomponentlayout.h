#ifndef CIRCLEPACKINGCOMPONENTLAYOUT_H
#define CIRCLEPACKINGCOMPONENTLAYOUT_H

#include "layout.h"

class CirclePackingComponentLayout : public ComponentLayout
{
    Q_OBJECT

public:
    CirclePackingComponentLayout(const Graph& graph, ComponentPositions& componentPositions, NodePositions& nodePositions) :
        ComponentLayout(graph, componentPositions, nodePositions, Iterative::Yes)
    {}

    void executeReal(uint64_t iteration);
};

#endif // CIRCLEPACKINGCOMPONENTLAYOUT_H
