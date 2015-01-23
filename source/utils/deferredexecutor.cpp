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

    _tasks.emplace_back(task);
}

void DeferredExecutor::execute()
{
    std::unique_lock<std::mutex>(_mutex);
    bool debug = qgetenv("DEFERREDEXECUTOR_DEBUG").toInt();

    if(debug)
    {
        if(!_tasks.empty())
            qDebug() << "[";

        for(auto task : _tasks)
            qDebug() << task._description;

        if(!_tasks.empty())
            qDebug() << "]";
    }

    while(!_tasks.empty())
    {
        auto task = _tasks.front();

        if(debug)
            qDebug() << "Executing" << task._description;

        task._function();
        _tasks.pop_front();
    }
}

void DeferredExecutor::cancel()
{
    std::unique_lock<std::mutex>(_mutex);

    while(!_tasks.empty())
        _tasks.pop_front();
}
