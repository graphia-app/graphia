#include "eadeslayout.h"
#include "randomlayout.h"

#include "../utils.h"

#define TAYLOR_LNX(x) (2.0f*(((x)-1.0f)/((x)+1.0f)))

#define FACTOR        0.6f

#define REPULSE(x)    (FACTOR*(1.0f/((x)*(x))))
#define REPULSE_SQ(x) (FACTOR*(1.0f/(x)))
#define ATTRACT(x)    (FACTOR*TAYLOR_LNX(x))

void EadesLayout::execute()
{
    NodeArray<QVector3D>& positions = *this->positions;
    NodeArray<QVector3D> moves(graph());

    QVector3D axialDirections[] =
    {
        {  1.0f,  0.0f,  0.0f },
        {  0.0f,  1.0f,  0.0f },
        {  0.0f,  0.0f,  1.0f },
        {  0.0f,  0.0f, -1.0f },
        {  0.0f, -1.0f,  0.0f },
        { -1.0f,  0.0f,  0.0f },
    };
    static int axialDirectionIndex = 0;
    static bool firstIteration = true;

    if(firstIteration)
    {
        RandomLayout randomLayout(positions);
        positions.mutex().lock();
        randomLayout.execute();
        positions.mutex().unlock();

        firstIteration = false;
    }

    // Repulsive forces
    for(Graph::NodeId i : graph().nodeIds())
    {
        for(Graph::NodeId j : graph().nodeIds())
        {
            if(i != j)
            {
                QVector3D difference = positions[j] - positions[i];
                qreal distance = difference.lengthSquared();
                qreal force;
                QVector3D direction;

                if(Utils::valueIsCloseToZero(distance))
                {
                    // Pick a "random" direction
                    force = 1.0f;
                    direction = axialDirections[axialDirectionIndex];
                    axialDirectionIndex = (axialDirectionIndex + 1) % 6;
                }
                else
                {
                    force = REPULSE_SQ(distance);
                    //direction = difference.normalized();
                    direction = Utils::FastNormalize(difference);
                }

                moves[j] += (force * direction);
            }
        }
    }

    // Attractive forces
    for(Graph::EdgeId edgeId : graph().edgeIds())
    {
        Graph::Edge& edge = graph().edgeById(edgeId);
        if(!edge.isLoop())
        {
            QVector3D difference = positions[edge.targetId()] - positions[edge.sourceId()];
            qreal distance = difference.length();

            if(distance > 0.0f)
            {
                qreal force = ATTRACT(distance);
                QVector3D direction = difference.normalized();
                moves[edge.targetId()] -= (force * direction);
                moves[edge.sourceId()] += (force * direction);
            }
        }
    }

    positions.mutex().lock();
    // Apply the moves
    for(Graph::NodeId nodeId : graph().nodeIds())
        positions[nodeId] += (moves[nodeId] * 0.1f); //FIXME not sure what this constant is about, damping?
    positions.mutex().unlock();

    emit complete();
}
