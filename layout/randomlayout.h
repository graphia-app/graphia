#ifndef RANDOMLAYOUT_H
#define RANDOMLAYOUT_H

#include "layoutalgorithm.h"

class RandomLayout : public LayoutAlgorithm
{
public:
    RandomLayout(NodeArray<QVector3D>& positions) : LayoutAlgorithm(positions) {}

    void execute();
};

#endif // RANDOMLAYOUT_H
