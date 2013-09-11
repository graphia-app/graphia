#ifndef RANDOMLAYOUT_H
#define RANDOMLAYOUT_H

#include "layout.h"

class RandomLayout : public NodeLayout
{
    Q_OBJECT
private:
    float spread;

public:
    RandomLayout(const ReadOnlyGraph& graph, NodeArray<QVector3D>& positions) :
        NodeLayout(graph, positions), spread(10.0f)
    {}

    void setSpread(float spread) { this->spread = spread; }
    void executeReal();
};

#endif // RANDOMLAYOUT_H
