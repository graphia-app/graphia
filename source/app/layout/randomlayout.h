#ifndef RANDOMLAYOUT_H
#define RANDOMLAYOUT_H

#include "layout.h"

class RandomLayout : public Layout
{
    Q_OBJECT
private:
    float _spread = 10.0f;

public:
    RandomLayout(const IGraphComponent& graphComponent, NodeLayoutPositions& positions) :
        Layout(graphComponent, positions)
    {}

    void setSpread(float spread) { _spread = spread; }
    void execute(bool, Dimensionality) override;
};

#endif // RANDOMLAYOUT_H
