#ifndef RANDOMLAYOUT_H
#define RANDOMLAYOUT_H

#include "layoutalgorithm.h"

class RandomLayout : public LayoutAlgorithm
{
    Q_OBJECT
public:
    RandomLayout(const ReadOnlyGraph& graph, NodeArray<QVector3D>& positions) :
        LayoutAlgorithm(graph, positions)
    {}

    void executeReal();
};

#endif // RANDOMLAYOUT_H
