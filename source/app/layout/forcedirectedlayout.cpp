/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "forcedirectedlayout.h"
#include "fastinitiallayout.h"
#include "barneshuttree.h"

#include "app/graph/graph.h"
#include "app/graph/graphmodel.h"

#include "app/preferences.h"

#include "shared/utils/threadpool.h"
#include "shared/utils/scopetimer.h"

#include <cmath>

using namespace Qt::Literals::StringLiterals;

static const float MINIMUM_STDDEV_THRESHOLD = 0.008f;
static const float FINETUNE_STDDEV_DELTA = 0.000005f;
static const float OSCILLATE_STDDEV_DELTA_PERCENT = 1.0f;
static const float MAXIMUM_AVG_FORCE_FOR_STOP = 1.0f;
static const size_t OSCILLATE_RUN_COUNT = 5;
static const size_t STDDEV_INCREASES_BEFORE_SWITCH_TO_OSCILLATE = 500;
static const size_t FINETUNE_SMOOTHING_SIZE = 10;
static const size_t INITIAL_SMOOTHING_SIZE = 50;

template<typename T> float meanWeightedAvgBuffer(size_t start, size_t end, const T& buffer)
{
    float average = 0.0f;
    auto size = static_cast<float>(end - start);
    const float gaussSum = (size * (size + 1)) / 2.0f;

    for(size_t i = start; i < end; ++i)
        average += buffer.at(i) * static_cast<float>((i - start) + 1) / gaussSum;

    return average / std::abs(size);
}

static QVector3D normalized(const QVector3D& v)
{
    const float lengthSq = v.lengthSquared();
    if(qFuzzyIsNull(lengthSq - 1.0f))
        return v;

    if(!qIsNull(lengthSq))
        return v / std::sqrt(lengthSq);

    return {};
}

void ForceDirectedDisplacement::computeAndDamp()
{
    _next = _repulsive + _attractive;
    _nextLength = _next.length();

    // Reset for next iteration
    _repulsive = {};
    _attractive = {};

    // The following computation encouragements movements where the
    // direction is constant and discourages movements when it changes

    const float MAX_DISPLACEMENT = 10.0f;

    // Filter large displacements that can induce instability
    if(_nextLength > MAX_DISPLACEMENT)
    {
        _nextLength = MAX_DISPLACEMENT;
        _next = normalized(_next) * _nextLength;
    }

    if(_previousLength > 0.0f && _nextLength > 0.0f)
    {
        const float dotProduct = QVector3D::dotProduct(_previous / _previousLength, _next / _nextLength);

        // http://www.wolframalpha.com/input/?i=plot+0.5x%5E2%2B1.2x%2B1+from+x%3D-1to1
        const float f = (0.5f * dotProduct * dotProduct) + (1.2f * dotProduct) + 1.0f;

        if(_nextLength > (_previousLength * f))
        {
            const float r = _previousLength / _nextLength;
            _next *= (f * r);
        }
    }

    _previous = _next;
    _previousLength = _previous.length();
}

// This is a fairly arbitrary function that was arrived at through experimentation. The parameters
// shortRange and longRange affect the emphasis that the result places on local forces and global
// forces, respectively.
static float repulse(float distanceSq, float shortRange, float longRange)
{
    return ((distanceSq * distanceSq * longRange) + shortRange) /
        ((distanceSq * distanceSq * distanceSq) + 0.0001f);
}

