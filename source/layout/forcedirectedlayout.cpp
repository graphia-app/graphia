#include "forcedirectedlayout.h"
#include "randomlayout.h"
#include "barneshuttree.h"

#include "../utils/utils.h"

ThreadPool ForceDirectedLayout::_threadPool("FDPLayout");

// This promotes movements where the direction is constant and mitigates movements when the direction changes
static void dampOscillations(QVector3D& previous, QVector3D& next)
{
    const float previousLength = previous.length();
    const float nextLength = next.length();

    if(previousLength > 0.0f && nextLength > 0.0f)
    {
        const float dotProduct = QVector3D::dotProduct(previous.normalized(), next.normalized());

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

void ForceDirectedLayout::executeReal(uint64_t iteration)
{
    const std::vector<NodeId>& nodeIds = _graph.nodeIds();
    const std::vector<EdgeId>& edgeIds = _graph.edgeIds();

    _prevDisplacements.resize(_positions.size());
    _displacements.resize(_positions.size());

    if(iteration == 0)
    {
        RandomLayout randomLayout(_graph, _positions);

        randomLayout.setSpread(10.0f);
        randomLayout.execute(iteration);

        for(NodeId nodeId : nodeIds)
            _prevDisplacements[nodeId] = QVector3D(0.0f, 0.0f, 0.0f);
    }

    for(NodeId nodeId : nodeIds)
        _displacements[nodeId] = QVector3D(0.0f, 0.0f, 0.0f);

    BarnesHutTree barnesHutTree;
    barnesHutTree.build(_graph, _positions);

    // Repulsive forces
    auto repulsiveResults = _threadPool.concurrentForEach(nodeIds.cbegin(), nodeIds.cend(),
        [this, &barnesHutTree](const NodeId& nodeId)
        {
            if(shouldCancel())
                return;

            _displacements[nodeId] -= barnesHutTree.evaluateKernel(nodeId,
                [](int mass, const QVector3D& difference, float distanceSq)
                {
                    return difference * mass / (0.0001f + distanceSq);
                });
        }, false);

    // Attractive forces
    auto attractiveResults = _threadPool.concurrentForEach(edgeIds.cbegin(), edgeIds.cend(),
        [this](const EdgeId& edgeId)
        {
            if(shouldCancel())
                return;

            const Edge& edge = _graph.edgeById(edgeId);
            if(!edge.isLoop())
            {
                const QVector3D difference = _positions.get(edge.targetId()) - _positions.get(edge.sourceId());
                float distanceSq = difference.lengthSquared();
                const float SPRING_LENGTH = 10.0f;
                const float force = distanceSq / (SPRING_LENGTH * SPRING_LENGTH * SPRING_LENGTH);

                _displacements[edge.targetId()] -= (force * difference);
                _displacements[edge.sourceId()] += (force * difference);
            }
        }, false);

    repulsiveResults.wait();
    attractiveResults.wait();

    if(shouldCancel())
        return;

    _threadPool.concurrentForEach(nodeIds.cbegin(), nodeIds.cend(),
        [this](const NodeId& nodeId)
        {
            dampOscillations(_prevDisplacements[nodeId], _displacements[nodeId]);
        });

    // Apply the forces
    _positions.update(_graph, [this](NodeId nodeId, const QVector3D& position)
        {
            return position + _displacements[nodeId];
        }, 0.4f, 8);
}
