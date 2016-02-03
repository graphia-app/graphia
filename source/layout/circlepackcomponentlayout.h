#ifndef CIRCLEPACKCOMPONENTLAYOUT_H
#define CIRCLEPACKCOMPONENTLAYOUT_H

#include "componentlayout.h"

class CirclePackComponentLayout : public ComponentLayout
{
private:
    void executeReal(const Graph &graph, const std::vector<ComponentId>& componentIds,
                     ComponentLayoutData &componentLayoutData);
};

#endif // CIRCLEPACKCOMPONENTLAYOUT_H
