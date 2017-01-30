#ifndef LAYOUT_H
#define LAYOUT_H

#include "graph/graph.h"
#include "shared/graph/grapharray.h"
#include "graph/graphmodel.h"
#include "graph/componentmanager.h"
#include "nodepositions.h"

#include "shared/utils/performancecounter.h"
#include "layoutsettings.h"

#include <QVector2D>
#include <QVector3D>
#include <QObject>

#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include <algorithm>
#include <limits>
#include <cstdint>
#include <set>
#include <map>

class Layout : public QObject
{
    Q_OBJECT

public:
    enum class Iterative
    {
        Yes,
        No
    };

private:
    std::atomic<bool> _atomicCancel;
    Iterative _iterative;
    float _scaling;
    int _smoothing;
    const GraphComponent* _graphComponent;
    NodePositions* _positions;

    void setCancel(bool cancel) { _atomicCancel = cancel; }

    virtual void executeReal(bool firstIteration) = 0;

protected:
    bool shouldCancel() const { return _atomicCancel; }
    const LayoutSettings* _settings;

    NodePositions& positions() { return *_positions; }

public:
    Layout(const GraphComponent& graphComponent,
           NodePositions& positions,
           const LayoutSettings* settings = nullptr,
           Iterative iterative = Iterative::No,
           float scaling = 1.0f,
           int smoothing = 1) :
        QObject(),
        _atomicCancel(false),
        _iterative(iterative),
        _scaling(scaling),
        _smoothing(smoothing),
        _graphComponent(&graphComponent),
        _positions(&positions),
        _settings(settings)
    {}

    float scaling() const { return _scaling; }
    int smoothing() const { return _smoothing; }

    const GraphComponent& graphComponent() const { return *_graphComponent; }
    const std::vector<NodeId>& nodeIds() const { return _graphComponent->nodeIds(); }
    const std::vector<EdgeId>& edgeIds() const { return _graphComponent->edgeIds(); }

    void execute(bool firstIteration) { executeReal(firstIteration); }

    virtual void cancel() { setCancel(true); }
    virtual void uncancel() { setCancel(false); }

    // Indicates that the algorithm is doing no useful work
    virtual bool finished() const { return false; }

    // Resets the state of the algorithm such that finished() no longer returns true
    virtual void unfinish() { Q_ASSERT(!"unfinish not implemented"); }

    virtual bool iterative() const { return _iterative == Iterative::Yes; }

signals:
    void progress(int percentage);
};

class GraphModel;

class LayoutFactory
{
protected:
    std::shared_ptr<GraphModel> _graphModel;
    LayoutSettings _layoutSettings;

public:
    explicit LayoutFactory(const std::shared_ptr<GraphModel>& graphModel) :
        _graphModel(graphModel)
    {}

    virtual ~LayoutFactory() {}

    LayoutSettings& settings()
    {
        return _layoutSettings;
    }

    virtual std::shared_ptr<Layout> create(ComponentId componentId, NodePositions& results) const = 0;
};

class LayoutThread : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool paused READ paused NOTIFY pausedChanged)

private:
    GraphModel* _graphModel;
    std::mutex _mutex;
    std::thread _thread;
    bool _started = false;
    bool _pause = false;
    bool _paused = true;
    bool _stop = false;
    bool _repeating = false;
    std::condition_variable _waitForPause;
    std::condition_variable _waitForResume;

    std::unique_ptr<LayoutFactory> _layoutFactory;
    std::map<ComponentId, std::shared_ptr<Layout>> _layouts;
    ComponentArray<bool> _executedAtLeastOnce;

    NodePositions _intermediatePositions;

    PerformanceCounter _performanceCounter;

    bool _layoutPotentiallyRequired = false;

    int _debug = 0;

public:
    LayoutThread(GraphModel& graphModel,
                 std::unique_ptr<LayoutFactory>&& layoutFactory,
                 bool repeating = false);

    virtual ~LayoutThread()
    {
        stop();

        if(_thread.joinable())
            _thread.join();
    }

    void pause();
    void pauseAndWait();
    bool paused();
    void resume();

    void start();
    void stop();

    bool finished();

    void addAllComponents();

    std::vector<LayoutSetting>& settingsVector();

private:
    bool iterative();
    bool allLayoutsFinished();
    bool workToDo();
    void uncancel();
    void run();

    void addComponent(ComponentId componentId);
    void removeComponent(ComponentId componentId);

private slots:
    void onComponentSplit(const Graph*, const ComponentSplitSet& componentSplitSet);
    void onComponentAdded(const Graph*, ComponentId componentId, bool);
    void onComponentWillBeRemoved(const Graph*, ComponentId componentId, bool);

signals:
    void executed();
    void pausedChanged();
    void settingChanged();
};

#endif // LAYOUT_H
