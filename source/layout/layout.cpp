#include "layout.h"
#include "../utils/namethread.h"

BoundingBox3D NodeLayout::boundingBox(const ReadOnlyGraph& graph, const NodePositions& positions)
{
    std::vector<QVector3D> graphPositions;
    for(NodeId nodeId : graph.nodeIds())
        graphPositions.push_back(positions.at(nodeId));

    return BoundingBox3D(graphPositions);
}

BoundingBox3D NodeLayout::boundingBox() const
{
    return NodeLayout::boundingBox(_graph, _positions);
}

BoundingBox2D NodeLayout::boundingBoxInXY(const ReadOnlyGraph& graph, const NodePositions& positions)
{
    BoundingBox3D boundingBox = NodeLayout::boundingBox(graph, positions);

    return BoundingBox2D(
                QVector2D(boundingBox.min().x(), boundingBox.min().y()),
                QVector2D(boundingBox.max().x(), boundingBox.max().y()));
}

BoundingSphere NodeLayout::boundingSphere() const
{
    return NodeLayout::boundingSphere(_graph, _positions);
}

float NodeLayout::boundingCircleRadiusInXY(const ReadOnlyGraph& graph, const NodePositions& positions)
{
    BoundingBox3D boundingBox = NodeLayout::boundingBox(graph, positions);
    return std::max(boundingBox.xLength(), boundingBox.yLength()) * 0.5f * std::sqrt(2.0f);
}

BoundingSphere NodeLayout::boundingSphere(const ReadOnlyGraph& graph, const NodePositions& positions)
{
    BoundingBox3D boundingBox = NodeLayout::boundingBox(graph, positions);
    BoundingSphere boundingSphere(
        boundingBox.centre(),
        std::max(std::max(boundingBox.xLength(), boundingBox.yLength()), boundingBox.zLength()) * 0.5f * std::sqrt(3.0f)
    );

    return boundingSphere;
}

void LayoutThread::addLayout(std::shared_ptr<Layout> layout)
{
    std::lock_guard<std::mutex> locker(_mutex);
    _layouts.insert(layout);
}

void LayoutThread::removeLayout(std::shared_ptr<Layout> layout)
{
    std::lock_guard<std::mutex> locker(_mutex);
    _layouts.erase(layout);
}

void LayoutThread::pause()
{
    std::lock_guard<std::mutex> locker(_mutex);
    if(_paused)
        return;

    _pause = true;

    for(auto layout : _layouts)
        layout->cancel();
}

void LayoutThread::pauseAndWait()
{
    std::unique_lock<std::mutex> lock(_mutex);
    if(_paused)
        return;

    _pause = true;

    for(auto layout : _layouts)
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
    if(!_started)
    {
        start();
        return;
    }

    std::lock_guard<std::mutex> locker(_mutex);
    if(!_paused)
        return;

    _pause = false;
    _paused = false;

    _waitForResume.notify_all();
}

void LayoutThread::start()
{
    _started = true;
    _paused = false;
    _thread = std::thread(&LayoutThread::run, this);
}

void LayoutThread::stop()
{
    std::lock_guard<std::mutex> locker(_mutex);
    _stop = true;
    _pause = false;

    for(auto layout : _layouts)
        layout->cancel();

    _waitForResume.notify_all();
}

bool LayoutThread::iterative()
{
    for(auto layout : _layouts)
    {
        if(layout->iterative())
            return true;
    }

    return false;
}

void LayoutThread::uncancel()
{
    for(auto layout : _layouts)
        layout->uncancel();
}

bool LayoutThread::allLayoutsShouldPause()
{
    for(auto layout : _layouts)
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
        nameCurrentThread("Layout >");
        for(auto layout : _layouts)
        {
            if(layout->shouldPause())
                continue;

            layout->execute(_iteration);
        }
        _iteration++;

        emit executed();

        std::unique_lock<std::mutex> lock(_mutex);

        if(_stop)
            break;

        if(!_stop && (_pause || allLayoutsShouldPause() || (!iterative() && _repeating)))
        {
            _paused = true;
            nameCurrentThread("Layout ||");
            uncancel();
            _waitForPause.notify_all();
            _waitForResume.wait(lock);
        }

        std::this_thread::yield();
    }
    while(iterative() || _repeating);

    _mutex.lock();
    _layouts.clear();
    _mutex.unlock();
}


void NodeLayoutThread::addComponent(ComponentId componentId)
{
    if(_componentLayouts.find(componentId) == _componentLayouts.end())
    {
        auto layout = _layoutFactory->create(componentId);
        addLayout(layout);
        _componentLayouts.emplace(componentId, layout);
    }
}

void NodeLayoutThread::addAllComponents(const Graph &graph)
{
    for(ComponentId componentId : graph.componentIds())
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

    if(_componentLayouts.find(componentId) != _componentLayouts.end())
    {
        auto layout = _componentLayouts[componentId];
        removeLayout(layout);
        _componentLayouts.erase(componentId);
    }

    if(resumeAfterRemoval)
        resume();
}
