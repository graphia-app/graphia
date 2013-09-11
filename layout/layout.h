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

protected:
    bool _iterative;

    bool shouldCancel()
    {
        return atomicCancel;
    }

public:
    Layout(bool _iterative) :
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

    virtual bool iterative() { return _iterative; }

signals:
    void progress(int percentage);
    void complete();
};

class NodeLayout : public Layout
{
    Q_OBJECT
protected:
    const ReadOnlyGraph* _graph;
    NodeArray<QVector3D>* positions;

public:
    NodeLayout(const ReadOnlyGraph& graph, NodeArray<QVector3D>& positions, bool iterative = false) :
        Layout(iterative),
        _graph(&graph),
        positions(&positions)
    {}

    const ReadOnlyGraph& graph() { return *_graph; }

    static BoundingBox3D boundingBox(const ReadOnlyGraph& graph, const NodeArray<QVector3D>& positions)
    {
        const QVector3D& firstNodePosition = positions[graph.nodeIds()[0]];
        BoundingBox3D boundingBox(firstNodePosition, firstNodePosition);

        for(NodeId nodeId : graph.nodeIds())
            boundingBox.expandToInclude(positions[nodeId]);

        return boundingBox;
    }

    BoundingBox3D boundingBox()
    {
        return NodeLayout::boundingBox(*_graph, *this->positions);
    }

    struct BoundingSphere
    {
        QVector3D centre;
        float radius;
    };

    static BoundingSphere boundingSphere(const ReadOnlyGraph& graph, const NodeArray<QVector3D>& positions)
    {
        BoundingBox3D boundingBox = NodeLayout::boundingBox(graph, positions);
        BoundingSphere boundingSphere =
        {
            boundingBox.centre(),
            std::max(std::max(boundingBox.xLength(), boundingBox.yLength()), boundingBox.zLength()) * 0.5f * std::sqrt(3.0f)
        };

        return boundingSphere;
    }

    BoundingSphere boundingSphere()
    {
        return NodeLayout::boundingSphere(*_graph, *this->positions);
    }

    static float boundingCircleRadiusInXY(const ReadOnlyGraph& graph, const NodeArray<QVector3D>& positions)
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
    ComponentArray<QVector2D>* positions;
    const NodeArray<QVector3D>* nodePositions;

public:
    ComponentLayout(const Graph& graph, ComponentArray<QVector2D>& positions,
                    const NodeArray<QVector3D>& nodePositions, bool iterative = false) :
        Layout(iterative),
        _graph(&graph),
        positions(&positions),
        nodePositions(&nodePositions)
    {}

    const ReadOnlyGraph& graph() { return *_graph; }

    BoundingBox2D boundingBox()
    {
        BoundingBox2D _boundingBox;

        for(ComponentId componentId : *_graph->componentIds())
        {
            const ReadOnlyGraph& component = *_graph->componentById(componentId);
            float componentRadius = NodeLayout::boundingCircleRadiusInXY(component, *nodePositions);
            QVector2D componentPosition = (*positions)[componentId];
            BoundingBox2D componentBoundingBox(
                        QVector2D(componentPosition.x() - componentRadius, componentPosition.y() - componentRadius),
                        QVector2D(componentPosition.x() + componentRadius, componentPosition.y() + componentRadius));

            _boundingBox.expandToInclude(componentBoundingBox);
        }

        return _boundingBox;
    }
};

class GraphModel;

class LayoutFactory
{
protected:
    GraphModel* _graphModel;

public:
    LayoutFactory(GraphModel* _graphModel) :
        _graphModel(_graphModel)
    {}
    virtual ~LayoutFactory() {}

    const GraphModel& graphModel() const { return *_graphModel; }

    virtual NodeLayout* create(ComponentId componentId) const = 0;
};

class LayoutThread : public QThread
{
    Q_OBJECT
private:
    const LayoutFactory* layoutFactory;
    QMap<ComponentId, Layout*> layouts;
    QMutex mutex;
    bool _pause;
    bool _isPaused;
    bool _stop;
    QWaitCondition waitForPause;
    QWaitCondition waitForResume;

public:
    LayoutThread(const LayoutFactory* layoutFactory) :
        layoutFactory(layoutFactory),
        _pause(false), _isPaused(false), _stop(false)
    {}

    virtual ~LayoutThread()
    {
        stop();
        wait();
        delete layoutFactory;
    }

    void add(ComponentId componentId)
    {
        QMutexLocker locker(&mutex);

        if(!layouts.contains(componentId))
        {
            Layout* layout = layoutFactory->create(componentId);

            // Take ownership of the algorithm
            layout->moveToThread(this);
            layouts.insert(componentId, layout);

            start();
        }
    }

    void remove(ComponentId componentId)
    {
        bool resumeAfterRemoval = false;

        if(isPaused())
        {
            pauseAndWait();
            resumeAfterRemoval = true;
        }

        if(layouts.contains(componentId))
        {
            Layout* layout = layouts[componentId];
            layouts.remove(componentId);
            delete layout;
        }

        if(resumeAfterRemoval)
            resume();
    }

    void pause()
    {
        QMutexLocker locker(&mutex);
        _pause = true;

        for(Layout* layout : layouts.values())
            layout->cancel();
    }

    void pauseAndWait()
    {
        QMutexLocker locker(&mutex);
        _pause = true;

        for(Layout* layout : layouts.values())
            layout->cancel();

        waitForPause.wait(&mutex);
    }

    bool isPaused()
    {
        QMutexLocker locker(&mutex);
        return _isPaused;
    }

    void resume()
    {
        {
            QMutexLocker locker(&mutex);
            _pause = false;
            _isPaused = false;
        }

        waitForResume.wakeAll();
    }

    void stop()
    {
        {
            QMutexLocker locker(&mutex);
            _stop = true;
            _pause = false;

            for(Layout* layout : layouts.values())
                layout->cancel();
        }

        waitForResume.wakeAll();
    }

private:
    bool workRemaining()
    {
        for(Layout* layout : layouts.values())
        {
            if(layout->iterative())
                return true;
        }

        return false;
    }

    bool allLayoutAlgorithmsShouldPause()
    {
        for(Layout* layout : layouts.values())
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
            for(Layout* layout : layouts.values())
            {
                if(layout->shouldPause())
                    continue;

                layout->execute();
            }

            {
                QMutexLocker locker(&mutex);

                if(_pause || allLayoutAlgorithmsShouldPause())
                {
                    _isPaused = true;
                    waitForPause.wakeAll();
                    waitForResume.wait(&mutex);
                }

                if(_stop)
                    break;
            }
        }
        while(workRemaining());

        for(Layout* layout : layouts.values())
            delete layout;
    }
};

#endif // LAYOUT_H
