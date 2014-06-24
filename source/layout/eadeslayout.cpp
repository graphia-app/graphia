#include "eadeslayout.h"
#include "randomlayout.h"

#include "../utils.h"

/*static float lnTaylorSeries(float x, int order)
{
    int   i, j, k;
    float result = 0.0f;
    float xMinus1 = x - 1.0f;
    float xPlus1  = x + 1.0f;
    float term = xMinus1 / xPlus1;
    float y;

    for(i = 0; i <= order; i++)
    {
        k = i * 2 + 1;

        y = term;

        for(j = 1; j < k; j++)
            y *= term;

        result += y *(1.0f / (float)k);
    }

    return 2.0f * result;
}*/

#define TAYLOR_LNX(x) (2.0f*(((x)-1.0f)/((x)+1.0f)))

#define FACTOR        0.6f

#define REPULSE(x)    (FACTOR*(1.0f/((x)*(x))))
#define REPULSE_SQ(x) (FACTOR*(1.0f/(x)))
//#define ATTRACT(x)    (FACTOR*lnTaylorSeries(x, 4))
#define ATTRACT(x)    (FACTOR*TAYLOR_LNX(x))

void EadesLayout::executeReal(uint64_t iteration)
{
    if(iteration == 0)
    {
        RandomLayout randomLayout(_graph, _positions);

        randomLayout.setSpread(10.0f);
        randomLayout.execute(iteration);
    }

    auto& positions = *_positions;

    _moves.resize(positions.size());

    const std::vector<NodeId>& nodeIds = _graph->nodeIds();
    const std::vector<EdgeId>& edgeIds = _graph->edgeIds();

    for(NodeId i : _graph->nodeIds())
        _moves[i] = QVector3D(0.0f, 0.0f, 0.0f);

    int numNodes = static_cast<int>(nodeIds.size());

    // Repulsive forces
    for(int i = 0; i < numNodes - 1; i++)
    {
        for(int j = i + 1; j < numNodes; j++)
        {
            if(shouldCancel())
                return;

            if(i != j)
            {
                NodeId nodeAId = nodeIds[i];
                NodeId nodeBId = nodeIds[j];
                QVector3D difference = positions[nodeBId] - positions[nodeAId];
                float x = difference.x(); float y = difference.y(); float z = difference.z();
                float distance = x * x + y * y + z * z;
                float force = REPULSE_SQ(distance);
                QVector3D direction = Utils::fastNormalize(difference, distance);

                _moves[nodeAId] -= (force * direction);
                _moves[nodeBId] += (force * direction);
            }
        }
    }

    // Attractive forces
    for(EdgeId edgeId : edgeIds)
    {
        if(shouldCancel())
            return;

        const Edge& edge = _graph->edgeById(edgeId);
        if(!edge.isLoop())
        {
            QVector3D difference = positions[edge.targetId()] - positions[edge.sourceId()];
            qreal distance = difference.length();

            if(distance > 0.0f)
            {
                qreal force = ATTRACT(distance);
                QVector3D direction = difference.normalized();
                _moves[edge.targetId()] -= (force * direction);
                _moves[edge.sourceId()] += (force * direction);
            }
        }
    }

    positions.lock();
    // Apply the moves
    for(NodeId nodeId : nodeIds)
        positions[nodeId] += (_moves[nodeId] * 0.1f); //FIXME not sure what this constant is about, damping?
    positions.unlock();

    emit changed();
}
