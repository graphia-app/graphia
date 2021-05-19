/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

#include "deferredexecutor.h"

#include "shared/utils/thread.h"
#include "shared/utils/container.h"

#include <QDebug>
#include <QtGlobal>

DeferredExecutor::DeferredExecutor()
{
    _debug = qEnvironmentVariableIntValue("DEFERREDEXECUTOR_DEBUG");
}

DeferredExecutor::~DeferredExecutor()
{
    cancel();
}

size_t DeferredExecutor::enqueue(TaskFn&& function, const QString& description)
{
    std::unique_lock<std::recursive_mutex> lock(_mutex);

    Task task;
    task._function = std::move(function);
    task._description = description;

    if(_debug > 1)
        qDebug() << "enqueue(...) thread:" << u::currentThreadName() << description;

    _tasks.emplace_back(task);

    return _tasks.size();
}

void DeferredExecutor::execute()
{
    std::unique_lock<std::recursive_mutex> lock(_mutex);

    if(_paused)
        return;

    if(!_tasks.empty() && _debug > 0)
    {
        qDebug() << "execute() thread" << u::currentThreadName();

        for(auto& task : _tasks)
            qDebug() << "\t" << task._description;
    }

    while(!_tasks.empty())
        executeOne();
}

void DeferredExecutor::executeOne()
{
    std::unique_lock<std::recursive_mutex> lock(_mutex);

    if(_paused)
        return;

    auto task = _tasks.front();
    _tasks.pop_front();

    if(_debug > 2)
        qDebug() << "Executing" << task._description;

    task._function();

    // Decrement all the wait counts
    for(auto it = _waitCount.begin(); it != _waitCount.end();)
    {
        if(it->second > 0)
            it->second--;

        if(it->second == 0)
            it = _waitCount.erase(it);
        else
            ++it;
    }

    _waitCondition.notify_all();
}

void DeferredExecutor::cancel()
{
    std::unique_lock<std::recursive_mutex> lock(_mutex);

    while(!_tasks.empty())
        _tasks.pop_front();
}

void DeferredExecutor::pause()
{
    std::unique_lock<std::recursive_mutex> lock(_mutex);
    _paused = true;
}

void DeferredExecutor::resume()
{
    std::unique_lock<std::recursive_mutex> lock(_mutex);
    _paused = false;
}

bool DeferredExecutor::hasTasks() const
{
    std::unique_lock<std::recursive_mutex> lock(_mutex);

    return !_tasks.empty();
}

void DeferredExecutor::waitFor(size_t numTasks)
{
    std::unique_lock<std::recursive_mutex> lock(_mutex);

    if(_tasks.size() == 0 || numTasks == 0)
    {
        // If there are no tasks to wait for, we can just return
        // This can happen when task(s) are executed and completed
        // before the call to waitFor occurs
        if(_debug > 2)
        {
            qDebug() << "waitFor(" << numTasks << ") called with" <<
                _tasks.size() << "tasks queued thread:" << u::currentThreadName();
        }

        return;
    }

    numTasks = std::min(_tasks.size(), numTasks);
    auto threadId = std::this_thread::get_id();
    _waitCount[threadId] = numTasks;

    if(_debug > 1)
    {
        qDebug() << "waitFor(" << numTasks << ") thread:" <<
            u::currentThreadName();
    }

    // Keep waiting until there are no remaining tasks
    _waitCondition.wait(lock,
    [this, threadId]
    {
        return !u::contains(_waitCount, threadId);
    });

    if(_debug > 1)
    {
        qDebug() << "waitFor complete thread:" <<
            u::currentThreadName();
    }
}
