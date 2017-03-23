#include "deferredexecutor.h"

#include "shared/utils/utils.h"

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
    std::unique_lock<std::recursive_mutex> lock(_mutex);

    Task task;
    task._function = std::move(function);
    task._description = description;

    if(_debug > 1)
        qDebug() << "enqueue(...) thread:" << u::currentThreadName() << description;

    _tasks.emplace_back(task);
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

    _executing = true;
    task._function();
    _executing = false;
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
