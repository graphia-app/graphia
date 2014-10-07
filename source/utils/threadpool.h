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

    class Results
    {
        friend class ThreadPool;

    private:
        std::vector<std::future<void>> _futures;

        Results(std::vector<std::future<void>>& futures) :
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
    };

    template<typename It, typename Fn> Results concurrentForEach(It first, It last, Fn&& f,
                                                                 bool blocking = true)
    {
        const int numElements = std::distance(first, last);
        const int threadElements = numElements / _threads.size() +
                (numElements % _threads.size() ? 1 : 0);

        std::vector<std::future<void>> futures;

        for(It it = first; it < last; it += threadElements)
        {
            It threadLast = std::min(it + threadElements, last);
            futures.emplace_back(execute([it, threadLast, f]
            {
                std::for_each(it, threadLast, f);
            }));
        }

        auto results = Results(futures);

        if(blocking)
            results.wait();

        return results;
    }
};

#endif // THREADPOOL_H
