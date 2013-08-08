#include "eadeslayout.h"

#include "../utils.h"

#define TAYLOR_LNX(x) (2.0*(((x)-1.0)/((x)+1.0)))

#define FACTOR        0.6

#define REPULSE(x)    (FACTOR*(1.0/((x)*(x))))
#define ATTRACT(x)    (FACTOR*TAYLOR_LNX(x))

void EadesLayout::execute()
{
    NodeArray<QVector3D>& positions = *this->positions;
    NodeArray<QVector3D> moves(graph());

    QVector3D axialDirections[] =
    {
        {  1.0,  0.0,  0.0 },
        {  0.0,  1.0,  0.0 },
        {  0.0,  0.0,  1.0 },
        {  0.0,  0.0, -1.0 },
        {  0.0, -1.0,  0.0 },
        { -1.0,  0.0,  0.0 },
    };
    int axialDirectionIndex = 0;

    // Repulsive forces
    for(Graph::NodeId i : graph().nodeIds())
    {
        for(Graph::NodeId j : graph().nodeIds())
        {
            if( i != j )
            {
                QVector3D difference = positions[j] - positions[i];
                qreal distance = difference.length();
                qreal force;
                QVector3D direction;

                if(Utils::valueIsCloseToZero(distance))
                {
                    // Pick a "random" direction
                    force = 1.0;
                    direction = axialDirections[axialDirectionIndex];
                    axialDirectionIndex = (axialDirectionIndex + 1) % 6;
                }
                else
                {
                    force = REPULSE(distance);
                    direction = difference.normalized();
                }

                moves[j] += (force * direction);
            }
        }
    }

    // Attractive forces
    for(Graph::Edge* edge : graph().edges())
    {
        if(!edge->isLoop())
        {
            QVector3D difference = positions[edge->sourceId()] - positions[edge->targetId()];
            qreal distance = difference.length();

            if(distance > 0.0)
            {
                qreal force = ATTRACT(distance);
                QVector3D direction = difference.normalized();
                moves[edge->targetId()] -= (force * direction);
                moves[edge->sourceId()] += (force * direction);
            }
        }
    }

    // Apply the moves
    for(Graph::NodeId nodeId : graph().nodeIds())
        positions[nodeId] += (moves[nodeId] * 0.1); //FIXME not sure what this constant is about, damping?

    emit complete();
}
