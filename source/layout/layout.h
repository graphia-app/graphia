#ifndef LAYOUT_H
#define LAYOUT_H

#include "../graph/graph.h"
#include "../graph/grapharray.h"
#include "../maths/boundingbox.h"
#include "../maths/boundingsphere.h"

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

    bool shouldCancel()
    {
        return _atomicCancel;
    }

public:
    Layout(Iterative iterative) :
        QObject(),
        _atomicCancel(false),
        _iterative(iterative)
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
    void changed();
};

typedef NodeArray<QVector3D> NodePositions;

class NodeLayout : public Layout
{
    Q_OBJECT
protected:
    const ReadOnlyGraph& _graph;
    NodePositions& _positions;

public:
    NodeLayout(const ReadOnlyGraph& graph,
               NodePositions& positions,
               Iterative iterative = Iterative::No) :
        Layout(iterative),
        _graph(graph),
        _positions(positions)
    {}

    static BoundingBox3D boundingBox(const ReadOnlyGraph& graph, const NodePositions& positions);
    BoundingBox3D boundingBox() const;
    static BoundingBox2D boundingBoxInXY(const ReadOnlyGraph& graph, const NodePositions& positions);

    static BoundingSphere boundingSphere(const ReadOnlyGraph& graph, const NodePositions& positions);
    BoundingSphere boundingSphere() const;
    static float boundingCircleRadiusInXY(const ReadOnlyGraph& graph, const NodePositions& positions);
};

class GraphModel;

class NodeLayoutFactory
{
protected:
    std::shared_ptr<GraphModel> _graphModel;

public:
    NodeLayoutFactory(std::shared_ptr<GraphModel> graphModel) :
        _graphModel(graphModel)
    {}
    virtual ~NodeLayoutFactory() {}

    virtual std::shared_ptr<NodeLayout> create(ComponentId componentId) const = 0;
};

class LayoutThread : public QObject
{
    Q_OBJECT
protected:
    std::set<std::shared_ptr<Layout>> _layouts;
    std::mutex _mutex;
    std::thread _thread;
    bool _started;
    bool _pause;
    bool _paused;
    bool _stop;
    bool _repeating;
    uint64_t _iteration;
    std::condition_variable _waitForPause;
    std::condition_variable _waitForResume;

public:
    LayoutThread(bool repeating = false) :
        _started(false), _pause(false), _paused(true), _stop(false), _repeating(repeating), _iteration(0)
    {}

    LayoutThread(std::shared_ptr<Layout> layout, bool repeating = false) :
        _started(false), _pause(false), _paused(true), _stop(false), _repeating(repeating), _iteration(0)
    {
        addLayout(layout);
    }

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

private:
    bool iterative();
    bool allLayoutsShouldPause();
    void uncancel();
    void run();

signals:
    void executed();
};

class NodeLayoutThread : public LayoutThread
{
    Q_OBJECT
private:
    std::unique_ptr<const NodeLayoutFactory> _layoutFactory;
    std::map<ComponentId, std::shared_ptr<Layout>> _componentLayouts;

public:
    NodeLayoutThread(std::unique_ptr<const NodeLayoutFactory> layoutFactory) :
        LayoutThread(),
        _layoutFactory(std::move(layoutFactory))
    {}

    void addComponent(ComponentId componentId);
    void addAllComponents(const Graph& graph);
    void removeComponent(ComponentId componentId);
};

#endif // LAYOUT_H
