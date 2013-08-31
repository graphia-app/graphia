#ifndef CENTERINGLAYOUT_H
#define CENTERINGLAYOUT_H

#include "layout.h"

class CentreingLayout : public Layout
{
    Q_OBJECT
public:
    CentreingLayout(const ReadOnlyGraph& graph, NodeArray<QVector3D>& positions) :
        Layout(graph, positions)
    {}

    void executeReal();
};


#endif // CENTERINGLAYOUT_H
