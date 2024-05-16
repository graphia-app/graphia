/* Copyright © 2013-2024 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "layout.h"
#include "shared/utils/thread.h"
#include "shared/utils/container.h"

#include "app/graph/graph.h"
#include "app/graph/graphmodel.h"
#include "app/graph/componentmanager.h"

#include <QDebug>

using namespace Qt::Literals::StringLiterals;

template<> constexpr bool EnableBitMaskOperators<Layout::Dimensionality> = true;

static bool layoutIsFinished(const Layout& layout)
{
    return layout.finished() || layout.graphComponent().numNodes() == 1;
}

LayoutThread::LayoutThread(GraphModel& graphModel,
                           std::unique_ptr<LayoutFactory>&& layoutFactory,
                           bool repeating) :
    _graphModel(&graphModel),
    _repeating(repeating),
    _layoutFactory(std::move(layoutFactory)),
    _executedAtLeastOnce(graphModel.graph()),
    _nodeLayoutPositions(graphModel.graph()),
    _performanceCounter(std::chrono::seconds(1)),
    _debug(qEnvironmentVariableIntValue("LAYOUT_DEBUG"))
{
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
        const std::unique_lock<std::mutex> lock(_mutex);
        _layoutPotentiallyRequired = true;
    });

    connect(&graphModel.graph(), &Graph::componentSplit, this, &LayoutThread::onComponentSplit, Qt::DirectConnection);
    connect(&graphModel.graph(), &Graph::componentAdded, this, &LayoutThread::onComponentAdded, Qt::DirectConnection);
    connect(&graphModel.graph(), &Graph::componentWillBeRemoved, this, &LayoutThread::onComponentWillBeRemoved, Qt::DirectConnection);
    connect(this, &LayoutThread::executed, &graphModel, &GraphModel::onLayoutChanged, Qt::DirectConnection);

    connect(&_layoutFactory->settings(), &LayoutSettings::settingChanged,
    [this]
    {
        const std::unique_lock<std::mutex> lock(_mutex);

        _layoutPotentiallyRequired = true;
        unfinish();
    });

    connect(&_layoutFactory->settings(), &LayoutSettings::settingChanged,
        this, &LayoutThread::settingChanged);
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

bool LayoutThread::paused() const
{
    const std::unique_lock<std::mutex> lock(_mutex);
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

    unfinish();

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
    const std::unique_lock<std::mutex> lock(_mutex);
    _stop = true;
    _pause = false;

    for(auto& layout : _layouts)
        layout.second->cancel();

    _waitForResume.notify_all();
}

bool LayoutThread::finished() const
{
    const std::unique_lock<std::mutex> lock(_mutex);

    return !workToDo();
}

bool LayoutThread::iterative() const
{
    return std::any_of(_layouts.begin(), _layouts.end(),
    [](const auto& layout)
    {
       return layout.second->iterative();
    });
}

void LayoutThread::uncancel()
{
    for(auto& layout : _layouts)
        layout.second->uncancel();
}

void LayoutThread::unfinish()
{
    for(auto& layout : _layouts)
        layout.second->unfinish();
}

bool LayoutThread::allLayoutsFinished() const
{
    return std::all_of(_layouts.begin(), _layouts.end(),
    [](const auto& layout)
    {
       return layoutIsFinished(*layout.second);
    });
}

bool LayoutThread::workToDo() const
{
    return _layoutPotentiallyRequired || !allLayoutsFinished();
}

void LayoutThread::run()
{
    emit pausedChanged();

    do
    {
        u::setCurrentThreadName(u"Layout >"_s);

        for(auto& [componentId, layout] : _layouts)
        {
            if(layoutIsFinished(*layout))
                continue;

            if(_dimensionalityMode == Layout::Dimensionality::TwoDee &&
               (layout->dimensionality() & _dimensionalityMode))
            {
                // If we're in 2D mode and the layout can handle it, flatten the positions
                _nodeLayoutPositions.flatten();
            }

            layout->execute(!_executedAtLeastOnce.get(componentId), _dimensionalityMode);
            _executedAtLeastOnce.set(componentId, true);
        }

        {
            const std::unique_lock<NodePositions> lock(_graphModel->nodePositions());
            _graphModel->nodePositions().update(_nodeLayoutPositions);

            const bool requiresFlattening = _dimensionalityMode == Layout::Dimensionality::TwoDee &&
                std::any_of(_layouts.begin(), _layouts.end(),
                [](const auto& layout)
                {
                    return layout.second->dimensionality() ==
                        Layout::Dimensionality::ThreeDee;
                });

            if(requiresFlattening)
                _graphModel->nodePositions().flatten();

            emit executed();
        }

        _performanceCounter.tick();

        std::unique_lock<std::mutex> lock(_mutex);

        _layoutPotentiallyRequired = false;

        if(_stop)
            break;

        if(_pause || allLayoutsFinished() || (!iterative() && _repeating))
        {
            _paused = true;
            emit pausedChanged();

            if(_debug != 0)
            {
                const auto* reason = _pause ? "manually" :
                    allLayoutsFinished() ? "because all layouts finished" :
                    "";
                qDebug() << "Layout paused" << reason;
            }

            u::setCurrentThreadName(u"Layout ||"_s);
            uncancel();
            _waitForPause.notify_all();
            _waitForResume.wait(lock);

            _paused = false;
            emit pausedChanged();

            if(_debug != 0) qDebug() << "Layout resumed";
        }

        std::this_thread::yield();
    }
    while(iterative() || _repeating || allLayoutsFinished());

    const std::unique_lock<std::mutex> lock(_mutex);
    _layouts.clear();
    _started = false;
    _paused = true;
    _waitForPause.notify_all();

    if(_debug != 0) qDebug() << "Layout stopped";
}

void LayoutThread::addComponent(ComponentId componentId)
{
    if(!u::contains(_layouts, componentId))
    {
        const std::unique_lock<std::mutex> lock(_mutex);

        auto layout = _layoutFactory->create(componentId,
            _nodeLayoutPositions, _dimensionalityMode);

        _graphModel->nodePositions().setScale(layout->scaling());
        _graphModel->nodePositions().setSmoothing(layout->smoothing());
        _layouts.emplace(componentId, std::move(layout));
    }
}

void LayoutThread::addAllComponents()
{
    for(const ComponentId componentId : _graphModel->graph().componentIds())
        addComponent(componentId);
}

void LayoutThread::setNodePositions(const ExactNodePositions& nodePositions)
{
    _nodeLayoutPositions.set(_graphModel->graph().nodeIds(), nodePositions);
    _graphModel->nodePositions().update(_nodeLayoutPositions);

    // Stop the layouts throwing away our newly set positions
    _executedAtLeastOnce.fill(true);
}

Layout::Dimensionality LayoutThread::dimensionalityMode()
{
    return _dimensionalityMode;
}

void LayoutThread::setDimensionalityMode(Layout::Dimensionality dimensionalityMode)
{
    Q_ASSERT(dimensionalityMode != Layout::Dimensionality::TwoOrThreeDee);

    if(dimensionalityMode == _dimensionalityMode)
        return;

    _dimensionalityMode = dimensionalityMode;
    _layoutPotentiallyRequired = true;
}

QString LayoutThread::layoutName() const
{
    return _layoutFactory->name();
}

QString LayoutThread::layoutDisplayName() const
{
    return _layoutFactory->displayName();
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
        const std::unique_lock<std::mutex> lock(_mutex);
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

std::vector<LayoutSetting>& LayoutThread::settings()
{
    return _layoutFactory->settings().vector();
}

const LayoutSetting* LayoutThread::setting(const QString& name) const
{
    return _layoutFactory->setting(name);
}

void LayoutThread::setSettingValue(const QString& name, float value)
{
    _layoutFactory->setSettingValue(name, value);
}

void LayoutThread::setSettingNormalisedValue(const QString& name, float normalisedValue)
{
    _layoutFactory->setSettingNormalisedValue(name, normalisedValue);
}

void LayoutThread::resetSettingValue(const QString& name)
{
    _layoutFactory->resetSettingValue(name);
}
