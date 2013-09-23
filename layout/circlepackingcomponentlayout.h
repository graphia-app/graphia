#ifndef CIRCLEPACKINGCOMPONENTLAYOUT_H
#define CIRCLEPACKINGCOMPONENTLAYOUT_H

#include "layout.h"

class CirclePackingComponentLayout : public ComponentLayout
{
    Q_OBJECT
private:
    bool firstIteration;

public:
    CirclePackingComponentLayout(const Graph& graph, ComponentPositions& componentPositions, NodePositions& nodePositions) :
        ComponentLayout(graph, componentPositions, nodePositions, true), firstIteration(true)
    {}

    void executeReal();
};

#endif // CIRCLEPACKINGCOMPONENTLAYOUT_H
