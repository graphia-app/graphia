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

#include <QMatrix3x3>
#include <QMatrix4x4>
#include <QQuaternion>

#include <cmath>

using namespace Qt::Literals::StringLiterals;

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

    _stabilityDetectionPathLength += _previousLength;
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

    if(_startStabilityDetection.exchange(false))
    {
        for(auto nodeId : nodeIds())
        {
            auto& displacement = _displacements->at(nodeId);

            displacement._stabilityDetectionStartPosition = positions().get(nodeId);
            displacement._stabilityDetectionPathLength = 0.0f;
        }

        _stabilityDetectionIteration = 0;
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

    checkForStability();
}

// A force directed layout never truly comes to rest; the nodes continue to move
// in response to the forces around them, indefinitely. It is therefore not possible
// to detect that it has converged as such, only that it has reached the point where
// it is no longer making a difference to what the user sees. To that end the nodes'
// net movement is sampled over a window of iterations, and there are two ways in
// which it can indicate that nothing more is going to happen:
//
//  * it is small in comparison to the size of the component, meaning the layout is
//    only being refined, by an ever decreasing amount; or
//  * it is small in comparison to the distance the nodes actually travelled, meaning
//    they are oscillating in place rather than going anywhere, which some graphs do
//    forever, without the amount of movement ever diminishing
//
// Expressing both as ratios means the tests hold irrespective of the scale of the
// layout, which varies enormously with the size of the graph and the settings in
// play. Any movement of the component as a whole is discounted, as it has no
// bearing on the layout.
void ForceDirectedLayout::checkForStability()
{
    // The number of iterations over which node movement is sampled, in order
    // to determine whether the layout is still making progress or not
    const int STABILITY_WINDOW_SIZE = 250;

    // When the mean net movement of the nodes over a window is less than this
    // proportion of the radius of the component, the layout is deemed to have
    // stopped making a difference to what the user sees
    const float STABILITY_MOVEMENT_THRESHOLD = 0.04f;

    // When the net movement of the nodes over a window is less than this proportion
    // of the distance they actually travelled, they are going nowhere, i.e. the
    // layout is oscillating in place, and will continue to do so indefinitely
    const float STABILITY_OSCILLATION_THRESHOLD = 0.12f;

    if(++_stabilityDetectionIteration < STABILITY_WINDOW_SIZE)
        return;

    _stabilityDetectionIteration = 0;

    const auto numNodes = static_cast<float>(nodeIds().size());

    QVector3D startCentre;
    QVector3D centre;

    for(auto nodeId : nodeIds())
    {
        startCentre += _displacements->at(nodeId)._stabilityDetectionStartPosition;
        centre += positions().get(nodeId);
    }

    startCentre /= numNodes;
    centre /= numNodes;

    // Find the rotation that best maps the nodes' positions at the start of the
    // window onto their current positions, using Horn's method
    QMatrix3x3 c;
    c.fill(0.0f);

    for(auto nodeId : nodeIds())
    {
        const auto from = _displacements->at(nodeId)._stabilityDetectionStartPosition - startCentre;
        const auto to = positions().get(nodeId) - centre;

        for(int i = 0; i < 3; i++)
            for(int j = 0; j < 3; j++)
                c(i, j) += from[i] * to[j];
    }

    const auto x = c(0, 0);
    const auto xy = c(0, 1) + c(1, 0);
    const auto yz = c(1, 2) + c(2, 1);
    const auto zx = c(2, 0) + c(0, 2);
    const auto ax = c(1, 2) - c(2, 1);
    const auto ay = c(2, 0) - c(0, 2);
    const auto az = c(0, 1) - c(1, 0);
    const auto yypzz = c(1, 1) + c(2, 2);
    const auto yymzz = c(1, 1) - c(2, 2);

    QMatrix4x4 n
    {
        x - yypzz,  xy,         zx,         ax,
        xy,        -x + yymzz,  yz,         ay,
        zx,         yz,        -x - yymzz,  az,
        ax,         ay,         az,         x + yypzz
    };

    float shift = 0.0f;
    for(int row = 0; row < 4; row++)
    {
        float absoluteSum = 0.0f;
        for(int column = 0; column < 4; column++)
            absoluteSum += std::abs(n(row, column));

        shift = std::max(shift, absoluteSum);
    }

    for(int i = 0; i < 4; i++)
        n(i, i) += shift;

    QQuaternion rotation;
    for(int i = 0; i < 64; i++)
    {
        const auto next = QQuaternion(n * rotation.toVector4D()).normalized();

        if(next.isNull())
            break;

        rotation = next;
    }

    float movementTotal = 0.0f;
    float pathTotal = 0.0f;
    float radiusSquaredTotal = 0.0f;

    for(auto nodeId : nodeIds())
    {
        auto& displacement = _displacements->at(nodeId);
        const auto position = positions().get(nodeId);

        // Where the node would be, had it only moved with the component as a whole
        const auto from = displacement._stabilityDetectionStartPosition - startCentre;
        const auto expected = centre + rotation.rotatedVector(from);

        movementTotal += (position - expected).length();
        pathTotal += displacement._stabilityDetectionPathLength;
        radiusSquaredTotal += (position - centre).lengthSquared();

        displacement._stabilityDetectionStartPosition = position;
        displacement._stabilityDetectionPathLength = 0.0f;
    }

    const auto radius = std::sqrt(radiusSquaredTotal / numNodes);
    const auto movement = radius > 0.0f ? ((movementTotal / numNodes) / radius) : 0.0f;
    const auto progress = pathTotal > 0.0f ? (movementTotal / pathTotal) : 0.0f;

    if(movement < STABILITY_MOVEMENT_THRESHOLD || progress < STABILITY_OSCILLATION_THRESHOLD)
        _layoutIsStable = true;
}

void ForceDirectedLayout::unfinish()
{
    _layoutIsStable = false;
    _startStabilityDetection = true;
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
