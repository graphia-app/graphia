#ifndef RANDOMLAYOUT_H
#define RANDOMLAYOUT_H

#include "layoutalgorithm.h"

class RandomLayout : public LayoutAlgorithm
{
    Q_OBJECT
public:
    RandomLayout(NodeArray<QVector3D>& positions) : LayoutAlgorithm(positions, 1) {}

    void executeReal();
};

#endif // RANDOMLAYOUT_H
