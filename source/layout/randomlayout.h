#ifndef RANDOMLAYOUT_H
#define RANDOMLAYOUT_H

#include "layout.h"

class RandomLayout : public Layout
{
    Q_OBJECT
private:
    float _spread = 10.0f;

public:
    RandomLayout(const Graph& graph, NodePositions& positions) :
        Layout(graph, positions)
    {}

    void setSpread(float spread) { _spread = spread; }
    void executeReal(bool);
};

#endif // RANDOMLAYOUT_H
