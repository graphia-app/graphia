#include "layout.h"

BoundingBox3D NodeLayout::boundingBox(const ReadOnlyGraph &graph, const NodePositions &positions)
{
    const QVector3D& firstNodePosition = positions[graph.nodeIds()[0]];
    BoundingBox3D boundingBox(firstNodePosition, firstNodePosition);

    for(NodeId nodeId : graph.nodeIds())
        boundingBox.expandToInclude(positions[nodeId]);

    return boundingBox;
}


BoundingBox3D NodeLayout::boundingBox() const
{
    return NodeLayout::boundingBox(*_graph, *this->positions);
}

BoundingBox2D NodeLayout::boundingBoxInXY(const ReadOnlyGraph &graph, const NodePositions &positions)
{
    BoundingBox3D boundingBox = NodeLayout::boundingBox(graph, positions);

    return BoundingBox2D(
                QVector2D(boundingBox.min().x(), boundingBox.min().y()),
                QVector2D(boundingBox.max().x(), boundingBox.max().y()));
}

NodeLayout::BoundingSphere NodeLayout::boundingSphere() const
{
    return NodeLayout::boundingSphere(*_graph, *this->positions);
}

float NodeLayout::boundingCircleRadiusInXY(const ReadOnlyGraph &graph, const NodePositions &positions)
{
    BoundingBox3D boundingBox = NodeLayout::boundingBox(graph, positions);
    return std::max(boundingBox.xLength(), boundingBox.yLength()) * 0.5f * std::sqrt(2.0f);
}

NodeLayout::BoundingSphere NodeLayout::boundingSphere(const ReadOnlyGraph &graph, const NodePositions &positions)
{
    BoundingBox3D boundingBox = NodeLayout::boundingBox(graph, positions);
    BoundingSphere boundingSphere =
    {
        boundingBox.centre(),
        std::max(std::max(boundingBox.xLength(), boundingBox.yLength()), boundingBox.zLength()) * 0.5f * std::sqrt(3.0f)
    };

    return boundingSphere;
}

BoundingBox2D ComponentLayout::boundingBox() const
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

float ComponentLayout::radiusOfComponent(ComponentId componentId) const
{
    const ReadOnlyGraph& component = *_graph->componentById(componentId);
    return NodeLayout::boundingCircleRadiusInXY(component, *nodePositions);
}

BoundingBox2D ComponentLayout::boundingBoxOfComponent(ComponentId componentId) const
{
    const ReadOnlyGraph& component = *_graph->componentById(componentId);
    return NodeLayout::boundingBoxInXY(component, *nodePositions);
}


void LayoutThread::addLayout(Layout *layout)
{
    QMutexLocker locker(&mutex);

    // Take ownership of the algorithm
    layout->moveToThread(this);
    layouts.insert(layout);
}

void LayoutThread::removeLayout(Layout *layout)
{
    QMutexLocker locker(&mutex);

    layouts.remove(layout);
    delete layout;
}

void LayoutThread::pause()
{
    QMutexLocker locker(&mutex);
    if(_paused)
        return;

    _pause = true;

    for(Layout* layout : layouts)
        layout->cancel();
}

void LayoutThread::pauseAndWait()
{
    QMutexLocker locker(&mutex);
    if(_paused)
        return;

    _pause = true;

    for(Layout* layout : layouts)
        layout->cancel();

    waitForPause.wait(&mutex);
}

bool LayoutThread::paused()
{
    QMutexLocker locker(&mutex);
    return _paused;
}

void LayoutThread::resume()
{
    QMutexLocker locker(&mutex);
    if(!_paused)
        return;

    _pause = false;
    _paused = false;

    waitForResume.wakeAll();
}

void LayoutThread::execute()
{
    resume();
}

void LayoutThread::stop()
{
    QMutexLocker locker(&mutex);
    _stop = true;
    _pause = false;

    for(Layout* layout : layouts)
        layout->cancel();

    waitForResume.wakeAll();
}

bool LayoutThread::iterative()
{
    for(Layout* layout : layouts)
    {
        if(layout->iterative())
            return true;
    }

    return false;
}

bool LayoutThread::allLayoutsShouldPause()
{
    for(Layout* layout : layouts)
    {
        if(!layout->shouldPause())
            return false;
    }

    return true;
}

void LayoutThread::run()
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

            if(!_stop && (_pause || allLayoutsShouldPause() || (!iterative() && repeating)))
            {
                _paused = true;
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


void NodeLayoutThread::addComponent(ComponentId componentId)
{
    if(!componentLayouts.contains(componentId))
    {
        Layout* layout = layoutFactory->create(componentId);
        addLayout(layout);
        componentLayouts.insert(componentId, layout);
    }
}

void NodeLayoutThread::addAllComponents(const Graph &graph)
{
    for(ComponentId componentId : *graph.componentIds())
        addComponent(componentId);
}

void NodeLayoutThread::removeComponent(ComponentId componentId)
{
    bool resumeAfterRemoval = false;

    if(!paused())
    {
        pauseAndWait();
        resumeAfterRemoval = true;
    }

    if(componentLayouts.contains(componentId))
    {
        Layout* layout = componentLayouts[componentId];
        removeLayout(layout);
        componentLayouts.remove(componentId);
    }

    if(resumeAfterRemoval)
        resume();
}
