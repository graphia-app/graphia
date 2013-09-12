#ifndef LINEARCOMPONENTLAYOUT_H
#define LINEARCOMPONENTLAYOUT_H

#include "layout.h"

class LinearComponentLayout : public ComponentLayout
{
    Q_OBJECT
public:
    LinearComponentLayout(const Graph& graph, ComponentArray<QVector2D>& componentPositions, NodeArray<QVector3D>& nodePositions) :
        ComponentLayout(graph, componentPositions, nodePositions)
    {}

    void executeReal();
};

#endif // LINEARCOMPONENTLAYOUT_H
