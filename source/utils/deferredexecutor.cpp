#include "deferredexecutor.h"

DeferredExecutor::~DeferredExecutor()
{
    cancel();
}

void DeferredExecutor::enqueue(TaskFn task)
{
    std::unique_lock<std::mutex>(_mutex);

    _tasks.emplace(task);
}

void DeferredExecutor::execute()
{
    std::unique_lock<std::mutex>(_mutex);

    while(!_tasks.empty())
    {
        auto task = _tasks.front();
        task();
        _tasks.pop();
    }
}

void DeferredExecutor::cancel()
{
    std::unique_lock<std::mutex>(_mutex);

    while(!_tasks.empty())
        _tasks.pop();
}
