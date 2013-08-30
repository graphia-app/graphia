#include "normalisinglayout.h"

#include "../maths/boundingbox.h"
#include "utils.h"

#include <cmath>

static float calculateScale(float nodeDensity, int numNodes, float currentVolume)
{
    if(Utils::valueIsCloseToZero(currentVolume))
        return 1.0f;

    float desiredVolume = numNodes / nodeDensity;
    float volumeScale = desiredVolume / currentVolume;

    return std::pow(volumeScale, 1.0f/3.0f);
}

void NormalisingLayout::executeReal()
{
    NodeArray<QVector3D>& positions = *this->positions;
    const QVector3D& firstNodePosition = positions[graph().nodeIds()[0]];
    BoundingBox boundingBox(firstNodePosition, firstNodePosition);

    QVector3D centerOfMass;
    for(NodeId nodeId : graph().nodeIds())
    {
        const QVector3D& nodePosition = positions[nodeId];

        centerOfMass += (nodePosition / graph().numNodes());
        boundingBox.expandToInclude(nodePosition);
    }

    QVector3D translate = -centerOfMass;
    float scale;
    float maxLength = boundingBox.maxLength();

    if(_nodeDensity > 0.0f)
        scale = calculateScale(_nodeDensity, graph().numNodes(), boundingBox.volume());
    else if(maxLength > 0.0f)
        scale = (_spread * 2.0f) / maxLength;
    else
        scale = 1.0f;

    _maxDimension = maxLength * scale;

    positions.lock();
    for(NodeId nodeId : graph().nodeIds())
    {
        positions[nodeId] = (positions[nodeId] + translate) * scale;
    }
    positions.unlock();

    emit complete();
}
