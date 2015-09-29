#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "singleton.h"

#include <QString>

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

template<typename It> static It incrementIterator(It it, It last, const int n)
{
    return it + std::min(n, static_cast<const int>(std::distance(it, last)));
}

class ThreadPool : public Singleton<ThreadPool>
{
private:
    std::vector<std::thread> _threads;
    std::mutex _mutex;
    std::condition_variable _waitForNewTask;
    std::queue<std::function<void()>> _tasks;
    std::atomic<bool> _stop;
    std::atomic<int> _activeThreads;

public:
    ThreadPool(const QString& threadNamePrefix = QString("Worker"),
               int numThreads = std::thread::hardware_concurrency());
    ~ThreadPool();

    bool saturated() { return _activeThreads >= static_cast<int>(_threads.size()); }
    bool idle() { return _activeThreads == 0; }

    template<typename Fn, typename... Args> using ReturnType = typename std::result_of<Fn(Args...)>::type;

    template<typename Fn, typename... Args> std::future<ReturnType<Fn, Args...>> execute_on_threadpool(Fn&& f, Args&&... args)
    {
        if(_stop)
            return std::future<ReturnType<Fn, Args...>>();

        auto taskPtr = std::make_shared<std::packaged_task<ReturnType<Fn, Args...>(Args...)>>(f);

        {
            std::unique_lock<std::mutex> lock(_mutex);
            _tasks.push(
                [taskPtr, args...]() mutable
                {
                    (*taskPtr)(std::forward<Args>(args)...);
                });
        }

        // Wake a thread up
        _waitForNewTask.notify_one();
        _activeThreads++;
        return taskPtr->get_future();
    }

private:
    template<typename ValueType> class ResultsType
    {
        friend class ThreadPool;

    private:
        std::vector<std::future<ValueType>> _futures;

        ResultsType(std::vector<std::future<ValueType>>& futures) :
            _futures(std::move(futures))
        {}

    public:
        ResultsType(ResultsType&& other) :
            _futures(std::move(other._futures))
        {}

        void wait()
        {
            for(auto& future : _futures)
                future.wait();
        }

        template<typename T = ValueType> typename std::enable_if<!std::is_void<T>::value, T>::type
        get()
        {
            //FIXME: profile this
            ValueType values;

            for(auto& future : _futures)
            {
                const auto& v = future.get();
                values.reserve(values.size() + v.size());
                values.insert(values.end(), v.begin(), v.end());
            }

            return values;
        }
    };

    template<typename It, typename Fn, typename T> struct Executor
    {
        using ValueType = std::vector<T>;

        ValueType operator()(It it, It last, Fn&& f)
        {
            ValueType values;

            for(; it != last; ++it)
                values.emplace_back(f(*it));

            return values;
        }
    };

    template<typename It, typename Fn> struct Executor<It, Fn, void>
    {
        using ValueType = void;

        ValueType operator()(It it, It last, Fn&& f)
        {
            for(; it != last; ++it)
                f(*it);
        }
    };

    template<typename It, typename Fn> using FnExecutor =
        Executor<It, Fn, typename std::result_of<Fn(typename It::value_type)>::type>;

public:
    template<typename It, typename Fn> using Results =
        ResultsType<typename FnExecutor<It, Fn>::ValueType>;

    template<typename It, typename Fn> auto concurrent_for(It first, It last, Fn&& f, bool blocking = true) ->
        Results<It, Fn>
    {
        const int numElements = std::distance(first, last);
        const int numThreads = static_cast<int>(_threads.size());
        const int numElementsPerThread = numElements / numThreads +
                (numElements % numThreads ? 1 : 0);

        FnExecutor<It, Fn> executor;
        std::vector<std::future<typename FnExecutor<It, Fn>::ValueType>> futures;

        for(It it = first; it < last; it = incrementIterator(it, last, numElementsPerThread))
        {
            It threadLast = incrementIterator(it, last, numElementsPerThread);
            futures.emplace_back(execute_on_threadpool([executor, it, threadLast, f]() mutable
            {
                return executor(it, threadLast, std::move(f));
            }));
        }

        auto results = Results<It, Fn>(futures);

        if(blocking)
            results.wait();

        return results;
    }
};

template<typename Fn, typename... Args> std::future<ThreadPool::ReturnType<Fn, Args...>> execute_on_threadpool(Fn&& f, Args&&... args)
{
    return ThreadPool::instance()->execute_on_threadpool(std::move(f), args...);
}

template<typename It, typename Fn> auto concurrent_for(It first, It last, Fn&& f, bool blocking = true) ->
    ThreadPool::Results<It, Fn>
{
    return ThreadPool::instance()->concurrent_for(first, last, std::move(f), blocking);
}

#endif // THREADPOOL_H
