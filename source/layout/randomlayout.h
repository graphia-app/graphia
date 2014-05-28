#ifndef RANDOMLAYOUT_H
#define RANDOMLAYOUT_H

#include "layout.h"

class RandomLayout : public NodeLayout
{
    Q_OBJECT
private:
    float spread;

public:
    RandomLayout(const ReadOnlyGraph& graph, NodePositions& positions) :
        NodeLayout(graph, positions), spread(10.0f)
    {}

    void setSpread(float spread) { this->spread = spread; }
    void executeReal(uint64_t);
};

#endif // RANDOMLAYOUT_H
