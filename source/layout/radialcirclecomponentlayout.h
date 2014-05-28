#ifndef RADIALCIRCLECOMPONENTLAYOUT_H
#define RADIALCIRCLECOMPONENTLAYOUT_H

#include "layout.h"

class RadialCircleComponentLayout : public ComponentLayout
{
    Q_OBJECT
public:
    RadialCircleComponentLayout(const Graph& graph, ComponentPositions& componentPositions, NodePositions& nodePositions) :
        ComponentLayout(graph, componentPositions, nodePositions, Iterative::No)
    {}

    void executeReal(uint64_t);
};
#endif // RADIALCIRCLECOMPONENTLAYOUT_H
