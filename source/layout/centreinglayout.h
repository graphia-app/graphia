#ifndef CENTERINGLAYOUT_H
#define CENTERINGLAYOUT_H

#include "layout.h"

class CentreingLayout : public NodeLayout
{
    Q_OBJECT
public:
    CentreingLayout(std::shared_ptr<const ReadOnlyGraph> graph,
                    std::shared_ptr<NodePositions> positions) :
        NodeLayout(graph, positions)
    {}

    void executeReal(uint64_t);
};


#endif // CENTERINGLAYOUT_H
