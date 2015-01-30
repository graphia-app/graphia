#ifndef THREADPOOL_H
#define THREADPOOL_H

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

class ThreadPool
{
private:
    std::vector<std::thread> _threads;
    std::mutex _mutex;
    std::condition_variable _waitForNewTask;
    std::queue<std::function<void()>> _tasks;
    std::atomic<bool> _stop;

public:
    ThreadPool(const QString& threadNamePrefix = QString("ThreadPool"),
               int numThreads = std::thread::hardware_concurrency());
    ~ThreadPool();

    template<typename Fn> using ReturnType = typename std::result_of<Fn()>::type;

    template<typename Fn> std::future<ReturnType<Fn>> execute(Fn&& f)
    {
        if(_stop)
            return std::future<ReturnType<Fn>>();

        auto taskPtr = std::make_shared<std::packaged_task<ReturnType<Fn>()>>(f);

        {
            std::unique_lock<std::mutex> lock(_mutex);
            _tasks.push(
                [taskPtr]
                {
                    (*taskPtr)();
                });
        }

        // Wake a thread up
        _waitForNewTask.notify_one();
        return taskPtr->get_future();
    }

    template<typename ValueType> class Results
    {
        friend class ThreadPool;

    private:
        std::vector<std::future<ValueType>> _futures;

        Results(std::vector<std::future<ValueType>>& futures) :
            _futures(std::move(futures))
        {}

    public:
        Results(Results&& other) :
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
                values.insert(values.end(), v.cbegin(), v.cend());
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

    template<typename It, typename Fn> auto concurrentForEach(It first, It last, Fn&& f, bool blocking = true) ->
        Results<typename FnExecutor<It, Fn>::ValueType>
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
            futures.emplace_back(execute([executor, it, threadLast, f]() mutable
            {
                return executor(it, threadLast, std::move(f));
            }));
        }

        auto results = Results<typename FnExecutor<It, Fn>::ValueType>(futures);

        if(blocking)
            results.wait();

        return results;
    }
};

#endif // THREADPOOL_H
