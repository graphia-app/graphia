#ifndef POWEROF2GRIDCOMPONENTLAYOUT_H
#define POWEROF2GRIDCOMPONENTLAYOUT_H

#include "componentlayout.h"

class PowerOf2GridComponentLayout : public ComponentLayout
{
public:
    void execute(const Graph &graph, const std::vector<ComponentId>& componentIds,
                 int width, int height, ComponentLayoutData &componentLayoutData);
};

#endif // POWEROF2GRIDCOMPONENTLAYOUT_H
