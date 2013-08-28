#ifndef EADESLAYOUT_H
#define EADESLAYOUT_H

#include "layout.h"

#include <QVector3D>

class EadesLayout : public Layout
{
    Q_OBJECT
private:
    bool firstIteration;
    QVector<QVector3D> moves;

public:
    EadesLayout(const ReadOnlyGraph& graph, NodeArray<QVector3D>& positions) :
        Layout(graph, positions, Layout::Unbounded),
        firstIteration(true),
        moves(graph.numNodes())
    {}

    void executeReal();
};

#endif // EADESLAYOUT_H
