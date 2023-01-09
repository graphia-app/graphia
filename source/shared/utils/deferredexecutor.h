/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

#ifndef DEFERREDEXECUTOR_H
#define DEFERREDEXECUTOR_H

#include <mutex>
#include <atomic>
#include <deque>
#include <functional>
#include <thread>
#include <condition_variable>
#include <map>

#include <QString>

class DeferredExecutor
{
public:
    using TaskFn = std::function<void()>;

private:
    struct Task
    {
        TaskFn _function;
        QString _description;
    };

    mutable std::recursive_mutex _mutex;
    std::deque<Task> _tasks;
    int _debug;
    bool _paused = false;

    std::condition_variable_any _waitCondition;
    std::map<std::thread::id, size_t> _waitCount;

public:
    DeferredExecutor();
    virtual ~DeferredExecutor();

    size_t enqueue(TaskFn&& function, const QString& description = QString());

    void execute();
    void executeOne();
    void cancel();

    void pause();
    void resume();

    bool hasTasks() const;

    void waitFor(size_t numTasks);
};

#endif // DEFERREDEXECUTOR_H
