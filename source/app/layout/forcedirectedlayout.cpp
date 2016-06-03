#include "forcedirectedlayout.h"
#include "randomlayout.h"
#include "barneshuttree.h"

#include "../utils/utils.h"
#include "../utils/threadpool.h"
#include "../utils/preferences.h"

template<typename T> float meanWeightedAvgBuffer(int start, int end, const T& buffer)
{
    float average = 0.0f;
    int size = end - start;
    float gaussSum = (size) * (size + 1) / 2.0f;

    for(int i = start; i < end; ++i)
        average += buffer.at(i) * ((i - start) + 1) / gaussSum;

    return average / std::abs(start - end);
}

static QVector3D normalized(const QVector3D& v)
{
    float lengthSq = v.lengthSquared();
    if(qFuzzyIsNull(lengthSq - 1.0f))
        return v;
    else if(!qIsNull(lengthSq))
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
        RandomLayout randomLayout(graphComponent(), positions());

        randomLayout.setSpread(10.0f);
        randomLayout.execute(firstIteration);

        for(NodeId nodeId : nodeIds())
            _prevDisplacements[nodeId] = QVector3D(0.0f, 0.0f, 0.0f);
    }

    for(NodeId nodeId : nodeIds())
        _displacements[nodeId] = QVector3D(0.0f, 0.0f, 0.0f);

    BarnesHutTree barnesHutTree;
    barnesHutTree.build(graphComponent(), positions());

    float REPULSIVE_FORCE = _settings->valueOf("RepulsiveForce");
    float ATTRACTIVE_FORCE = _settings->valueOf("AttractiveForce");

    // Repulsive forces
    auto repulsiveResults = concurrent_for(nodeIds().begin(), nodeIds().end(),
        [this, &barnesHutTree, REPULSIVE_FORCE](const NodeId nodeId)
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
        [this, ATTRACTIVE_FORCE](const EdgeId edgeId)
        {
            if(shouldCancel())
                return;

            const IEdge& edge = graphComponent().graph().edgeById(edgeId);
            if(!edge.isLoop())
            {
                const QVector3D difference = positions().get(edge.targetId()) - positions().get(edge.sourceId());
                float distanceSq = difference.lengthSquared();
                const float SPRING_LENGTH = 10.0f;
                const float force = ATTRACTIVE_FORCE * distanceSq / (SPRING_LENGTH * SPRING_LENGTH * SPRING_LENGTH);

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

    // There are three main phases which decide when to stop the layout.
    // The phases operate primarily on the stddev of the forces within the graph
    //
    // Initial   - Initial phase, If the std dev drops below MINIMUM_STDDEV_THRESHOLD.
    //             this will move the phase onto FineTune. If the std dev oscillates
    //             enough, will move the phase onto Oscillate
    // FineTune  - Allows the layout algorithm to further calculate small layout changes
    //             until the change amount falls below FINETUNE_STDDEV_DELTA, where it moves
    //             the phase to Finished
    // Oscillate - Monitors the delta of Stddev over OSCILLATE_DELTA_SAMPLE_SIZE steps
    //             OSCILLATE_RUN_COUNT times. If delta is less than OSCILLATE_STDDEV_DELTA_PERCENT
    //             the layout finishes, if OSCILLATE_RUN_COUNT is reached, returns phase to initial.
    // Finished  - Finish layout
    //

    NodeArray<float> displacementSizes(graphComponent().graph());
    concurrent_for(nodeIds().begin(), nodeIds().end(),
        [this, &displacementSizes](const NodeId& nodeId)
        {
            displacementSizes[nodeId] = _displacements[nodeId].length();
        });

    // Calculate force averages
    float deltaForceTotal = 0.0f;
    for(auto nodeId : nodeIds())
        deltaForceTotal += displacementSizes[nodeId];

    _forceMean = deltaForceTotal / nodeIds().size();

    // Calculate Standard Deviation
    float variance = 0.0f;
    for(auto nodeId : nodeIds())
    {
        float d = displacementSizes[nodeId] - _forceMean;
        variance += (d * d);
    }

    _forceStdDeviation = std::sqrt(variance / nodeIds().size());
    switch(_changeDetectionPhase)
    {
        case ChangeDetectionPhase::Initial:
            initialChangeDetection();
            break;

        case ChangeDetectionPhase::FineTune:
            fineTuneChangeDetection();
            break;

        case ChangeDetectionPhase::Oscillate:
            oscillateChangeDetection();
            break;

        case ChangeDetectionPhase::Finished:
        default:
            break;
    }

    _prevStdDevs.push_back(_forceStdDeviation);
    _prevAvgForces.push_back(_forceMean);
    _prevCaptureStdDevs.push_back(_forceStdDeviation);
}

// Initial phase. If the std dev drops below MINIMUM_STDDEV_THRESHOLD this will move the phase onto
// FineTune. If the std dev oscillates enough, will move the phase onto Oscillate
void ForceDirectedLayout::initialChangeDetection()
{
    if(_forceStdDeviation < MINIMUM_STDDEV_THRESHOLD && _forceMean < MAXIMUM_AVG_FORCE_FOR_STOP)
        _changeDetectionPhase = ChangeDetectionPhase::FineTune;

    if(_prevCaptureStdDevs.full())
    {
        float currentSmoothedStdDev = meanWeightedAvgBuffer(static_cast<int>(_prevCaptureStdDevs.size()) - INITIAL_SMOOTHING_SIZE,
                                                    static_cast<int>(_prevCaptureStdDevs.size()),
                                                    _prevCaptureStdDevs);

        float previousSmoothedStdDev = meanWeightedAvgBuffer(static_cast<int>(_prevCaptureStdDevs.size()) - (2 * INITIAL_SMOOTHING_SIZE),
                                                     static_cast<int>(_prevCaptureStdDevs.size()) - INITIAL_SMOOTHING_SIZE,
                                                     _prevCaptureStdDevs);

        // Long step sample (For unstable graphs)
        if(_increasingStdDevIterationCount >= STDDEV_INCREASES_BEFORE_SWITCH_TO_OSCILLATE)
            _changeDetectionPhase = ChangeDetectionPhase::Oscillate;

        if(currentSmoothedStdDev > previousSmoothedStdDev)
            _increasingStdDevIterationCount++;
    }
}

// Allows the layout algorithm to further calculate small layout changes until the change amount
// falls below FINETUNE_STDDEV_DELTA, where it moves the phase to Finished
void ForceDirectedLayout::fineTuneChangeDetection()
{
    if(_prevAvgForces.full() && _prevStdDevs.full())
    {
        float prevAvgStdDev = meanWeightedAvgBuffer(static_cast<int>(_prevStdDevs.size()) - 2 * FINETUNE_SMOOTHING_SIZE,
                                            static_cast<int>(_prevStdDevs.size()) - FINETUNE_SMOOTHING_SIZE,
                                            _prevStdDevs);

        float curAvgStdDev = meanWeightedAvgBuffer(static_cast<int>(_prevStdDevs.size()) - FINETUNE_SMOOTHING_SIZE,
                                           static_cast<int>(_prevStdDevs.size()),
                                           _prevStdDevs);

        float delta = (prevAvgStdDev - curAvgStdDev);
        if(delta < FINETUNE_STDDEV_DELTA && delta >= 0.0f)
            _changeDetectionPhase = ChangeDetectionPhase::Finished;

    }
}

// Monitors the delta of Stddev over OSCILLATE_DELTA_SAMPLE_SIZE steps OSCILLATE_RUN_COUNT times.
// If delta is less than OSCILLATE_STDDEV_DELTA_PERCENT the layout finishes, if OSCILLATE_RUN_COUNT
// is reached, returns phase to initial.
void ForceDirectedLayout::oscillateChangeDetection()
{
    if(_prevCaptureStdDevs.full())
    {
        float averageCap = meanWeightedAvgBuffer(0, OSCILLATE_DELTA_SAMPLE_SIZE, _prevCaptureStdDevs);

        auto deltaStdDev = _prevUnstableStdDev - averageCap;
        auto percentDelta = OSCILLATE_STDDEV_DELTA_PERCENT;
        if(_prevUnstableStdDev != 0.0f)
            percentDelta = (deltaStdDev / _prevUnstableStdDev) * 100.0f;

        if(std::abs(percentDelta) < OSCILLATE_STDDEV_DELTA_PERCENT)
        {
            _changeDetectionPhase = ChangeDetectionPhase::Finished;
            _increasingStdDevIterationCount = 0;
            _unstableIterationCount = 0;
            _prevCaptureStdDevs.clear();
        }

        _prevUnstableStdDev = averageCap;
        _prevCaptureStdDevs.clear();
        _unstableIterationCount++;

        if(_unstableIterationCount >= OSCILLATE_RUN_COUNT)
        {
            _changeDetectionPhase = ChangeDetectionPhase::Initial;
            _increasingStdDevIterationCount = 0;
            _unstableIterationCount = 0;
        }
    }
}
