#ifndef RANDOMLAYOUT_H
#define RANDOMLAYOUT_H

#include "layoutalgorithm.h"

class RandomLayout : public LayoutAlgorithm
{
    Q_OBJECT
private:
    float spread;

public:
    RandomLayout(const ReadOnlyGraph& graph, NodeArray<QVector3D>& positions) :
        LayoutAlgorithm(graph, positions), spread(10.0f)
    {}

    void setSpread(float spread) { this->spread = spread; }
    void executeReal();
};

#endif // RANDOMLAYOUT_H