void ForceDirectedLayout::execute(bool firstIteration, Dimensionality dimensionality)
{
    SCOPE_TIMER_MULTISAMPLES(50)

    if(firstIteration)
    {
        FastInitialLayout initialLayout(graphComponent(), positions());
        initialLayout.execute(firstIteration, dimensionality);

        for(const NodeId nodeId : nodeIds())
            _displacements->at(nodeId)._previous = {};
    }

    std::unique_ptr<AbstractBarnesHutTree> barnesHutTree;

    if(dimensionality == Dimensionality::ThreeDee)
    {
        if(_hasBeenFlattened)
        {
            // If we're in 3D mode but have previously been in 2D mode,
            // then all of the nodes will have 0 for their Z coordinate,
            // meaning there is no mathematical way for forces to be
            // computed along the Z-axis, so to mitigate this we jiggle
            // the Z coordinate up and down a small amount in order to
            // force the nodes out of the XY-plane
            float jiggle = 0.1f;
            for(auto nodeId : graphComponent().nodeIds())
            {
                auto position = positions().get(nodeId);
                position.setZ(jiggle);
                positions().set(nodeId, position);

                jiggle = -jiggle;
            }

            _hasBeenFlattened = false;
        }

        barnesHutTree = std::make_unique<BarnesHutTree3D>();
    }
    else if(dimensionality == Dimensionality::TwoDee)
    {
        _hasBeenFlattened = true;
        barnesHutTree = std::make_unique<BarnesHutTree2D>();
    }

    barnesHutTree->build(graphComponent(), positions());

    const float SHORT_RANGE = _settings->value(u"ShortRangeRepulseTerm"_s);
    const float LONG_RANGE = 0.01f + _settings->value(u"LongRangeRepulseTerm"_s);

    // Repulsive forces
    auto repulsiveResults = parallel_for(nodeIds().begin(), nodeIds().end(),
    [this, &barnesHutTree, SHORT_RANGE, LONG_RANGE](NodeId nodeId)
    {
        if(cancelled())
            return;

        _displacements->at(nodeId)._repulsive -= barnesHutTree->evaluateKernel(positions(), nodeId,
        [SHORT_RANGE, LONG_RANGE](int mass, const QVector3D& difference, float distanceSq)
        {
            return difference * (static_cast<float>(mass) * repulse(distanceSq, SHORT_RANGE, LONG_RANGE));
        });
    }, ThreadPool::NonBlocking);

    // Attractive forces
    auto attractiveResults = parallel_for(edgeIds().begin(), edgeIds().end(),
    [this](EdgeId edgeId)
    {
        if(cancelled())
            return;

        const auto& edge = graphComponent().graph().edgeById(edgeId);
        if(!edge.isLoop())
        {
            const QVector3D difference = positions().get(edge.targetId()) - positions().get(edge.sourceId());
            const float distanceSq = difference.lengthSquared();
            const float force = distanceSq * 0.001f;

            _attractiveForces->at(edgeId) = force * difference;
        }
    }, ThreadPool::NonBlocking);

    repulsiveResults.wait();
    attractiveResults.wait();

    if(cancelled())
        return;

    for(auto edgeId : edgeIds())
    {
        const auto& edge = graphComponent().graph().edgeById(edgeId);
        _displacements->at(edge.targetId())._attractive -= _attractiveForces->at(edgeId);
        _displacements->at(edge.sourceId())._attractive += _attractiveForces->at(edgeId);
    }

    parallel_for(nodeIds().begin(), nodeIds().end(),
    [this](NodeId nodeId)
    {
        _displacements->at(nodeId).computeAndDamp();
    });

    // Apply the forces
    for(auto nodeId : nodeIds())
        positions().set(nodeId, positions().get(nodeId) + _displacements->at(nodeId)._next);

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

    // Calculate force averages
    float deltaForceTotal = 0.0f;
    for(auto nodeId : nodeIds())
        deltaForceTotal += _displacements->at(nodeId)._nextLength;

    _forceMean = deltaForceTotal / static_cast<float>(nodeIds().size());

    // Calculate Standard Deviation
    float variance = 0.0f;
    for(auto nodeId : nodeIds())
    {
        const float d = _displacements->at(nodeId)._nextLength - _forceMean;
        variance += (d * d);
    }

    _forceStdDeviation = std::sqrt(variance / static_cast<float>(nodeIds().size()));
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
        const float currentSmoothedStdDev = meanWeightedAvgBuffer(
            _prevCaptureStdDevs.size() - INITIAL_SMOOTHING_SIZE,
            _prevCaptureStdDevs.size(),
            _prevCaptureStdDevs);

        const float previousSmoothedStdDev = meanWeightedAvgBuffer(
            _prevCaptureStdDevs.size() - (2 * INITIAL_SMOOTHING_SIZE),
            _prevCaptureStdDevs.size() - INITIAL_SMOOTHING_SIZE,
            _prevCaptureStdDevs);

        // Long step sample (For unstable graphs)
        if(_increasingStdDevIterationCount >= STDDEV_INCREASES_BEFORE_SWITCH_TO_OSCILLATE)
            _changeDetectionPhase = ChangeDetectionPhase::Oscillate;

        if(currentSmoothedStdDev > previousSmoothedStdDev)
            _increasingStdDevIterationCount++;
    }
}

