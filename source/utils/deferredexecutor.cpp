#include "deferredexecutor.h"

#include <QDebug>
#include <QtGlobal>

DeferredExecutor::~DeferredExecutor()
{
    cancel();
}

void DeferredExecutor::enqueue(TaskFn function, const QString& description)
{
    std::unique_lock<std::mutex>(_mutex);

    Task task;
    task._function = function;
    task._description = description;

    _tasks.emplace(task);
}

void DeferredExecutor::execute()
{
    std::unique_lock<std::mutex>(_mutex);

    while(!_tasks.empty())
    {
        auto task = _tasks.front();
        task._function();

        if(qgetenv("DEFERREDEXECUTOR_DEBUG").toInt())
            qDebug() << task._description;

        _tasks.pop();
    }
}

void DeferredExecutor::cancel()
{
    std::unique_lock<std::mutex>(_mutex);

    while(!_tasks.empty())
        _tasks.pop();
}
