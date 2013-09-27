#include "radialcirclecomponentlayout.h"

#include "../utils.h"

#include <cmath>

#include <QList>
#include <QMutexLocker>
#include <QtAlgorithms>

static float halfAngleOfComponent(float componentRadius, float placementRadius)
{
    // Half the angle of a component is described by a right angled triangle
    // with the hypotenuse equal to the sum of the placment radius and the
    // component radius, and the opposite equal to the component radius

    float totalRadius = componentRadius + placementRadius;
    return std::asin(componentRadius / totalRadius);
}

struct PlacementRegion
{
    float startAngle;
    float endAngle;
    float angle() const
    {
        return endAngle - startAngle;
    }

    float radius;
};

struct PendingComponentPlacement
{
    ComponentId id;
    float radius;
    float angle;
};

static void makePlacements(PlacementRegion& placementRegion,
                           QList<PendingComponentPlacement>& placements,
                           ComponentPositions& componentPositions,
                           QList<PlacementRegion>& placementRegions)
{
    float totalAngle = 0.0f;
    for(PendingComponentPlacement pcp : placements)
        totalAngle += pcp.angle;

    float remainingAngle = placementRegion.angle() - totalAngle;
    float perComponentPaddingAngle = remainingAngle / placements.size();

    float placementAngle = placementRegion.startAngle;
    for(PendingComponentPlacement pcp : placements)
    {
        float paddedAngle = pcp.angle + perComponentPaddingAngle;

        placementAngle += (0.5f * paddedAngle);
        QVector2D nextPosition = QVector2D(std::sin(placementAngle), std::cos(placementAngle)) *
                (placementRegion.radius + pcp.radius);
        componentPositions[pcp.id] = nextPosition;

        PlacementRegion childPlacementRegion =
        {
            placementAngle - (0.5f * paddedAngle),
            placementAngle + (0.5f * paddedAngle),
            placementRegion.radius + (2.0f * pcp.radius)
        };

        placementRegions.append(childPlacementRegion);
        placementAngle += (0.5f * paddedAngle);
    }

    placements.clear();
}

void RadialCircleComponentLayout::executeReal()
{
    QList<ComponentId> componentIds = *graph().componentIds();
    const float COMPONENT_SEPARATION = 1.0f;
    ComponentPositions& componentPositions = *this->componentPositions;

    qStableSort(componentIds.begin(), componentIds.end(),
        [&](const ComponentId& a, const ComponentId& b)
        {
            const int numNodesA = graph().componentById(a)->numNodes();
            const int numNodesB = graph().componentById(b)->numNodes();

            return numNodesA >= numNodesB;
        });

    QMutexLocker(&componentPositions.mutex());

    // Place first component in the centre
    componentPositions[componentIds[0]] = QVector2D(0.0f, 0.0f);

    if(componentIds.size() == 1)
        return;

    float secondComponentHalfAngle = halfAngleOfComponent(
            radiusOfComponent(componentIds[1]) + COMPONENT_SEPARATION,
            radiusOfComponent(componentIds[0]) + COMPONENT_SEPARATION);

    QList<PlacementRegion> placementRegions;
    PlacementRegion currentPlacementRegion =
    {
        0.5f * Utils::Pi() - secondComponentHalfAngle,
        2.5f * Utils::Pi() - secondComponentHalfAngle,
        radiusOfComponent(componentIds[0]) + COMPONENT_SEPARATION
    };

    float angleRemaining = currentPlacementRegion.angle();

    QList<PendingComponentPlacement> placements;

    for(int i = 1; i < componentIds.size(); i++)
    {
        ComponentId componentId = componentIds[i];
        float componentRadius = radiusOfComponent(componentId) + COMPONENT_SEPARATION;
        float componentHalfAngle = halfAngleOfComponent(componentRadius, currentPlacementRegion.radius);
        float componentAngle = 2.0f * componentHalfAngle;

        if((angleRemaining - componentAngle) < 0.0f)
        {
            // Make any pending placements first
            makePlacements(currentPlacementRegion, placements, componentPositions, placementRegions);

            bool foundNewRegion = false;
            float largestAngle = 0.0f;
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
                // Didn't find a large enough region, so pick the biggest and increase its placement radius to suit
                currentPlacementRegion = placementRegions[largestAngleIndex];
                currentPlacementRegion.radius =
                        ((componentRadius / std::sin(0.5f * largestAngle)) - componentRadius) + COMPONENT_SEPARATION;
            }

            angleRemaining = currentPlacementRegion.angle();
        }

        PendingComponentPlacement placement =
        {
            componentId,
            componentRadius,
            componentAngle
        };

        placements.append(placement);

        angleRemaining -= componentAngle;
    }

    makePlacements(currentPlacementRegion, placements, componentPositions, placementRegions);

    emit changed();
}
