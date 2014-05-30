#include "circlepackingcomponentlayout.h"

#include "../utils.h"

#include <QtAlgorithms>

static ComponentPositions* sortComponentPositions;
static bool distanceFromOriginLessThan(const ComponentId& a, const ComponentId& b)
{
    const QVector2D& positionA = (*sortComponentPositions)[a];
    const QVector2D& positionB = (*sortComponentPositions)[b];

    return positionA.lengthSquared() < positionB.lengthSquared();
}

void CirclePackingComponentLayout::executeReal(uint64_t iteration)
{
    QList<ComponentId> componentIds = *graph().componentIds();
    const float COMPONENT_SEPARATION = 2.0f;
    ComponentPositions& componentPositions = *this->_componentPositions;
    ComponentArray<float> componentRadii(const_cast<Graph&>(graph()));

    for(ComponentId componentId : componentIds)
        componentRadii[componentId] = radiusOfComponent(componentId) + COMPONENT_SEPARATION;

    if(iteration == 0)
    {
        for(ComponentId componentId : componentIds)
            componentPositions[componentId] = Utils::randQVector2D(-1.0f, 1.0f);
    }

    sortComponentPositions = &componentPositions;
    qSort(componentIds.begin(), componentIds.end(), distanceFromOriginLessThan);

    QVector<QVector2D> moves(componentPositions.size());

    for(int i = 0; i < componentIds.size() - 1; i++)
    {
        for(int j = i + 1; j < componentIds.size(); j++)
        {
            if(i == j)
                continue;

            ComponentId componentAId = componentIds[i];
            ComponentId componentBId = componentIds[j];
            QVector2D& positionA = componentPositions[componentAId];
            QVector2D& positionB = componentPositions[componentBId];

            QVector2D separation = positionB - positionA;
            float radii = componentRadii[componentAId] + componentRadii[componentBId];

            float d = separation.lengthSquared();

            if(d < (radii * radii))
            {
                separation.normalize();
                separation *= ((radii - std::sqrt(d)) * 0.5f);
                moves[componentAId] -= separation;
                moves[componentBId] += separation;
            }
        }
    }

    componentPositions.lock();
    for(ComponentId componentId : componentIds)
    {
        componentPositions[componentId] += moves[componentId];
        componentPositions[componentId] *= 0.9999f; //FIXME: this is not stable
    }
    componentPositions.unlock();

    emit changed();
}
