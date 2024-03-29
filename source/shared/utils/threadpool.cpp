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

#include "threadpool.h"

#include "shared/utils/thread.h"

#include <QtGlobal>

using namespace Qt::Literals::StringLiterals;

ThreadPool::ThreadPool(const QString& threadNamePrefix, unsigned int numThreads)
{
#ifdef Q_OS_WASM
    // On Emscripten thread support is implemented by providing a fixed pthread pool
    // which we size relative to hardware concurrency (via navigator.hardwareConcurrency)
    // Exhausting this pool potentially results in deadlock, so for /our/ thread pool,
    // we only occupy half of them, hopefully reducing the chance of this occuring
    numThreads = std::min(numThreads, std::thread::hardware_concurrency() / 2);
#endif

    for(unsigned int i = 0U; i < numThreads; i++)
    {
        auto threadName = u"%1%2"_s.arg(threadNamePrefix).arg(i + 1);

        _threads.emplace_back([threadName, this]
        {
            std::unique_lock<std::mutex> lock(_mutex);

            while(!_stop)
            {
                while(_tasks.empty() && !_stop)
                {
                    u::setCurrentThreadName(u"%1 (idle)"_s.arg(threadName));

                    // Block until a new task is queued
                    _waitForNewTask.wait(lock);
                }

                if(_stop)
                    break;

                auto task = std::move(_tasks.front());
                _tasks.pop();
                lock.unlock();

                u::setCurrentThreadName(u"%1 (busy)"_s.arg(threadName));
                task();

                lock.lock();
            }
        });
    }
}

ThreadPool::~ThreadPool()
{
    // Cancel all pending tasks
    std::unique_lock<std::mutex> lock(_mutex);
    _stop = true;
    while(!_tasks.empty())
        _tasks.pop();
    lock.unlock();

    // Tell all idle threads to unblock
    _waitForNewTask.notify_all();

    // Wait for all threads to finish
    for(auto& thread : _threads)
    {
        if(thread.joinable())
            thread.join();
    }
}
