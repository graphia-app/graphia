#ifndef DEFERREDEXECUTOR_H
#define DEFERREDEXECUTOR_H

#include <mutex>
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

    std::mutex _mutex;
    std::deque<Task> _tasks;

public:
    DeferredExecutor() {}
    virtual ~DeferredExecutor();

    void enqueue(TaskFn function, const QString& description = QString());

    void execute();
    void cancel();
};

#endif // DEFERREDEXECUTOR_H
