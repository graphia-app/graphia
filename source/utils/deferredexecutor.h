#ifndef DEFERREDEXECUTOR_H
#define DEFERREDEXECUTOR_H

#include <mutex>
#include <atomic>
#include <deque>
#include <functional>

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

    std::recursive_mutex _mutex;
    std::deque<Task> _tasks;
    int _debug;
    std::atomic<bool> _executing = false;

public:
    DeferredExecutor();
    virtual ~DeferredExecutor();

    void enqueue(TaskFn function, const QString& description = QString());

    void execute();
    void cancel();
};

#endif // DEFERREDEXECUTOR_H
