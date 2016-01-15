#ifndef COMPONENTLAYOUT_H
#define COMPONENTLAYOUT_H

#include "../graph/graph.h"
#include "../graph/grapharray.h"
#include "../utils/utils.h"

#include <QRectF>

using ComponentLayoutData = ComponentArray<QRectF, u::Locking>;

class ComponentLayout
{
public:
    virtual ~ComponentLayout() {}

    virtual void execute(const Graph &graph, const std::vector<ComponentId>& componentIds,
                         int width, int height, ComponentLayoutData &componentLayoutData) = 0;
};

#endif // COMPONENTLAYOUT_H

