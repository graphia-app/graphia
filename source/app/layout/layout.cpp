#include "layout.h"
#include "shared/utils/utils.h"

#include "../graph/componentmanager.h"

static bool layoutIsFinished(const Layout& layout)
{
    return layout.finished() || layout.graphComponent().numNodes() == 1;
}

LayoutThread::LayoutThread(GraphModel& graphModel,
                           std::unique_ptr<LayoutFactory> layoutFactory,
                           bool repeating) :
    _graphModel(&graphModel),
    _repeating(repeating),
    _layoutFactory(std::move(layoutFactory)),
    _executedAtLeastOnce(graphModel.graph()),
    _intermediatePositions(graphModel.graph()),
    _performanceCounter(std::chrono::seconds(1))
{
    _debug = qgetenv("LAYOUT_DEBUG").toInt();
    _performanceCounter.setReportFn([this](float ticksPerSecond)
    {
        if(_debug > 1)
        {
            auto activeLayouts = std::count_if(_layouts.begin(), _layouts.end(),
                                               [](auto& layout) { return !layoutIsFinished(*layout.second); });
            qDebug() << activeLayouts << "layouts\t" << ticksPerSecond << "ips";
        }
    });

    connect(&graphModel.graph(), &Graph::graphChanged,
    [this]
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _layoutPotentiallyRequired = true;
    });

    connect(&graphModel.graph(), &Graph::componentSplit, this, &LayoutThread::onComponentSplit, Qt::DirectConnection);
    connect(&graphModel.graph(), &Graph::componentAdded, this, &LayoutThread::onComponentAdded, Qt::DirectConnection);
    connect(&graphModel.graph(), &Graph::componentWillBeRemoved, this, &LayoutThread::onComponentWillBeRemoved, Qt::DirectConnection);
}

void LayoutThread::pause()
{
    std::unique_lock<std::mutex> lock(_mutex);
    if(_paused)
        return;

    _pause = true;

    for(auto& layout : _layouts)
        layout.second->cancel();

    lock.unlock();
}

void LayoutThread::pauseAndWait()
{
    std::unique_lock<std::mutex> lock(_mutex);
    if(_paused)
        return;

    _pause = true;

    for(auto& layout : _layouts)
        layout.second->cancel();

    _waitForPause.wait(lock);

    lock.unlock();
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

    // Don't resume if there is nothing to do
    if(!workToDo())
        return;

    for(auto& layout : _layouts)
    {
        if(layout.second->finished())
            layout.second->unfinish();
    }

    _pause = false;

    _waitForResume.notify_all();

    lock.unlock();
}

void LayoutThread::start()
{
    _started = true;
    _paused = false;

    if(_thread.joinable())
        _thread.join();

    _thread = std::thread(&LayoutThread::run, this);
}

void LayoutThread::stop()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _stop = true;
    _pause = false;

    for(auto& layout : _layouts)
        layout.second->cancel();

    _waitForResume.notify_all();
}

bool LayoutThread::finished()
{
    std::unique_lock<std::mutex> lock(_mutex);

    return !workToDo();
}

bool LayoutThread::iterative()
{
    for(auto& layout : _layouts)
    {
        if(layout.second->iterative())
            return true;
    }

    return false;
}

void LayoutThread::uncancel()
{
    for(auto& layout : _layouts)
        layout.second->uncancel();
}

bool LayoutThread::allLayoutsFinished()
{
    for(auto& layout : _layouts)
    {
        if(!layoutIsFinished(*layout.second))
            return false;
    }

    return true;
}

bool LayoutThread::workToDo()
{
    return _layoutPotentiallyRequired || !allLayoutsFinished();
}

void LayoutThread::run()
{
    do
    {
        u::setCurrentThreadName("Layout >");

        for(auto& layout : _layouts)
        {
            if(layoutIsFinished(*layout.second))
                continue;

            layout.second->execute(!_executedAtLeastOnce.get(layout.first));
            _executedAtLeastOnce.set(layout.first, true);
        }

        _graphModel->nodePositions().update(_intermediatePositions);
        _performanceCounter.tick();
        emit executed();

        std::unique_lock<std::mutex> lock(_mutex);

        _layoutPotentiallyRequired = false;

        if(_stop)
            break;

        if(!_stop && (_pause || allLayoutsFinished() || (!iterative() && _repeating)))
        {
            _paused = true;
            emit pausedChanged();

            if(_debug) qDebug() << "Layout paused";

            u::setCurrentThreadName("Layout ||");
            uncancel();
            _waitForPause.notify_all();
            _waitForResume.wait(lock);

            _paused = false;
            emit pausedChanged();

            if(_debug) qDebug() << "Layout resumed";
        }

        std::this_thread::yield();
    }
    while(iterative() || _repeating || allLayoutsFinished());

    std::unique_lock<std::mutex> lock(_mutex);
    _layouts.clear();
    _started = false;
    _paused = true;
    _waitForPause.notify_all();

    if(_debug) qDebug() << "Layout stopped";
}


void LayoutThread::addComponent(ComponentId componentId)
{
    if(!u::contains(_layouts, componentId))
    {
        std::unique_lock<std::mutex> lock(_mutex);

        auto layout = _layoutFactory->create(componentId, _intermediatePositions);

        connect(&_layoutFactory->settings(), &LayoutSettings::settingChanged,
        [this]
        {
            std::unique_lock<std::mutex> innerLock(_mutex);
            _layoutPotentiallyRequired = true;
            innerLock.unlock();

            emit settingChanged();
        });

        _layouts.emplace(componentId, layout);
        _graphModel->nodePositions().setScale(layout->scaling());
        _graphModel->nodePositions().setSmoothing(layout->smoothing());
    }
}

void LayoutThread::addAllComponents()
{
    for(ComponentId componentId : _graphModel->graph().componentIds())
        addComponent(componentId);

    // The new components may need to be laid out so resume in case they do
    resume();
}

void LayoutThread::removeComponent(ComponentId componentId)
{
    bool resumeAfterRemoval = false;

    if(!paused())
    {
        pauseAndWait();
        resumeAfterRemoval = true;
    }

    if(u::contains(_layouts, componentId))
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _layouts.erase(componentId);
    }

    if(resumeAfterRemoval)
        resume();
}

void LayoutThread::onComponentSplit(const Graph*, const ComponentSplitSet& componentSplitSet)
{
    // When a component splits and it has already had a layout iteration, all the splitees
    // have therefore also had an iteration
    if(_executedAtLeastOnce.get(componentSplitSet.oldComponentId()))
    {
        for(auto componentId : componentSplitSet.splitters())
            _executedAtLeastOnce.set(componentId, true);
    }
}

void LayoutThread::onComponentAdded(const Graph*, ComponentId componentId, bool)
{
    addComponent(componentId);
}

void LayoutThread::onComponentWillBeRemoved(const Graph*, ComponentId componentId, bool)
{
    removeComponent(componentId);
}

std::vector<LayoutSetting>& LayoutThread::settingsVector()
{
   return _layoutFactory->settings().vector();
}
