#ifndef LAYOUT_H
#define LAYOUT_H

#include "../graph/graph.h"
#include "../graph/grapharray.h"
#include "../graph/graphmodel.h"
#include "nodepositions.h"

#include "../utils/performancecounter.h"

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
    const Graph& _graph;
    NodePositions& _positions;

    void setCancel(bool cancel) { _atomicCancel = cancel; }

    virtual void executeReal(uint64_t iteration) = 0;

protected:
    bool shouldCancel() const { return _atomicCancel; }

    NodePositions& positions() { return _positions; }

public:
    Layout(const Graph& graph,
           NodePositions& positions,
           Iterative iterative = Iterative::No,
           float scaling = 1.0f,
           int smoothing = 1) :
        QObject(),
        _atomicCancel(false),
        _iterative(iterative),
        _scaling(scaling),
        _smoothing(smoothing),
        _graph(graph),
        _positions(positions)
    {}

    float scaling() const { return _scaling; }
    int smoothing() const { return _smoothing; }

    const Graph& graph() const { return _graph; }
    const std::vector<NodeId>& nodeIds() const { return _graph.nodeIds(); }
    const std::vector<EdgeId>& edgeIds() const { return _graph.edgeIds(); }

    void execute(int iteration) { executeReal(iteration); }

    virtual void cancel() { setCancel(true); }
    virtual void uncancel() { setCancel(false); }

    // Indicates that the algorithm is doing no useful work
    virtual bool shouldPause() { return false; }

    virtual bool iterative() const { return _iterative == Iterative::Yes; }

signals:
    void progress(int percentage);
};

class GraphModel;

class LayoutFactory
{
protected:
    std::shared_ptr<GraphModel> _graphModel;

public:
    LayoutFactory(std::shared_ptr<GraphModel> graphModel) :
        _graphModel(graphModel)
    {}
    virtual ~LayoutFactory() {}

    virtual std::shared_ptr<Layout> create(ComponentId componentId, NodePositions& results) const = 0;
};

class LayoutThread : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool paused READ paused NOTIFY pausedChanged)

private:
    GraphModel* _graphModel;
    std::set<std::shared_ptr<Layout>> _layouts;
    std::mutex _mutex;
    std::thread _thread;
    bool _started = false;
    bool _pause = false;
    bool _paused = true;
    bool _stop = false;
    bool _repeating = false;
    uint64_t _iteration = 0;
    std::condition_variable _waitForPause;
    std::condition_variable _waitForResume;

    std::unique_ptr<const LayoutFactory> _layoutFactory;
    std::map<ComponentId, std::shared_ptr<Layout>> _componentLayouts;

    NodePositions _intermediatePositions;

    PerformanceCounter _performanceCounter;

public:
    LayoutThread(GraphModel& graphModel,
                 std::unique_ptr<const LayoutFactory> layoutFactory,
                 bool repeating = false);

    virtual ~LayoutThread()
    {
        stop();

        if(_thread.joinable())
            _thread.join();
    }

    void addLayout(std::shared_ptr<Layout> layout);
    void removeLayout(std::shared_ptr<Layout> layout);

    void pause();
    void pauseAndWait();
    bool paused();
    void resume();

    void start();
    void stop();

    void addAllComponents();

private:
    bool iterative();
    bool allLayoutsShouldPause();
    void uncancel();
    void run();

    void addComponent(ComponentId componentId);
    void removeComponent(ComponentId componentId);

private slots:
    void onComponentAdded(const Graph*, ComponentId componentId, bool);
    void onComponentWillBeRemoved(const Graph*, ComponentId componentId, bool);
    void onComponentSplit(const Graph*, const ComponentSplitSet& componentSplitSet);
    void onComponentsWillMerge(const Graph*, const ComponentMergeSet& componentMergeSet);

signals:
    void executed();
    void pausedChanged();
};

#endif // LAYOUT_H
