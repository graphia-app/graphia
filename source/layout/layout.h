#ifndef LAYOUT_H
#define LAYOUT_H

#include "../graph/graph.h"
#include "../graph/grapharray.h"
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
private:
    std::atomic<bool> _atomicCancel;
    void setCancel(bool cancel)
    {
        _atomicCancel = cancel;
    }

    virtual void executeReal(uint64_t iteration) = 0;

public:
    enum class Iterative
    {
        Yes,
        No
    };

protected:
    Iterative _iterative;
    const Graph& _graph;
    NodePositions& _positions;

    bool shouldCancel()
    {
        return _atomicCancel;
    }

public:
    Layout(const Graph& graph,
           NodePositions& positions,
           Iterative iterative = Iterative::No) :
        QObject(),
        _atomicCancel(false),
        _iterative(iterative),
        _graph(graph),
        _positions(positions)
    {}

    void execute(int iteration)
    {
        executeReal(iteration);
    }

    virtual void cancel()
    {
        setCancel(true);
    }

    virtual void uncancel()
    {
        setCancel(false);
    }

    // Indicates that the algorithm is doing no useful work
    virtual bool shouldPause() { return false; }

    virtual bool iterative() { return _iterative == Iterative::Yes; }

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

    virtual std::shared_ptr<Layout> create(ComponentId componentId) const = 0;
};

class LayoutThread : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool paused READ paused NOTIFY pausedChanged)

private:
    const Graph* _graph;
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

    PerformanceCounter _performanceCounter;

public:
    LayoutThread(const Graph& graph,
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
