#ifndef COMPONENTLAYOUT_H
#define COMPONENTLAYOUT_H

#include "../graph/graph.h"
#include "shared/graph/grapharray.h"
#include "shared/utils/utils.h"
#include "../maths/circle.h"

#include <QRectF>

using ComponentLayoutData = ComponentArray<Circle, u::Locking>;

class ComponentLayout
{
public:
    virtual ~ComponentLayout() {}

    void execute(const Graph &graph, const std::vector<ComponentId>& componentIds,
                 ComponentLayoutData &componentLayoutData);

    float boundingWidth() const { return _boundingBox.width(); }
    float boundingHeight() const { return _boundingBox.height(); }

private:
    virtual void executeReal(const Graph &graph, const std::vector<ComponentId>& componentIds,
                             ComponentLayoutData &componentLayoutData) = 0;

    QRectF _boundingBox;
};

#endif // COMPONENTLAYOUT_H

