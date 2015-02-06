#ifndef RANDOMLAYOUT_H
#define RANDOMLAYOUT_H

#include "layout.h"

class RandomLayout : public NodeLayout
{
    Q_OBJECT
private:
    float _spread;

public:
    RandomLayout(const ImmutableGraph& graph,
                 NodePositions& positions) :
        NodeLayout(graph, positions), _spread(10.0f)
    {}

    void setSpread(float spread) { this->_spread = spread; }
    void executeReal(uint64_t);
};

#endif // RANDOMLAYOUT_H
