#ifndef POWEROF2GRIDCOMPONENTLAYOUT_H
#define POWEROF2GRIDCOMPONENTLAYOUT_H

#include "componentlayout.h"

class PowerOf2GridComponentLayout : public ComponentLayout
{
private:
    void executeReal(const Graph &graph, const std::vector<ComponentId>& componentIds,
                     ComponentLayoutData &componentLayoutData);
};

#endif // POWEROF2GRIDCOMPONENTLAYOUT_H
