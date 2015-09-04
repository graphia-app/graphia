#include "layout.h"
#include "../utils/utils.h"
#include "../utils/namethread.h"

LayoutThread::LayoutThread(GraphModel& graphModel,
                           std::unique_ptr<const LayoutFactory> layoutFactory,
                           bool repeating) :
    _graphModel(&graphModel),
    _repeating(repeating),
    _layoutFactory(std::move(layoutFactory)),
    _intermediatePositions(graphModel.graph()),
    _performanceCounter(std::chrono::seconds(3))
{
    bool debug = qgetenv("LAYOUT_DEBUG").toInt();
    _performanceCounter.setReportFn([debug](float ticksPerSecond)
    {
        if(debug)
            qDebug() << "Layout" << ticksPerSecond << "ips";
    });

    connect(&graphModel.graph(), &Graph::componentAdded, this, &LayoutThread::onComponentAdded, Qt::DirectConnection);
    connect(&graphModel.graph(), &Graph::componentWillBeRemoved, this, &LayoutThread::onComponentWillBeRemoved, Qt::DirectConnection);
}

void LayoutThread::addLayout(std::shared_ptr<Layout> layout)
{
    std::unique_lock<std::mutex> lock(_mutex);
    _layouts.insert(layout);

    if(_layouts.size() == 1)
    {
        // If this is the first layout, resume
        lock.unlock();
        resume();
    }
}

void LayoutThread::removeLayout(std::shared_ptr<Layout> layout)
{
    std::unique_lock<std::mutex> lock(_mutex);
    _layouts.erase(layout);
}

void LayoutThread::pause()
{
    std::unique_lock<std::mutex> lock(_mutex);
    if(_paused)
        return;

    _pause = true;

    for(auto layout : _layouts)
        layout->cancel();

    lock.unlock();
    emit pausedChanged();
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

    lock.unlock();
    emit pausedChanged();
}

bool LayoutThread::paused()
{
    std::unique_lock<std::mutex> lock(_mutex);
    return _paused;
}

void LayoutThread::resume()
{
    if(!_started)
    {
        start();
        return;
    }

    std::unique_lock<std::mutex> lock(_mutex);
    if(!_paused)
        return;

    _pause = false;
    _paused = false;

    _waitForResume.notify_all();

    lock.unlock();
    emit pausedChanged();
}

void LayoutThread::start()
{
    _started = true;
    _paused = false;
    emit pausedChanged();

    if(_thread.joinable())
        _thread.join();

    _thread = std::thread(&LayoutThread::run, this);
}

void LayoutThread::stop()
{
    std::unique_lock<std::mutex> lock(_mutex);
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

        _graphModel->nodePositions().update(_intermediatePositions);

        _performanceCounter.tick();

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
    while(iterative() || _repeating || allLayoutsShouldPause());

    std::unique_lock<std::mutex> lock(_mutex);
    _layouts.clear();
    _started = false;
    _paused = true;
    _waitForPause.notify_all();
}


void LayoutThread::addComponent(ComponentId componentId)
{
    if(!u::contains(_componentLayouts, componentId))
    {
        auto layout = _layoutFactory->create(componentId, _intermediatePositions);
        addLayout(layout);
        _componentLayouts.emplace(componentId, layout);
        _graphModel->nodePositions().setScale(layout->scaling());
        _graphModel->nodePositions().setSmoothing(layout->smoothing());
    }
}

void LayoutThread::addAllComponents()
{
    for(ComponentId componentId : _graphModel->graph().componentIds())
        addComponent(componentId);
}

void LayoutThread::removeComponent(ComponentId componentId)
{
    bool resumeAfterRemoval = false;

    if(!paused())
    {
        pauseAndWait();
        resumeAfterRemoval = true;
    }

    if(u::contains(_componentLayouts, componentId))
    {
        auto layout = _componentLayouts[componentId];
        removeLayout(layout);
        _componentLayouts.erase(componentId);
    }

    if(resumeAfterRemoval)
        resume();
}

void LayoutThread::onComponentAdded(const Graph*, ComponentId componentId, bool)
{
    addComponent(componentId);
}

void LayoutThread::onComponentWillBeRemoved(const Graph*, ComponentId componentId, bool)
{
    removeComponent(componentId);
}
