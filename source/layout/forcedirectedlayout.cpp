#include "forcedirectedlayout.h"
#include "randomlayout.h"
#include "barneshuttree.h"

#include "../utils/utils.h"
#include "../utils/threadpool.h"
#include "../ui/preferences.h"

static QVector3D normalized(const QVector3D& v)
{
    float lengthSq = v.lengthSquared();
    if (qFuzzyIsNull(lengthSq - 1.0f))
        return v;
    else if (!qIsNull(lengthSq))
        return v / std::sqrt(lengthSq);

    return QVector3D();
}

// This promotes movements where the direction is constant and mitigates movements when the direction changes
static void dampOscillations(QVector3D& previous, QVector3D& next)
{
    const float previousLength = previous.length();
    float nextLength = next.length();
    const float MAX_DISPLACEMENT = 10.0f;

    // Filter large displacements that can induce instability
    if(nextLength > MAX_DISPLACEMENT)
    {
        nextLength = MAX_DISPLACEMENT;
        next = normalized(next) * nextLength;
    }

    if(previousLength > 0.0f && nextLength > 0.0f)
    {
        const float dotProduct = QVector3D::dotProduct(previous / previousLength, next / nextLength);

        // http://www.wolframalpha.com/input/?i=plot+0.5x%5E2%2B1.2x%2B1+from+x%3D-1to1
        const float f = (0.5f * dotProduct * dotProduct) + (1.2f * dotProduct) + 1.0f;

        if(nextLength > (previousLength * f))
        {
            const float r = previousLength / nextLength;
            next *= (f * r);
        }
    }

    previous = next;
}

void ForceDirectedLayout::executeReal(bool firstIteration)
{
    _prevDisplacements.resize(positions().size());
    _displacements.resize(positions().size());

    if(firstIteration)
    {
        RandomLayout randomLayout(graph(), positions());

        randomLayout.setSpread(10.0f);
        randomLayout.execute(firstIteration);

        for(NodeId nodeId : nodeIds())
            _prevDisplacements[nodeId] = QVector3D(0.0f, 0.0f, 0.0f);
    }

    for(NodeId nodeId : nodeIds())
        _displacements[nodeId] = QVector3D(0.0f, 0.0f, 0.0f);

    BarnesHutTree barnesHutTree;
    barnesHutTree.build(graph(), positions());

    float SPRING_LENGTH = _settings->getParam("Spring Length")->value;
    float REPULSIVE_FORCE = _settings->getParam("Repulsive Force")->value;
    float ATTRACTIVE_FORCE = _settings->getParam("Attractive Force")->value;

    // Repulsive forces
    auto repulsiveResults = concurrent_for(nodeIds().begin(), nodeIds().end(),
        [this, &barnesHutTree, REPULSIVE_FORCE ](const NodeId nodeId)
        {
            if(shouldCancel())
                return;

            _displacements[nodeId] -= barnesHutTree.evaluateKernel(nodeId,
                [REPULSIVE_FORCE](int mass, const QVector3D& difference, float distanceSq)
                {
                    return REPULSIVE_FORCE * difference * mass / (0.0001f + distanceSq);
                });
        }, false);

    // Attractive forces
    auto attractiveResults = concurrent_for(edgeIds().begin(), edgeIds().end(),
        [this, SPRING_LENGTH, ATTRACTIVE_FORCE](const EdgeId edgeId)
        {
            if(shouldCancel())
                return;

            const Edge& edge = graph().edgeById(edgeId);
            if(!edge.isLoop())
            {
                const QVector3D difference = positions().get(edge.targetId()) - positions().get(edge.sourceId());
                float distanceSq = difference.lengthSquared();
                //const float SPRING_LENGTH = 10.0f;
                float force = ATTRACTIVE_FORCE * distanceSq / (SPRING_LENGTH * SPRING_LENGTH * SPRING_LENGTH);

                _displacements[edge.targetId()] -= (force * difference);
                _displacements[edge.sourceId()] += (force * difference);
            }
        }, false);

    repulsiveResults.wait();
    attractiveResults.wait();

    if(shouldCancel())
        return;

    concurrent_for(nodeIds().begin(), nodeIds().end(),
        [this](const NodeId& nodeId)
        {
            dampOscillations(_prevDisplacements[nodeId], _displacements[nodeId]);
        });

    // Apply the forces
    for(auto nodeId : nodeIds())
        positions().set(nodeId, positions().get(nodeId) + _displacements[nodeId]);
}
