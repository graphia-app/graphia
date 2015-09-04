#include "threadpool.h"

#include "utils.h"

ThreadPool::ThreadPool(const QString& threadNamePrefix, int numThreads) :
    _stop(false), _activeThreads(0)
{
    for(int i = 0; i < numThreads; i++)
    {
        _threads.emplace_back([threadNamePrefix, i, this]
            {
                u::nameCurrentThread(QString("%1%2").arg(threadNamePrefix).arg(i + 1));

                while(!_stop)
                {
                    std::unique_lock<std::mutex> lock(_mutex);

                    if(!_tasks.empty())
                    {
                        auto task = _tasks.front();
                        _tasks.pop();
                        lock.unlock();
                        task();
                        _activeThreads--;
                    }
                    else if(!_stop)
                    {
                        // Block until a new task is queued
                        _waitForNewTask.wait(lock);
                    }
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
