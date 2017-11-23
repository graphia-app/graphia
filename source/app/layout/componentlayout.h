#ifndef COMPONENTLAYOUT_H
#define COMPONENTLAYOUT_H

#include "shared/graph/grapharray.h"
#include "maths/circle.h"

#include <QRectF>

using ComponentLayoutData = ComponentArray<Circle, LockingGraphArray>;

class Graph;

class ComponentLayout
{
public:
    virtual ~ComponentLayout() = default;

    void execute(const Graph& graph, const std::vector<ComponentId>& componentIds,
                 ComponentLayoutData &componentLayoutData);

    float boundingWidth() const { return _boundingBox.width(); }
    float boundingHeight() const { return _boundingBox.height(); }

private:
    virtual void executeReal(const Graph& graph, const std::vector<ComponentId>& componentIds,
                             ComponentLayoutData &componentLayoutData) = 0;

    QRectF _boundingBox;
};

#endif // COMPONENTLAYOUT_H

