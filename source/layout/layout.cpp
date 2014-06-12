#include "layout.h"

BoundingBox3D NodeLayout::boundingBox(const ReadOnlyGraph &graph, const NodePositions &positions)
{
    std::vector<QVector3D> graphPositions;
    for(NodeId nodeId : graph.nodeIds())
        graphPositions.push_back(positions[nodeId]);

    return BoundingBox3D(graphPositions);
}

BoundingBox3D NodeLayout::boundingBox() const
{
    return NodeLayout::boundingBox(*_graph, *this->_positions);
}

BoundingBox2D NodeLayout::boundingBoxInXY(const ReadOnlyGraph &graph, const NodePositions &positions)
{
    BoundingBox3D boundingBox = NodeLayout::boundingBox(graph, positions);

    return BoundingBox2D(
                QVector2D(boundingBox.min().x(), boundingBox.min().y()),
                QVector2D(boundingBox.max().x(), boundingBox.max().y()));
}

BoundingSphere NodeLayout::boundingSphere() const
{
    return NodeLayout::boundingSphere(*_graph, *this->_positions);
}

float NodeLayout::boundingCircleRadiusInXY(const ReadOnlyGraph &graph, const NodePositions &positions)
{
    BoundingBox3D boundingBox = NodeLayout::boundingBox(graph, positions);
    return std::max(boundingBox.xLength(), boundingBox.yLength()) * 0.5f * std::sqrt(2.0f);
}

BoundingSphere NodeLayout::boundingSphere(const ReadOnlyGraph &graph, const NodePositions &positions)
{
    BoundingBox3D boundingBox = NodeLayout::boundingBox(graph, positions);
    BoundingSphere boundingSphere(
        boundingBox.centre(),
        std::max(std::max(boundingBox.xLength(), boundingBox.yLength()), boundingBox.zLength()) * 0.5f * std::sqrt(3.0f)
    );

    return boundingSphere;
}

BoundingBox2D ComponentLayout::boundingBox() const
{
    BoundingBox2D _boundingBox;

    for(ComponentId componentId : *_graph->componentIds())
    {
        const ReadOnlyGraph& component = *_graph->componentById(componentId);
        float componentRadius = NodeLayout::boundingCircleRadiusInXY(component, *_nodePositions);
        QVector2D componentPosition = (*_componentPositions)[componentId];
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
    return NodeLayout::boundingCircleRadiusInXY(component, *_nodePositions);
}

BoundingBox2D ComponentLayout::boundingBoxOfComponent(ComponentId componentId) const
{
    const ReadOnlyGraph& component = *_graph->componentById(componentId);
    return NodeLayout::boundingBoxInXY(component, *_nodePositions);
}


void LayoutThread::addLayout(Layout *layout)
{
    std::lock_guard<std::mutex> locker(_mutex);
    _layouts.insert(layout);
}

void LayoutThread::removeLayout(Layout *layout)
{
    std::lock_guard<std::mutex> locker(_mutex);

    _layouts.erase(layout);
    delete layout;
}

void LayoutThread::pause()
{
    std::lock_guard<std::mutex> locker(_mutex);
    if(_paused)
        return;

    _pause = true;

    for(Layout* layout : _layouts)
        layout->cancel();
}

void LayoutThread::pauseAndWait()
{
    std::unique_lock<std::mutex> lock(_mutex);
    if(_paused)
        return;

    _pause = true;

    for(Layout* layout : _layouts)
        layout->cancel();

    _waitForPause.wait(lock);
}

bool LayoutThread::paused()
{
    std::lock_guard<std::mutex> locker(_mutex);
    return _paused;
}

void LayoutThread::resume()
{
    std::lock_guard<std::mutex> locker(_mutex);
    if(!_paused)
        return;

    _pause = false;
    _paused = false;

    _waitForResume.notify_all();
}

void LayoutThread::execute()
{
    resume();
}

void LayoutThread::start()
{
    _thread = std::thread(&LayoutThread::run, this);
}

void LayoutThread::stop()
{
    std::lock_guard<std::mutex> locker(_mutex);
    _stop = true;
    _pause = false;

    for(Layout* layout : _layouts)
        layout->cancel();

    _waitForResume.notify_all();
}

bool LayoutThread::iterative()
{
    for(Layout* layout : _layouts)
    {
        if(layout->iterative())
            return true;
    }

    return false;
}

bool LayoutThread::allLayoutsShouldPause()
{
    for(Layout* layout : _layouts)
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
        for(Layout* layout : _layouts)
        {
            if(layout->shouldPause())
                continue;

            layout->execute(_iteration);
        }
        _iteration++;

        emit executed();

        std::unique_lock<std::mutex> lock(_mutex);

        if(_stop)
        {
            lock.unlock();
            break;
        }

        if(!_stop && (_pause || allLayoutsShouldPause() || (!iterative() && _repeating)))
        {
            _paused = true;
            _waitForPause.notify_all();
            _waitForResume.wait(lock);
        }
        else
            lock.unlock();

        std::this_thread::yield();
    }
    while(iterative() || _repeating);

    _mutex.lock();
    for(Layout* layout : _layouts)
        delete layout;

    _layouts.clear();
    _mutex.unlock();
}


void NodeLayoutThread::addComponent(ComponentId componentId)
{
    if(componentLayouts.find(componentId) == componentLayouts.end())
    {
        Layout* layout = layoutFactory->create(componentId);
        addLayout(layout);
        componentLayouts.insert(std::pair<ComponentId, Layout*>(componentId, layout));
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

    if(componentLayouts.find(componentId) != componentLayouts.end())
    {
        Layout* layout = componentLayouts[componentId];
        removeLayout(layout);
        componentLayouts.erase(componentId);
    }

    if(resumeAfterRemoval)
        resume();
}
