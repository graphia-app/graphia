#include "linearcomponentlayout.h"

#include "../utils.h"

void LinearComponentLayout::executeReal()
{
    ComponentArray<QVector2D>& componentPositions = *this->componentPositions;
    const ReadOnlyGraph& firstComponent = *graph().firstComponent();
    const float COMPONENT_SEPARATION = 2.0f;

    float offset = -(NodeLayout::boundingCircleRadiusInXY(firstComponent, *nodePositions) +
                     COMPONENT_SEPARATION);

    componentPositions.lock();
    for(ComponentId componentId : *graph().componentIds())
    {
        const ReadOnlyGraph& component = *graph().componentById(componentId);
        const float radius = NodeLayout::boundingCircleRadiusInXY(component, *nodePositions);

        offset += (radius + COMPONENT_SEPARATION);
        componentPositions[componentId] = QVector2D(offset, 0.0f);
        offset += (radius + COMPONENT_SEPARATION);
    }
    componentPositions.unlock();

    emit changed();
}
