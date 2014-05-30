#ifndef LAYOUT_H
#define LAYOUT_H

#include "../graph/graph.h"
#include "../graph/grapharray.h"
#include "../maths/boundingbox.h"

#include <QList>
#include <QMap>
#include <QVector2D>
#include <QVector3D>
#include <QObject>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QWaitCondition>

#include <algorithm>
#include <limits>
#include <atomic>
#include <cstdint>

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
        _atomicCancel(false),
        _iterative(iterative)
    {}

    void execute(int iteration)
    {
        setCancel(false);
        executeReal(iteration);
    }

    virtual void cancel()
    {
        setCancel(true);
    }

    // Indicates that the algorithm is doing no useful work
    virtual bool shouldPause() { return false; }

    virtual bool iterative() { return _iterative == Iterative::Yes; }

signals:
    void progress(int percentage);
    void changed();
};

typedef NodeArray<QVector3D> NodePositions;
typedef ComponentArray<QVector2D> ComponentPositions;

class NodeLayout : public Layout
{
    Q_OBJECT
protected:
    const ReadOnlyGraph* _graph;
    NodePositions* _positions;

public:
    NodeLayout(const ReadOnlyGraph& graph, NodePositions& positions, Iterative iterative = Iterative::No) :
        Layout(iterative),
        _graph(&graph),
        _positions(&positions)
    {}

    const ReadOnlyGraph& graph() { return *_graph; }

    static BoundingBox3D boundingBox(const ReadOnlyGraph& graph, const NodePositions& _positions);
    BoundingBox3D boundingBox() const;
    static BoundingBox2D boundingBoxInXY(const ReadOnlyGraph& graph, const NodePositions& _positions);

    //FIXME we have boundingsphere.h too!
    struct BoundingSphere
    {
        QVector3D _centre;
        float _radius;
    };

    static BoundingSphere boundingSphere(const ReadOnlyGraph& graph, const NodePositions& _positions);
    BoundingSphere boundingSphere() const;
    static float boundingCircleRadiusInXY(const ReadOnlyGraph& graph, const NodePositions& _positions);
};

class ComponentLayout : public Layout
{
    Q_OBJECT
protected:
    const Graph* _graph;
    ComponentPositions* _componentPositions;
    const NodePositions* _nodePositions;

public:
    ComponentLayout(const Graph& graph, ComponentPositions& componentPositions,
                    const NodePositions& nodePositions, Iterative iterative = Iterative::No) :
        Layout(iterative),
        _graph(&graph),
        _componentPositions(&componentPositions),
        _nodePositions(&nodePositions)
    {}

    const Graph& graph() { return *_graph; }

    BoundingBox2D boundingBox() const;
    float radiusOfComponent(ComponentId componentId) const;
    BoundingBox2D boundingBoxOfComponent(ComponentId componentId) const;
};

class GraphModel;

class NodeLayoutFactory
{
protected:
    GraphModel* _graphModel;

public:
    NodeLayoutFactory(GraphModel* graphModel) :
        _graphModel(graphModel)
    {}
    virtual ~NodeLayoutFactory() {}

    const GraphModel& graphModel() const { return *_graphModel; }

    virtual NodeLayout* create(ComponentId componentId) const = 0;
};

class LayoutThread : public QThread
{
    Q_OBJECT
protected:
    QSet<Layout*> _layouts;
    QMutex _mutex;
    bool _pause;
    bool _paused;
    bool _stop;
    bool _repeating;
    uint64_t _iteration;
    QWaitCondition _waitForPause;
    QWaitCondition _waitForResume;

public:
    LayoutThread(bool repeating = false) :
        _pause(false), _paused(false), _stop(false), _repeating(repeating), _iteration(0)
    {}

    LayoutThread(Layout* layout, bool repeating = false) :
        _pause(false), _paused(false), _stop(false), _repeating(repeating), _iteration(0)
    {
        addLayout(layout);
    }

    virtual ~LayoutThread()
    {
        stop();
        wait();
    }

    void addLayout(Layout* layout);
    void removeLayout(Layout* layout);

    void pause();
    void pauseAndWait();
    bool paused();
    void resume();

    void execute();
    void stop();

private:
    bool iterative();
    bool allLayoutsShouldPause();
    void run() Q_DECL_OVERRIDE;

signals:
    void executed();
};

class NodeLayoutThread : public LayoutThread
{
    Q_OBJECT
private:
    const NodeLayoutFactory* layoutFactory;
    QMap<ComponentId, Layout*> componentLayouts;

public:
    NodeLayoutThread(const NodeLayoutFactory* layoutFactory) :
        LayoutThread(),
        layoutFactory(layoutFactory)
    {}

    virtual ~NodeLayoutThread()
    {
        delete layoutFactory;
    }

    void addComponent(ComponentId componentId);
    void addAllComponents(const Graph& graph);
    void removeComponent(ComponentId componentId);
};

#endif // LAYOUT_H
