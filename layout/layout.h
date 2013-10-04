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

class Layout : public QObject
{
    Q_OBJECT
private:
    std::atomic<bool> atomicCancel;
    void setCancel(bool cancel)
    {
        atomicCancel = cancel;
    }

    virtual void executeReal() = 0;

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
        return atomicCancel;
    }

public:
    Layout(Iterative _iterative) :
        atomicCancel(false),
        _iterative(_iterative)
    {}

    void execute()
    {
        setCancel(false);
        executeReal();
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
    NodePositions* positions;

public:
    NodeLayout(const ReadOnlyGraph& graph, NodePositions& positions, Iterative iterative = Iterative::No) :
        Layout(iterative),
        _graph(&graph),
        positions(&positions)
    {}

    const ReadOnlyGraph& graph() { return *_graph; }

    static BoundingBox3D boundingBox(const ReadOnlyGraph& graph, const NodePositions& positions);
    BoundingBox3D boundingBox() const;
    static BoundingBox2D boundingBoxInXY(const ReadOnlyGraph& graph, const NodePositions& positions);

    struct BoundingSphere
    {
        QVector3D centre;
        float radius;
    };

    static BoundingSphere boundingSphere(const ReadOnlyGraph& graph, const NodePositions& positions);
    BoundingSphere boundingSphere() const;
    static float boundingCircleRadiusInXY(const ReadOnlyGraph& graph, const NodePositions& positions);
};

class ComponentLayout : public Layout
{
    Q_OBJECT
protected:
    const Graph* _graph;
    ComponentPositions* componentPositions;
    const NodePositions* nodePositions;

public:
    ComponentLayout(const Graph& graph, ComponentPositions& componentPositions,
                    const NodePositions& nodePositions, Iterative iterative = Iterative::No) :
        Layout(iterative),
        _graph(&graph),
        componentPositions(&componentPositions),
        nodePositions(&nodePositions)
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
    NodeLayoutFactory(GraphModel* _graphModel) :
        _graphModel(_graphModel)
    {}
    virtual ~NodeLayoutFactory() {}

    const GraphModel& graphModel() const { return *_graphModel; }

    virtual NodeLayout* create(ComponentId componentId) const = 0;
};

class LayoutThread : public QThread
{
    Q_OBJECT
protected:
    QSet<Layout*> layouts;
    QMutex mutex;
    bool _pause;
    bool _paused;
    bool _stop;
    bool repeating;
    QWaitCondition waitForPause;
    QWaitCondition waitForResume;

public:
    LayoutThread(bool repeating = false) :
        _pause(false), _paused(false), _stop(false), repeating(repeating)
    {}

    LayoutThread(Layout* layout, bool _repeating = false) :
        _pause(false), _paused(false), _stop(false), repeating(_repeating)
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
