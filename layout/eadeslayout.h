#ifndef EADESLAYOUT_H
#define EADESLAYOUT_H

#include "layoutalgorithm.h"

#include "../graph/grapharray.h"

class EadesLayout : public LayoutAlgorithm
{
    Q_OBJECT
private:
    bool firstIteration;
    NodeArray<QVector3D> moves;

public:
    EadesLayout(NodeArray<QVector3D>& positions) :
        LayoutAlgorithm(positions, LayoutAlgorithm::Unbounded),
        firstIteration(true),
        moves(graph())
    {}

    void executeReal();
};

#endif // EADESLAYOUT_H
