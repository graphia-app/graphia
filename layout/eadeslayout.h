#ifndef EADESLAYOUT_H
#define EADESLAYOUT_H

#include "layoutalgorithm.h"

#include <QVector3D>

class EadesLayout : public LayoutAlgorithm
{
    Q_OBJECT
private:
    bool firstIteration;
    QVector<QVector3D> moves;

public:
    EadesLayout(const ReadOnlyGraph& graph, NodeArray<QVector3D>& positions) :
        LayoutAlgorithm(graph, positions, LayoutAlgorithm::Unbounded),
        firstIteration(true),
        moves(graph.numNodes())
    {}

    void executeReal();
};

#endif // EADESLAYOUT_H
