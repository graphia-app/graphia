#ifndef FORCEDIRECTEDLAYOUT_H
#define FORCEDIRECTEDLAYOUT_H

#include "layout.h"
#include "../graph/graphmodel.h"
#include "../graph/componentmanager.h"
#include "../utils/circularbuffer.h"

#include <QVector3D>

#include <vector>

class ForceDirectedLayout : public Layout
{
    Q_OBJECT
private:
    const float MINIMUM_STDDEV_THRESHOLD = 0.008f;
    const float FINETUNE_STDDEV_DELTA = 0.000005f;
    const float OSCILLATE_STDDEV_DELTA_PERCENT = 1.0f;
    const float MAXIMUM_AVG_FORCE_FOR_STOP = 1.0f;
    static const int OSCILLATE_DELTA_SAMPLE_SIZE = 500;
    static const int OSCILLATE_RUN_COUNT = 5;
    static const int STDDEV_INCREASES_BEFORE_SWITCH_TO_OSCILLATE = 500;
    static const int FINETUNE_DELTA_SAMPLE_SIZE = 50;
    static const int FINETUNE_SMOOTHING_SIZE = 10;
    static const int INITIAL_SMOOTHING_SIZE = 50;

    CircularBuffer<float, FINETUNE_DELTA_SAMPLE_SIZE> _prevStdDevs;
    CircularBuffer<float, FINETUNE_DELTA_SAMPLE_SIZE> _prevAvgForces;
    CircularBuffer<float, OSCILLATE_DELTA_SAMPLE_SIZE> _prevCaptureStdDevs;

    enum class ChangeDetectionPhase { Initial, FineTune, Oscillate, Finished };

    ChangeDetectionPhase _changeDetectionPhase = ChangeDetectionPhase::Initial;

    std::vector<QVector3D> _prevDisplacements;
    std::vector<QVector3D> _displacements;

    float _forceStdDeviation = 0;
    float _forceMean = 0;
    float _prevUnstableStdDev = 0;

    int _unstableIterationCount = 0;
    int _increasingStdDevIterationCount = 0;

    void fineTuneChangeDetection();
    void oscillateChangeDetection();
    void initialChangeDetection();

public:
    ForceDirectedLayout(const Graph& graph,
                        NodePositions& positions,
                        const LayoutSettings* settings) :
        Layout(graph, positions, settings, Iterative::Yes, 0.4f, 4),
        _prevDisplacements(graph.numNodes()),
        _displacements(graph.numNodes())
    {}

    bool finished() const { return _changeDetectionPhase == ChangeDetectionPhase::Finished; }
    void unfinish() { _changeDetectionPhase = ChangeDetectionPhase::Initial; }

    void executeReal(bool firstIteration);
};

class ForceDirectedLayoutFactory : public LayoutFactory
{
public:
    explicit ForceDirectedLayoutFactory(std::shared_ptr<GraphModel> graphModel) :
        LayoutFactory(graphModel)
    {
        _layoutSettings.registerSetting("RepulsiveForce",  QObject::tr("Repulsive Force"),  1.0f, 100.0f, 1.0f);
        _layoutSettings.registerSetting("AttractiveForce", QObject::tr("Attractive Force"), 1.0f, 100.0f, 1.0f);
        _layoutSettings.finishRegistration();
    }

    std::shared_ptr<Layout> create(ComponentId componentId, NodePositions& nodePositions) const
    {
        auto component = this->_graphModel->graph().componentById(componentId);
        return std::make_shared<ForceDirectedLayout>(*component, nodePositions, &_layoutSettings);
    }
};

#endif // FORCEDIRECTEDLAYOUT_H
