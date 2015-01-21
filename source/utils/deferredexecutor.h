#ifndef DEFERREDEXECUTOR_H
#define DEFERREDEXECUTOR_H

#include <mutex>
#include <queue>
#include <functional>

class DeferredExecutor
{
public:
    using TaskFn = std::function<void()>;

private:
    std::mutex _mutex;
    std::queue<TaskFn> _tasks;

public:
    DeferredExecutor() {}
    virtual ~DeferredExecutor();

    void enqueue(TaskFn task);

    void execute();
    void cancel();
};

#endif // DEFERREDEXECUTOR_H