// Set change detection phase to Finished. Clear previous data
void ForceDirectedLayout::finishChangeDetection()
{
    _changeDetectionPhase = ChangeDetectionPhase::Finished;
    _increasingStdDevIterationCount = 0;
    _unstableIterationCount = 0;
    _prevCaptureStdDevs.clear();
    _prevStdDevs.clear();
    _prevAvgForces.clear();
}

void ForceDirectedLayout::unfinish()
{
    if(_changeDetectionPhase == ChangeDetectionPhase::Finished)
        _changeDetectionPhase = ChangeDetectionPhase::Initial;
}

// Allows the layout algorithm to further calculate small layout changes until the change amount
// falls below FINETUNE_STDDEV_DELTA, where it moves the phase to Finished
void ForceDirectedLayout::fineTuneChangeDetection()
{
    if(_prevAvgForces.full() && _prevStdDevs.full())
    {
        const float prevAvgStdDev = meanWeightedAvgBuffer(
            _prevStdDevs.size() - (2 * FINETUNE_SMOOTHING_SIZE),
            _prevStdDevs.size() - FINETUNE_SMOOTHING_SIZE,
            _prevStdDevs);

        const float curAvgStdDev = meanWeightedAvgBuffer(
            _prevStdDevs.size() - FINETUNE_SMOOTHING_SIZE,
            _prevStdDevs.size(),
            _prevStdDevs);

        const float delta = (prevAvgStdDev - curAvgStdDev);
        if(delta < FINETUNE_STDDEV_DELTA && delta >= 0.0f)
            finishChangeDetection();
    }
}

// Monitors the delta of Stddev over OSCILLATE_DELTA_SAMPLE_SIZE steps OSCILLATE_RUN_COUNT times.
// If delta is less than OSCILLATE_STDDEV_DELTA_PERCENT the layout finishes, if OSCILLATE_RUN_COUNT
// is reached, returns phase to initial.
void ForceDirectedLayout::oscillateChangeDetection()
{
    if(_prevCaptureStdDevs.full())
    {
        const float averageCap = meanWeightedAvgBuffer(0, OSCILLATE_DELTA_SAMPLE_SIZE, _prevCaptureStdDevs);

        auto deltaStdDev = _prevUnstableStdDev - averageCap;
        auto percentDelta = OSCILLATE_STDDEV_DELTA_PERCENT;
        if(_prevUnstableStdDev != 0.0f)
            percentDelta = (deltaStdDev / _prevUnstableStdDev) * 100.0f;

        if(std::abs(percentDelta) < OSCILLATE_STDDEV_DELTA_PERCENT)
            finishChangeDetection();

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

ForceDirectedLayoutFactory::ForceDirectedLayoutFactory(GraphModel* graphModel) :
    LayoutFactory(graphModel), _displacements(graphModel->graph()), _attractiveForces(graphModel->graph())
{
    _layoutSettings.registerSetting("ShortRangeRepulseTerm", QObject::tr("Local"),
        QObject::tr("The repulsive force between nodes that are near each other"),
        1000.0f, 1000000000.0f, 1000000.0f, LayoutSettingScaleType::Log);

    _layoutSettings.registerSetting("LongRangeRepulseTerm", QObject::tr("Global"),
        QObject::tr("The repulsive force between nodes that are far apart"),
        0.0f, 20.0f, 10.0f);
}

std::unique_ptr<Layout> ForceDirectedLayoutFactory::create(ComponentId componentId,
    NodeLayoutPositions& nodePositions, Layout::Dimensionality dimensionalityMode)
{
    const auto* component = _graphModel->graph().componentById(componentId);
    return std::make_unique<ForceDirectedLayout>(*component,
        _displacements, _attractiveForces,
        nodePositions, dimensionalityMode, &_layoutSettings);
}
