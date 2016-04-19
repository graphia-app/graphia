#ifndef CENTERINGLAYOUT_H
#define CENTERINGLAYOUT_H

#include "layout.h"

class CentreingLayout : public Layout
{
    Q_OBJECT
public:
    CentreingLayout(const GraphComponent& graphComponent,
                    NodePositions& positions) :
        Layout(graphComponent, positions)
    {}

    void executeReal(bool);
};


#endif // CENTERINGLAYOUT_H
