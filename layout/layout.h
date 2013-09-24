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

    static BoundingBox3D boundingBox(const ReadOnlyGraph& graph, const NodePositions& positions)
    {
        const QVector3D& firstNodePosition = positions[graph.nodeIds()[0]];
        BoundingBox3D boundingBox(firstNodePosition, firstNodePosition);

        for(NodeId nodeId : graph.nodeIds())
            boundingBox.expandToInclude(positions[nodeId]);

        return boundingBox;
    }

    BoundingBox3D boundingBox() const
    {
        return NodeLayout::boundingBox(*_graph, *this->positions);
    }

    static BoundingBox2D boundingBoxInXY(const ReadOnlyGraph& graph, const NodePositions& positions)
    {
        BoundingBox3D boundingBox = NodeLayout::boundingBox(graph, positions);

        return BoundingBox2D(
                    QVector2D(boundingBox.min().x(), boundingBox.min().y()),
                    QVector2D(boundingBox.max().x(), boundingBox.max().y()));
    }

    struct BoundingSphere
    {
        QVector3D centre;
        float radius;
    };

    static BoundingSphere boundingSphere(const ReadOnlyGraph& graph, const NodePositions& positions)
    {
        BoundingBox3D boundingBox = NodeLayout::boundingBox(graph, positions);
        BoundingSphere boundingSphere =
        {
            boundingBox.centre(),
            std::max(std::max(boundingBox.xLength(), boundingBox.yLength()), boundingBox.zLength()) * 0.5f * std::sqrt(3.0f)
        };

        return boundingSphere;
    }

    BoundingSphere boundingSphere() const
    {
        return NodeLayout::boundingSphere(*_graph, *this->positions);
    }

    static float boundingCircleRadiusInXY(const ReadOnlyGraph& graph, const NodePositions& positions)
    {
        BoundingBox3D boundingBox = NodeLayout::boundingBox(graph, positions);
        return std::max(boundingBox.xLength(), boundingBox.yLength()) * 0.5f * std::sqrt(2.0f);
    }
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

    BoundingBox2D boundingBox() const
    {
        BoundingBox2D _boundingBox;

        for(ComponentId componentId : *_graph->componentIds())
        {
            const ReadOnlyGraph& component = *_graph->componentById(componentId);
            float componentRadius = NodeLayout::boundingCircleRadiusInXY(component, *nodePositions);
            QVector2D componentPosition = (*componentPositions)[componentId];
            BoundingBox2D componentBoundingBox(
                        QVector2D(componentPosition.x() - componentRadius, componentPosition.y() - componentRadius),
                        QVector2D(componentPosition.x() + componentRadius, componentPosition.y() + componentRadius));

            _boundingBox.expandToInclude(componentBoundingBox);
        }

        return _boundingBox;
    }

    float radiusOfComponent(ComponentId componentId) const
    {
        const ReadOnlyGraph& component = *_graph->componentById(componentId);
        return NodeLayout::boundingCircleRadiusInXY(component, *nodePositions);
    }

    BoundingBox2D boundingBoxOfComponent(ComponentId componentId) const
    {
        const ReadOnlyGraph& component = *_graph->componentById(componentId);
        return NodeLayout::boundingBoxInXY(component, *nodePositions);
    }
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
    bool _isPaused;
    bool _stop;
    bool repeating;
    QWaitCondition waitForPause;
    QWaitCondition waitForResume;

public:
    LayoutThread(bool repeating = false) :
        _pause(false), _isPaused(false), _stop(false), repeating(repeating)
    {}

    LayoutThread(Layout* layout, bool _repeating = false) :
        _pause(false), _isPaused(false), _stop(false), repeating(_repeating)
    {
        addLayout(layout);
    }

    virtual ~LayoutThread()
    {
        stop();
        wait();
    }

    void addLayout(Layout* layout)
    {
        QMutexLocker locker(&mutex);

        // Take ownership of the algorithm
        layout->moveToThread(this);
        layouts.insert(layout);

        start();
    }

    void removeLayout(Layout* layout)
    {
        QMutexLocker locker(&mutex);

        layouts.remove(layout);
        delete layout;
    }

    void pause()
    {
        QMutexLocker locker(&mutex);
        _pause = true;

        for(Layout* layout : layouts)
            layout->cancel();
    }

    void pauseAndWait()
    {
        QMutexLocker locker(&mutex);
        _pause = true;

        for(Layout* layout : layouts)
            layout->cancel();

        waitForPause.wait(&mutex);
    }

    bool paused()
    {
        QMutexLocker locker(&mutex);
        return _isPaused;
    }

    void resume()
    {
        {
            QMutexLocker locker(&mutex);
            if(!_isPaused)
                return;

            _pause = false;
            _isPaused = false;
        }

        waitForResume.wakeAll();
    }

    void execute()
    {
        resume();
    }

    void stop()
    {
        {
            QMutexLocker locker(&mutex);
            _stop = true;
            _pause = false;

            for(Layout* layout : layouts)
                layout->cancel();
        }

        waitForResume.wakeAll();
    }

private:
    bool iterative()
    {
        for(Layout* layout : layouts)
        {
            if(layout->iterative())
                return true;
        }

        return false;
    }

    bool allLayoutsShouldPause()
    {
        for(Layout* layout : layouts)
        {
            if(!layout->shouldPause())
                return false;
        }

        return true;
    }

    void run() Q_DECL_OVERRIDE
    {
        do
        {
            for(Layout* layout : layouts)
            {
                if(layout->shouldPause())
                    continue;

                layout->execute();
            }

            emit executed();

            {
                QMutexLocker locker(&mutex);

                if(_pause || allLayoutsShouldPause() || (!iterative() && repeating))
                {
                    _isPaused = true;
                    waitForPause.wakeAll();
                    waitForResume.wait(&mutex);
                }

                if(_stop)
                    break;
            }
        }
        while(iterative() || repeating);

        mutex.lock();
        for(Layout* layout : layouts)
            delete layout;

        layouts.clear();
        mutex.unlock();
    }

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

    void addComponent(ComponentId componentId)
    {
        if(!componentLayouts.contains(componentId))
        {
            Layout* layout = layoutFactory->create(componentId);

            addLayout(layout);
            componentLayouts.insert(componentId, layout);

            start();
        }
    }

    void addAllComponents(const Graph& graph)
    {
        for(ComponentId componentId : *graph.componentIds())
            addComponent(componentId);
    }

    void removeComponent(ComponentId componentId)
    {
        bool resumeAfterRemoval = false;

        if(!paused())
        {
            pauseAndWait();
            resumeAfterRemoval = true;
        }

        if(componentLayouts.contains(componentId))
        {
            componentLayouts.remove(componentId);
            Layout* layout = componentLayouts[componentId];
            removeLayout(layout);
        }

        if(resumeAfterRemoval)
            resume();
    }
};

#endif // LAYOUT_H
