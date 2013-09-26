#include "radialcirclecomponentlayout.h"

#include "../utils.h"

#include <cmath>

#include <QList>
#include <QtAlgorithms>

static float halfAngleOfComponent(float componentRadius, float placementRadius)
{
    // Half the angle of a component is described by a right angled triangle
    // with the hypotenuse equal to the sum of the placment radius and the
    // component radius, and the opposite equal to the component radius

    float totalRadius = componentRadius + placementRadius;
    return std::asin(componentRadius / totalRadius);
}

void RadialCircleComponentLayout::executeReal()
{
    QList<ComponentId> componentIds = *graph().componentIds();
    const float COMPONENT_SEPARATION = 2.0f;
    ComponentPositions& componentPositions = *this->componentPositions;

    qStableSort(componentIds.begin(), componentIds.end(),
        [&](const ComponentId& a, const ComponentId& b)
        {
            const int numNodesA = graph().componentById(a)->numNodes();
            const int numNodesB = graph().componentById(b)->numNodes();

            return numNodesA >= numNodesB;
        });

    componentPositions.lock();

    // Place first component in the centre
    componentPositions[componentIds[0]] = QVector2D(0.0f, 0.0f);

    struct PlacementRegion
    {
        float startAngle;
        float endAngle;
        float angle() const { return endAngle - startAngle; }
        float radius;
    };

    QList<PlacementRegion> placementRegions;
    PlacementRegion currentPlacementRegion =
    {
        0.0f,
        2.0f * Utils::Pi(),
        radiusOfComponent(componentIds[0]) + COMPONENT_SEPARATION
    };

    float angleRemaining = currentPlacementRegion.angle();
    float angleOfPlacement = 0.0f;

    for(int i = 1; i < componentIds.size(); i++)
    {
        ComponentId componentId = componentIds[i];
        float componentRadius = radiusOfComponent(componentId) + COMPONENT_SEPARATION;
        float componentHalfAngle = halfAngleOfComponent(componentRadius, currentPlacementRegion.radius);
        float componentAngle = 2.0f * componentHalfAngle;

        if((angleRemaining - componentAngle) < 0.0f)
        {
            bool foundNewRegion = false;
            int largestAngle = 0.0f;
            int largestAngleIndex = -1;

            // Can't fit the component in this region so we need to find a new one
            for(int j = 0; j < placementRegions.size(); j++)
            {
                PlacementRegion placementRegion = placementRegions[j];

                if(placementRegion.angle() > largestAngle)
                {
                    largestAngleIndex = j;
                    largestAngle = placementRegion.angle();
                }

                if(placementRegion.angle() >= componentAngle)
                {
                    currentPlacementRegion = placementRegion;
                    placementRegions.removeAt(j);
                    foundNewRegion = true;
                    break;
                }
            }

            if(!foundNewRegion)
            {
                // Didn't find a large enough region, so pick the biggest and increase its placement radius
                currentPlacementRegion = placementRegions[largestAngleIndex];
                currentPlacementRegion.radius =
                        ((componentRadius / std::sin(largestAngle)) - componentRadius) + COMPONENT_SEPARATION;
            }

            angleRemaining = currentPlacementRegion.angle();
            angleOfPlacement = currentPlacementRegion.startAngle + componentHalfAngle;
        }

        PlacementRegion childPlacementRegion =
        {
            angleOfPlacement - componentHalfAngle,
            angleOfPlacement + componentHalfAngle,
            currentPlacementRegion.radius + (2.0f * componentRadius)
        };

        placementRegions.append(childPlacementRegion);

        QVector2D nextPosition = QVector2D(std::sin(angleOfPlacement), std::cos(angleOfPlacement)) *
                (currentPlacementRegion.radius + componentRadius);
        componentPositions[componentId] = nextPosition;
        angleRemaining -= componentAngle;
        angleOfPlacement += componentAngle;
    }

    componentPositions.unlock();

    emit changed();
}
