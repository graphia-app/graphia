#include "deferredexecutor.h"

#include "../utils/utils.h"

#include <QDebug>
#include <QtGlobal>

DeferredExecutor::DeferredExecutor() :
    _executing(false)
{
    _debug = qgetenv("DEFERREDEXECUTOR_DEBUG").toInt();
}

DeferredExecutor::~DeferredExecutor()
{
    cancel();
}

void DeferredExecutor::enqueue(TaskFn function, const QString& description)
{
    if(_executing)
        std::abort(); // Calling enqueue from execute

    std::unique_lock<std::mutex> lock(_mutex);

    Task task;
    task._function = function;
    task._description = description;

    if(_debug)
        qDebug() << "enqueue(...) thread ID:" << Utils::currentThreadId() << description;

    _tasks.emplace_back(task);
}

void DeferredExecutor::execute()
{
    std::unique_lock<std::mutex> lock(_mutex);

    if(!_tasks.empty() && _debug)
    {
        qDebug() << "execute() thread ID" << Utils::currentThreadId();
        qDebug() << "[";

        for(auto task : _tasks)
            qDebug() << task._description;

        qDebug() << "]";
    }

    while(!_tasks.empty())
    {
        auto task = _tasks.front();

        if(_debug)
            qDebug() << "Executing" << task._description;

        _executing = true;
        task._function();
        _executing = false;
        _tasks.pop_front();
    }
}

void DeferredExecutor::cancel()
{
    std::unique_lock<std::mutex> lock(_mutex);

    while(!_tasks.empty())
        _tasks.pop_front();
}
