#ifndef DEFERREDEXECUTOR_H
#define DEFERREDEXECUTOR_H

#include <mutex>
#include <atomic>
#include <deque>
#include <functional>
#include <thread>
#include <condition_variable>
#include <map>

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

    mutable std::recursive_mutex _mutex;
    std::deque<Task> _tasks;
    int _debug;
    bool _paused = false;

    std::condition_variable_any _waitCondition;
    std::map<std::thread::id, size_t> _waitCount;

public:
    DeferredExecutor();
    virtual ~DeferredExecutor();

    size_t enqueue(TaskFn&& function, const QString& description = QString());

    void execute();
    void executeOne();
    void cancel();

    void pause();
    void resume();

    bool hasTasks() const;

    void waitFor(size_t numTasks);
};

#endif // DEFERREDEXECUTOR_H
