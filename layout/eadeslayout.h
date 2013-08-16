#ifndef EADESLAYOUT_H
#define EADESLAYOUT_H

#include "layoutalgorithm.h"

class EadesLayout : public LayoutAlgorithm
{
    Q_OBJECT
public:
    EadesLayout(NodeArray<QVector3D>& positions) :
        LayoutAlgorithm(positions, LayoutAlgorithm::Unbounded)
    {}

    void execute();
};

#endif // EADESLAYOUT_H
