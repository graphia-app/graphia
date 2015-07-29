#ifndef RANDOMLAYOUT_H
#define RANDOMLAYOUT_H

#include "layout.h"

class RandomLayout : public Layout
{
    Q_OBJECT
private:
    float _spread;

public:
    RandomLayout(const Graph& graph,
                 NodePositions& positions) :
        Layout(graph, positions), _spread(10.0f)
    {}

    void setSpread(float spread) { this->_spread = spread; }
    void executeReal(uint64_t);
};

#endif // RANDOMLAYOUT_H
