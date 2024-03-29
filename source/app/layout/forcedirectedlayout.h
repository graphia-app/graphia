/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

#ifndef FORCEDIRECTEDLAYOUT_H
#define FORCEDIRECTEDLAYOUT_H

#include "layout.h"
#include "graph/componentmanager.h"
#include "shared/utils/circularbuffer.h"

#include <QVector3D>

#include <vector>

using namespace Qt::Literals::StringLiterals;

struct ForceDirectedDisplacement
{
    QVector3D _repulsive;
    QVector3D _attractive;

    QVector3D _previous;
    QVector3D _next;
    float _previousLength = 0.0f;
    float _nextLength = 0.0f;

    void computeAndDamp();
};

using ForceDirectedDisplacements = NodeArray<ForceDirectedDisplacement>;

class ForceDirectedLayout : public Layout
{
    Q_OBJECT
private:
    static const size_t FINETUNE_DELTA_SAMPLE_SIZE = 50;
    static const size_t OSCILLATE_DELTA_SAMPLE_SIZE = 500;

    CircularBuffer<float, FINETUNE_DELTA_SAMPLE_SIZE> _prevStdDevs;
    CircularBuffer<float, FINETUNE_DELTA_SAMPLE_SIZE> _prevAvgForces;
    CircularBuffer<float, OSCILLATE_DELTA_SAMPLE_SIZE> _prevCaptureStdDevs;

    enum class ChangeDetectionPhase { Initial, FineTune, Oscillate, Finished };

    std::atomic<ChangeDetectionPhase> _changeDetectionPhase = ChangeDetectionPhase::Initial;

    ForceDirectedDisplacements* _displacements = nullptr;
    EdgeArray<QVector3D>* _attractiveForces = nullptr;

    float _forceStdDeviation = 0;
    float _forceMean = 0;
    float _prevUnstableStdDev = 0;

    size_t _unstableIterationCount = 0;
    size_t _increasingStdDevIterationCount = 0;

    bool _hasBeenFlattened = false;

    void fineTuneChangeDetection();
    void oscillateChangeDetection();
    void initialChangeDetection();
    void finishChangeDetection();

public:
    ForceDirectedLayout(const IGraphComponent& graphComponent,
                        ForceDirectedDisplacements& displacements,
                        EdgeArray<QVector3D>& attractiveForces,
                        NodeLayoutPositions& positions,
                        Layout::Dimensionality dimensionalityMode,
                        const LayoutSettings* settings) :
        Layout(graphComponent, positions, settings, Iterative::Yes,
            Dimensionality::TwoOrThreeDee, 0.4f, 4),
        _displacements(&displacements), _attractiveForces(&attractiveForces),
        _hasBeenFlattened(dimensionalityMode == Layout::Dimensionality::TwoDee)
    {}

    bool finished() const override { return _changeDetectionPhase == ChangeDetectionPhase::Finished; }
    void unfinish() override;

    void execute(bool firstIteration, Dimensionality dimensionality) override;
};

class ForceDirectedLayoutFactory : public LayoutFactory
{
private:
    ForceDirectedDisplacements _displacements;
    EdgeArray<QVector3D> _attractiveForces;

public:
    explicit ForceDirectedLayoutFactory(GraphModel* graphModel);

    QString name() const override { return u"ForceDirected"_s; }
    QString displayName() const override { return QObject::tr("Force Directed"); }
    std::unique_ptr<Layout> create(ComponentId componentId, NodeLayoutPositions& nodePositions,
        Layout::Dimensionality dimensionalityMode) override;
};

#endif // FORCEDIRECTEDLAYOUT_H
